// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <tokens/token.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <consensus/amount.h>
#include <crypto/sha256.h>
#include <util/strencodings.h>

#include <algorithm>
#include <sstream>

// Global token manager instance
std::unique_ptr<CTokenManager> g_tokenManager = std::make_unique<CTokenManager>();

// CTokenInfo implementation
uint256 CTokenInfo::GetHash() const
{
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << name << symbol << decimals << totalSupply << creator << creationTime;
    return hasher.GetHash();
}

bool CTokenInfo::IsValid() const
{
    return !name.empty() && !symbol.empty() && totalSupply > 0 && !tokenHash.IsNull();
}

std::string CTokenInfo::GetDisplayName() const
{
    return name + " (" + symbol + ")";
}

// CTokenTx implementation
uint256 CTokenTx::GetHash() const
{
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << static_cast<int>(type) << tokenHash << from << to << amount << timestamp;
    if (IsCreationTx()) {
        hasher << tokenName << tokenSymbol << tokenDecimals << tokenTotalSupply;
    }
    return hasher.GetHash();
}

bool CTokenTx::IsValid() const
{
    if (amount < 0) {
        return false;
    }
    
    switch (type) {
        case TokenTxType::CREATE:
            return !tokenName.empty() && !tokenSymbol.empty() && tokenTotalSupply > 0;
        case TokenTxType::TRANSFER:
        case TokenTxType::TRANSFER_FROM:
            return !tokenHash.IsNull() && amount > 0;
        case TokenTxType::MINT:
        case TokenTxType::BURN:
            return !tokenHash.IsNull() && amount > 0;
        case TokenTxType::APPROVE:
            return !tokenHash.IsNull();
        default:
            return false;
    }
}

bool CTokenTx::IsCreationTx() const
{
    return type == TokenTxType::CREATE;
}

// CTokenManager implementation
CTokenManager::CTokenManager()
{
    m_stats = {0, 0, 0, 0, GetTime()};
    m_currentSHAHPrice = 5000; // Default: $50.00 in cents
}

CTokenManager::~CTokenManager()
{
}

void CTokenManager::SetFeeConfig(const TokenFeeConfig& config)
{
    m_feeConfig = config;
    LogPrint(BCLog::TOKENS, "Token fee configuration updated\n");
}

bool CTokenManager::CreateToken(const CTxDestination& creator, const std::string& name, 
                               const std::string& symbol, uint8_t decimals, CAmount totalSupply)
{
    if (!ValidateTokenName(name) || !ValidateTokenSymbol(symbol) || 
        !ValidateTokenDecimals(decimals) || !ValidateTokenSupply(totalSupply)) {
        LogPrint(BCLog::TOKENS, "Invalid token parameters: name=%s, symbol=%s, decimals=%d, supply=%d\n",
                 name, symbol, decimals, totalSupply);
        return false;
    }
    
    CTokenInfo token;
    token.name = name;
    token.symbol = symbol;
    token.decimals = decimals;
    token.totalSupply = totalSupply;
    token.creator = creator;
    token.creationTime = GetTime();
    token.tokenHash = TokenUtils::GenerateTokenAddress(creator, name, symbol, token.creationTime);
    token.isActive = true;
    
    // Add to storage
    m_tokens[token.tokenHash] = token;
    UpdateTokenIndexes(token, true);
    
    // Set initial balance to creator
    UpdateTokenBalance(token.tokenHash, creator, totalSupply);
    
    UpdateStats();
    
    LogPrint(BCLog::TOKENS, "Created token: %s (%s), hash=%s, creator=%s\n",
             name, symbol, token.tokenHash.ToString(), EncodeDestination(creator));
    
    return true;
}

bool CTokenManager::ValidateTokenCreation(const CTokenTx& tokenTx)
{
    if (!tokenTx.IsCreationTx()) {
        return false;
    }
    
    return ValidateTokenName(tokenTx.tokenName) && 
           ValidateTokenSymbol(tokenTx.tokenSymbol) && 
           ValidateTokenDecimals(tokenTx.tokenDecimals) && 
           ValidateTokenSupply(tokenTx.tokenTotalSupply);
}

bool CTokenManager::TransferTokens(const uint256& tokenHash, const CTxDestination& from,
                                  const CTxDestination& to, CAmount amount)
{
    if (!IsValidToken(tokenHash)) {
        return false;
    }
    
    CAmount currentBalance = GetTokenBalance(tokenHash, from);
    if (currentBalance < amount) {
        LogPrint(BCLog::TOKENS, "Insufficient token balance: required=%d, available=%d\n",
                 amount, currentBalance);
        return false;
    }
    
    // Update balances
    if (!UpdateTokenBalance(tokenHash, from, currentBalance - amount)) {
        return false;
    }
    
    CAmount toBalance = GetTokenBalance(tokenHash, to);
    if (!UpdateTokenBalance(tokenHash, to, toBalance + amount)) {
        // Revert from balance
        UpdateTokenBalance(tokenHash, from, currentBalance);
        return false;
    }
    
    LogPrint(BCLog::TOKENS, "Transferred %d tokens from %s to %s\n",
             amount, EncodeDestination(from), EncodeDestination(to));
    
    return true;
}

bool CTokenManager::MintTokens(const uint256& tokenHash, const CTxDestination& to, CAmount amount)
{
    if (!IsValidToken(tokenHash)) {
        return false;
    }
    
    CAmount currentBalance = GetTokenBalance(tokenHash, to);
    if (!UpdateTokenBalance(tokenHash, to, currentBalance + amount)) {
        return false;
    }
    
    // Update total supply
    auto it = m_tokens.find(tokenHash);
    if (it != m_tokens.end()) {
        it->second.totalSupply += amount;
    }
    
    LogPrint(BCLog::TOKENS, "Minted %d tokens to %s\n", amount, EncodeDestination(to));
    
    return true;
}

bool CTokenManager::BurnTokens(const uint256& tokenHash, const CTxDestination& from, CAmount amount)
{
    if (!IsValidToken(tokenHash)) {
        return false;
    }
    
    CAmount currentBalance = GetTokenBalance(tokenHash, from);
    if (currentBalance < amount) {
        return false;
    }
    
    if (!UpdateTokenBalance(tokenHash, from, currentBalance - amount)) {
        return false;
    }
    
    // Update total supply
    auto it = m_tokens.find(tokenHash);
    if (it != m_tokens.end()) {
        it->second.totalSupply -= amount;
    }
    
    LogPrint(BCLog::TOKENS, "Burned %d tokens from %s\n", amount, EncodeDestination(from));
    
    return true;
}

bool CTokenManager::ApproveTokens(const uint256& tokenHash, const CTxDestination& owner,
                                 const CTxDestination& spender, CAmount amount)
{
    if (!IsValidToken(tokenHash)) {
        return false;
    }
    
    return UpdateTokenAllowance(tokenHash, owner, spender, amount);
}

CTokenInfo CTokenManager::GetToken(const uint256& tokenHash) const
{
    auto it = m_tokens.find(tokenHash);
    if (it != m_tokens.end()) {
        return it->second;
    }
    return CTokenInfo();
}

std::vector<CTokenInfo> CTokenManager::GetTokensByCreator(const CTxDestination& creator) const
{
    std::vector<CTokenInfo> tokens;
    auto it = m_creatorTokens.find(creator);
    if (it != m_creatorTokens.end()) {
        for (const auto& tokenHash : it->second) {
            auto tokenIt = m_tokens.find(tokenHash);
            if (tokenIt != m_tokens.end()) {
                tokens.push_back(tokenIt->second);
            }
        }
    }
    return tokens;
}

std::vector<CTokenInfo> CTokenManager::GetAllTokens() const
{
    std::vector<CTokenInfo> tokens;
    for (const auto& pair : m_tokens) {
        if (pair.second.isActive) {
            tokens.push_back(pair.second);
        }
    }
    return tokens;
}

CAmount CTokenManager::GetTokenBalance(const uint256& tokenHash, const CTxDestination& address) const
{
    auto it = m_tokenBalances.find(std::make_pair(tokenHash, address));
    if (it != m_tokenBalances.end()) {
        return it->second;
    }
    return 0;
}

CAmount CTokenManager::GetTokenAllowance(const uint256& tokenHash, const CTxDestination& owner,
                                        const CTxDestination& spender) const
{
    auto it = m_tokenAllowances.find(std::make_tuple(tokenHash, owner, spender));
    if (it != m_tokenAllowances.end()) {
        return it->second;
    }
    return 0;
}

CAmount CTokenManager::CalculateTokenCreationFee() const
{
    if (!m_feeConfig.dynamicPricing) {
        return m_feeConfig.baseCreationFee;
    }
    
    // Calculate dynamic fee based on SHAH price
    // Target: $100 USD worth of SHAH
    CAmount shahPriceCents = GetCurrentSHAHPrice();
    if (shahPriceCents == 0) {
        return m_feeConfig.baseCreationFee; // Fallback to base fee
    }
    
    // Convert USD target to SHAH amount
    // $100 = 10000 cents
    CAmount targetSHAH = (10000 * COIN) / shahPriceCents;
    
    // Ensure minimum fee
    return std::max(targetSHAH, m_feeConfig.baseCreationFee / 10);
}

CAmount CTokenManager::GetCurrentSHAHPrice() const
{
    return m_currentSHAHPrice;
}

void CTokenManager::UpdateSHAHPrice()
{
    CAmount newPrice = FetchSHAHPrice();
    if (newPrice > 0) {
        CacheSHAHPrice(newPrice);
        LogPrint(BCLog::TOKENS, "Updated SHAH price: $%.2f\n", newPrice / 100.0);
    }
}

CTransactionRef CTokenManager::CreateTokenCreationTx(const CTxDestination& creator, const std::string& name,
                                                    const std::string& symbol, uint8_t decimals, CAmount totalSupply)
{
    CTokenTxBuilder builder;
    return builder.BuildTokenCreationTx(creator, name, symbol, decimals, totalSupply);
}

CTransactionRef CTokenManager::CreateTokenTransferTx(const uint256& tokenHash, const CTxDestination& from,
                                                    const CTxDestination& to, CAmount amount)
{
    CTokenTxBuilder builder;
    return builder.BuildTokenTransferTx(tokenHash, from, to, amount);
}

bool CTokenManager::ValidateTokenTransaction(const CTransaction& tx) const
{
    CTokenTx tokenTx;
    if (!ParseTokenTransaction(tx, tokenTx)) {
        return false;
    }
    
    return tokenTx.IsValid();
}

bool CTokenManager::IsTokenTransaction(const CTransaction& tx) const
{
    CTokenTx tokenTx;
    return ParseTokenTransaction(tx, tokenTx);
}

bool CTokenManager::IsValidToken(const uint256& tokenHash) const
{
    auto it = m_tokens.find(tokenHash);
    return it != m_tokens.end() && it->second.isActive;
}

CTokenManager::TokenStats CTokenManager::GetStats() const
{
    return m_stats;
}

void CTokenManager::ResetStats()
{
    m_stats = {0, 0, 0, 0, GetTime()};
    LogPrint(BCLog::TOKENS, "Token statistics reset\n");
}

void CTokenManager::LogTokenCreation(const CTokenInfo& token)
{
    LogPrint(BCLog::TOKENS, "Token created: %s (%s), hash=%s, creator=%s, supply=%d\n",
             token.name, token.symbol, token.tokenHash.ToString(),
             EncodeDestination(token.creator), token.totalSupply);
}

void CTokenManager::LogTokenTransaction(const CTokenTx& tokenTx)
{
    LogPrint(BCLog::TOKENS, "Token transaction: type=%s, token=%s, amount=%d\n",
             TokenUtils::GetTokenTxTypeName(tokenTx.type),
             tokenTx.tokenHash.ToString(), tokenTx.amount);
}

void CTokenManager::LogTokenStats()
{
    LogPrint(BCLog::TOKENS, "Token statistics:\n");
    LogPrint(BCLog::TOKENS, "  Total tokens: %d\n", m_stats.totalTokens);
    LogPrint(BCLog::TOKENS, "  Active tokens: %d\n", m_stats.activeTokens);
    LogPrint(BCLog::TOKENS, "  Total transactions: %d\n", m_stats.totalTransactions);
    LogPrint(BCLog::TOKENS, "  Total creation fees: %d\n", m_stats.totalCreationFees);
}

// Private helper functions
void CTokenManager::UpdateTokenIndexes(const CTokenInfo& token, bool add)
{
    if (add) {
        m_creatorTokens[token.creator].push_back(token.tokenHash);
    } else {
        auto& tokens = m_creatorTokens[token.creator];
        tokens.erase(std::remove(tokens.begin(), tokens.end(), token.tokenHash), tokens.end());
    }
}

void CTokenManager::RemoveTokenIndexes(const CTokenInfo& token)
{
    UpdateTokenIndexes(token, false);
}

bool CTokenManager::ValidateTokenName(const std::string& name) const
{
    return !name.empty() && name.length() <= 100;
}

bool CTokenManager::ValidateTokenSymbol(const std::string& symbol) const
{
    return !symbol.empty() && symbol.length() <= 10;
}

bool CTokenManager::ValidateTokenDecimals(uint8_t decimals) const
{
    return decimals <= 18;
}

bool CTokenManager::ValidateTokenSupply(CAmount supply) const
{
    return supply > 0;
}

void CTokenManager::UpdateStats()
{
    m_stats.totalTokens = m_tokens.size();
    m_stats.activeTokens = 0;
    for (const auto& pair : m_tokens) {
        if (pair.second.isActive) {
            m_stats.activeTokens++;
        }
    }
    m_stats.lastUpdateTime = GetTime();
}

void CTokenManager::CleanupInactiveTokens()
{
    std::vector<uint256> toRemove;
    
    for (const auto& pair : m_tokens) {
        if (!pair.second.isActive) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (const auto& tokenHash : toRemove) {
        auto it = m_tokens.find(tokenHash);
        if (it != m_tokens.end()) {
            RemoveTokenIndexes(it->second);
            m_tokens.erase(it);
        }
    }
}

bool CTokenManager::UpdateTokenBalance(const uint256& tokenHash, const CTxDestination& address, CAmount amount)
{
    auto key = std::make_pair(tokenHash, address);
    m_tokenBalances[key] = amount;
    return true;
}

bool CTokenManager::UpdateTokenAllowance(const uint256& tokenHash, const CTxDestination& owner,
                                        const CTxDestination& spender, CAmount amount)
{
    auto key = std::make_tuple(tokenHash, owner, spender);
    m_tokenAllowances[key] = amount;
    return true;
}

bool CTokenManager::AddTokenTransaction(CTransaction& tx, const CTokenTx& tokenTx)
{
    // Create token transaction
    CMutableTransaction mutableTx;
    mutableTx.nVersion = 1;
    mutableTx.vin.resize(1);
    mutableTx.vout.resize(1);
    
    // Add token output
    mutableTx.vout[0].nValue = 0; // Token transactions have 0 SHAH value
    mutableTx.vout[0].scriptPubKey = CTokenTxBuilder().CreateTokenScript(tokenTx);
    
    tx = CTransaction(mutableTx);
    return true;
}

bool CTokenManager::ParseTokenTransaction(const CTransaction& tx, CTokenTx& tokenTx) const
{
    if (tx.vout.empty()) {
        return false;
    }
    
    // Parse token script to extract token transaction data
    const CScript& script = tx.vout[0].scriptPubKey;
    return CTokenTxBuilder().ParseTokenScript(script, tokenTx);
}

CAmount CTokenManager::FetchSHAHPrice() const
{
    // In a real implementation, this would fetch from a price oracle
    // For now, return a simulated price
    return 5000; // $50.00 in cents
}

void CTokenManager::CacheSHAHPrice(CAmount price)
{
    m_currentSHAHPrice = price;
}

// CTokenTxBuilder implementation
CTokenTxBuilder::CTokenTxBuilder()
{
}

CTransactionRef CTokenTxBuilder::BuildTokenCreationTx(const CTxDestination& creator, const std::string& name,
                                                     const std::string& symbol, uint8_t decimals, CAmount totalSupply)
{
    CTokenTx tokenTx;
    tokenTx.type = TokenTxType::CREATE;
    tokenTx.creator = creator;
    tokenTx.tokenName = name;
    tokenTx.tokenSymbol = symbol;
    tokenTx.tokenDecimals = decimals;
    tokenTx.tokenTotalSupply = totalSupply;
    tokenTx.timestamp = GetTime();
    
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddTokenCreationOutput(tx, tokenTx);
    
    return MakeTransactionRef(std::move(tx));
}

CTransactionRef CTokenTxBuilder::BuildTokenTransferTx(const uint256& tokenHash, const CTxDestination& from,
                                                     const CTxDestination& to, CAmount amount)
{
    CTokenTx tokenTx;
    tokenTx.type = TokenTxType::TRANSFER;
    tokenTx.tokenHash = tokenHash;
    tokenTx.from = from;
    tokenTx.to = to;
    tokenTx.amount = amount;
    tokenTx.timestamp = GetTime();
    
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddTokenTransferOutput(tx, tokenTx);
    
    return MakeTransactionRef(std::move(tx));
}

CTransactionRef CTokenTxBuilder::BuildTokenMintTx(const uint256& tokenHash, const CTxDestination& to, CAmount amount)
{
    CTokenTx tokenTx;
    tokenTx.type = TokenTxType::MINT;
    tokenTx.tokenHash = tokenHash;
    tokenTx.to = to;
    tokenTx.amount = amount;
    tokenTx.timestamp = GetTime();
    
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddTokenOutput(tx, tokenTx);
    
    return MakeTransactionRef(std::move(tx));
}

CTransactionRef CTokenTxBuilder::BuildTokenBurnTx(const uint256& tokenHash, const CTxDestination& from, CAmount amount)
{
    CTokenTx tokenTx;
    tokenTx.type = TokenTxType::BURN;
    tokenTx.tokenHash = tokenHash;
    tokenTx.from = from;
    tokenTx.amount = amount;
    tokenTx.timestamp = GetTime();
    
    CMutableTransaction tx;
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vout.resize(1);
    
    AddTokenOutput(tx, tokenTx);
    
    return MakeTransactionRef(std::move(tx));
}

CScript CTokenTxBuilder::CreateTokenScript(const CTokenTx& tokenTx)
{
    // Create a script that encodes token transaction data
    CScript script;
    script << OP_RETURN;
    
    // Encode token transaction data
    std::vector<unsigned char> data;
    data.push_back(static_cast<unsigned char>(tokenTx.type));
    
    // Add token hash
    data.insert(data.end(), tokenTx.tokenHash.begin(), tokenTx.tokenHash.end());
    
    // Add amount (8 bytes)
    for (int i = 0; i < 8; i++) {
        data.push_back((tokenTx.amount >> (i * 8)) & 0xFF);
    }
    
    script << data;
    return script;
}

CScript CTokenTxBuilder::CreateTokenCreationScript(const CTokenTx& tokenTx)
{
    CScript script;
    script << OP_RETURN;
    
    std::vector<unsigned char> data;
    data.push_back(static_cast<unsigned char>(TokenTxType::CREATE));
    
    // Add token name and symbol
    data.insert(data.end(), tokenTx.tokenName.begin(), tokenTx.tokenName.end());
    data.push_back(0); // Null terminator
    data.insert(data.end(), tokenTx.tokenSymbol.begin(), tokenTx.tokenSymbol.end());
    data.push_back(0); // Null terminator
    
    // Add decimals and total supply
    data.push_back(tokenTx.tokenDecimals);
    for (int i = 0; i < 8; i++) {
        data.push_back((tokenTx.tokenTotalSupply >> (i * 8)) & 0xFF);
    }
    
    script << data;
    return script;
}

CScript CTokenTxBuilder::CreateTokenTransferScript(const CTokenTx& tokenTx)
{
    return CreateTokenScript(tokenTx);
}

bool CTokenTxBuilder::ValidateTokenScript(const CScript& script)
{
    if (script.size() < 2 || script[0] != OP_RETURN) {
        return false;
    }
    
    // Basic validation - check if it has token data
    return script.size() > 10; // Minimum size for token data
}

bool CTokenTxBuilder::ValidateTokenCreationScript(const CScript& script)
{
    if (script.size() < 2 || script[0] != OP_RETURN) {
        return false;
    }
    
    // Check for creation transaction marker
    if (script.size() < 3) {
        return false;
    }
    
    return script[2] == static_cast<unsigned char>(TokenTxType::CREATE);
}

bool CTokenTxBuilder::ValidateTokenTransferScript(const CScript& script)
{
    return ValidateTokenScript(script);
}

bool CTokenTxBuilder::ParseTokenScript(const CScript& script, CTokenTx& tokenTx)
{
    if (script.size() < 3 || script[0] != OP_RETURN) {
        return false;
    }
    
    // Parse token transaction data
    const std::vector<unsigned char>& data = script[1];
    if (data.size() < 10) {
        return false;
    }
    
    tokenTx.type = static_cast<TokenTxType>(data[0]);
    
    // Parse token hash
    if (data.size() >= 33) {
        std::copy(data.begin() + 1, data.begin() + 33, tokenTx.tokenHash.begin());
    }
    
    // Parse amount
    if (data.size() >= 41) {
        tokenTx.amount = 0;
        for (int i = 0; i < 8; i++) {
            tokenTx.amount |= static_cast<CAmount>(data[33 + i]) << (i * 8);
        }
    }
    
    return true;
}

bool CTokenTxBuilder::ParseTokenCreationScript(const CScript& script, CTokenTx& tokenTx)
{
    if (script.size() < 3 || script[0] != OP_RETURN) {
        return false;
    }
    
    const std::vector<unsigned char>& data = script[1];
    if (data.size() < 2 || data[0] != static_cast<unsigned char>(TokenTxType::CREATE)) {
        return false;
    }
    
    tokenTx.type = TokenTxType::CREATE;
    
    // Parse token name and symbol
    size_t pos = 1;
    while (pos < data.size() && data[pos] != 0) {
        tokenTx.tokenName += static_cast<char>(data[pos]);
        pos++;
    }
    pos++; // Skip null terminator
    
    while (pos < data.size() && data[pos] != 0) {
        tokenTx.tokenSymbol += static_cast<char>(data[pos]);
        pos++;
    }
    pos++; // Skip null terminator
    
    // Parse decimals and total supply
    if (pos < data.size()) {
        tokenTx.tokenDecimals = data[pos++];
    }
    
    if (pos + 7 < data.size()) {
        tokenTx.tokenTotalSupply = 0;
        for (int i = 0; i < 8; i++) {
            tokenTx.tokenTotalSupply |= static_cast<CAmount>(data[pos + i]) << (i * 8);
        }
    }
    
    return true;
}

bool CTokenTxBuilder::ParseTokenTransferScript(const CScript& script, CTokenTx& tokenTx)
{
    return ParseTokenScript(script, tokenTx);
}

void CTokenTxBuilder::AddTokenOutput(CTransaction& tx, const CTokenTx& tokenTx)
{
    tx.vout[0].nValue = 0;
    tx.vout[0].scriptPubKey = CreateTokenScript(tokenTx);
}

void CTokenTxBuilder::AddTokenCreationOutput(CTransaction& tx, const CTokenTx& tokenTx)
{
    tx.vout[0].nValue = 0;
    tx.vout[0].scriptPubKey = CreateTokenCreationScript(tokenTx);
}

void CTokenTxBuilder::AddTokenTransferOutput(CTransaction& tx, const CTokenTx& tokenTx)
{
    tx.vout[0].nValue = 0;
    tx.vout[0].scriptPubKey = CreateTokenTransferScript(tokenTx);
}

// Utility functions implementation
namespace TokenUtils {
    bool InitializeTokenSystem()
    {
        if (!g_tokenManager) {
            g_tokenManager = std::make_unique<CTokenManager>();
        }
        LogPrint(BCLog::TOKENS, "Token system initialized\n");
        return true;
    }
    
    void ShutdownTokenSystem()
    {
        if (g_tokenManager) {
            g_tokenManager->LogTokenStats();
            g_tokenManager.reset();
        }
        LogPrint(BCLog::TOKENS, "Token system shutdown\n");
    }
    
    bool IsTokenSystemEnabled()
    {
        return g_tokenManager != nullptr;
    }
    
    void SetTokenSystemEnabled(bool enabled)
    {
        if (enabled && !g_tokenManager) {
            InitializeTokenSystem();
        } else if (!enabled && g_tokenManager) {
            ShutdownTokenSystem();
        }
    }
    
    bool CanCreateToken(const CTxDestination& creator, CAmount fee)
    {
        if (!g_tokenManager) {
            return false;
        }
        
        // Check if creator has sufficient SHAH for fee
        // This would need to be implemented with wallet integration
        return true;
    }
    
    bool CanTransferTokens(const uint256& tokenHash, const CTxDestination& from, CAmount amount)
    {
        if (!g_tokenManager) {
            return false;
        }
        
        CAmount balance = g_tokenManager->GetTokenBalance(tokenHash, from);
        return balance >= amount;
    }
    
    bool CanMintTokens(const uint256& tokenHash, const CTxDestination& minter)
    {
        if (!g_tokenManager) {
            return false;
        }
        
        // Check if minter is the token creator
        CTokenInfo token = g_tokenManager->GetToken(tokenHash);
        return token.creator == minter;
    }
    
    CAmount GetTokenCreationFee()
    {
        if (!g_tokenManager) {
            return 500 * COIN; // Default fee
        }
        return g_tokenManager->CalculateTokenCreationFee();
    }
    
    bool IsTokenTransaction(const CTransaction& tx)
    {
        if (!g_tokenManager) {
            return false;
        }
        return g_tokenManager->IsTokenTransaction(tx);
    }
    
    bool ValidateTokenTransaction(const CTransaction& tx)
    {
        if (!g_tokenManager) {
            return false;
        }
        return g_tokenManager->ValidateTokenTransaction(tx);
    }
    
    std::string GetTokenTxTypeName(TokenTxType type)
    {
        switch (type) {
            case TokenTxType::CREATE: return "CREATE";
            case TokenTxType::TRANSFER: return "TRANSFER";
            case TokenTxType::MINT: return "MINT";
            case TokenTxType::BURN: return "BURN";
            case TokenTxType::APPROVE: return "APPROVE";
            case TokenTxType::TRANSFER_FROM: return "TRANSFER_FROM";
            default: return "UNKNOWN";
        }
    }
    
    uint256 GenerateTokenAddress(const CTxDestination& creator, const std::string& name, 
                                const std::string& symbol, int64_t timestamp)
    {
        CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
        hasher << creator << name << symbol << timestamp;
        return hasher.GetHash();
    }
}
