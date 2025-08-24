// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <dex/dex.h>
#include <chain.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/validation.h>
#include <key.h>
#include <logging.h>
#include <script/standard.h>
#include <serialize.h>
#include <util/time.h>
#include <validation.h>

#include <crypto/sha256.h>

// Global DEX manager instance
std::unique_ptr<CDEXManager> g_dexManager = std::make_unique<CDEXManager>();

CDEXManager::CDEXManager() : nextPairId(uint256::ZERO), nextOrderId(uint256::ZERO) {
}

bool CDEXManager::CreateTradingPair(const uint256& tokenA, const uint256& tokenB,
                                   const std::string& symbol, CValidationState& state) {
    LOCK(cs_dex);
    
    if (!DEXValidation::ValidateTradingPairCreation(tokenA, tokenB, symbol, state)) {
        return false;
    }
    
    // Check if symbol already exists
    if (mapSymbolToPairId.find(symbol) != mapSymbolToPairId.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-symbol-exists", 
                           "Trading pair symbol already exists");
    }
    
    // Generate unique pair ID
    uint256 pairId = GeneratePairId(tokenA, tokenB);
    
    // Create trading pair
    CTradingPair pair(tokenA, tokenB, symbol);
    pair.creationTime = GetTime();
    mapTradingPairs[pairId] = pair;
    mapSymbolToPairId[symbol] = pairId;
    
    LogPrint(BCLog::DEX, "Created trading pair %s (%s)\n", symbol, pairId.ToString());
    return true;
}

bool CDEXManager::RemoveTradingPair(const uint256& pairId, CValidationState& state) {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    // Check if pair has liquidity
    if (it->second.totalLiquidity > 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-has-liquidity", 
                           "Cannot remove pair with existing liquidity");
    }
    
    // Remove pair
    mapSymbolToPairId.erase(it->second.symbol);
    mapTradingPairs.erase(it);
    
    LogPrint(BCLog::DEX, "Removed trading pair %s\n", pairId.ToString());
    return true;
}

bool CDEXManager::AddLiquidity(const uint256& pairId, const CTxDestination& provider,
                              uint64_t amountA, uint64_t amountB, const uint256& txHash,
                              CValidationState& state) {
    LOCK(cs_dex);
    
    auto pairIt = mapTradingPairs.find(pairId);
    if (pairIt == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    if (!ValidateLiquidityAddition(pairId, amountA, amountB, state)) {
        return false;
    }
    
    // Calculate liquidity tokens to mint
    uint64_t liquidityTokens = CalculateLiquidityTokens(pairId, amountA, amountB);
    
    // Update reserves
    if (!UpdateReserves(pairId, static_cast<int64_t>(amountA), static_cast<int64_t>(amountB), state)) {
        return false;
    }
    
    // Update liquidity position
    if (!UpdateLiquidityPosition(pairId, provider, static_cast<int64_t>(liquidityTokens), 
                                static_cast<int64_t>(amountA), static_cast<int64_t>(amountB), state)) {
        return false;
    }
    
    LogPrint(BCLog::DEX, "Added liquidity %d tokens to pair %s by %s\n", 
             liquidityTokens, pairId.ToString(), EncodeDestination(provider));
    return true;
}

bool CDEXManager::RemoveLiquidity(const uint256& pairId, const CTxDestination& provider,
                                 uint64_t liquidityTokens, const uint256& txHash,
                                 CValidationState& state) {
    LOCK(cs_dex);
    
    auto pairIt = mapTradingPairs.find(pairId);
    if (pairIt == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    // Get current liquidity position
    auto posKey = std::make_pair(pairId, provider);
    auto posIt = mapLiquidityPositions.find(posKey);
    if (posIt == mapLiquidityPositions.end() || posIt->second.liquidityTokens < liquidityTokens) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "insufficient-liquidity", 
                           "Insufficient liquidity tokens");
    }
    
    // Calculate amounts to return
    auto [amountA, amountB] = CalculateLiquidityRemoval(pairId, liquidityTokens);
    
    // Update reserves (negative amounts)
    if (!UpdateReserves(pairId, -static_cast<int64_t>(amountA), -static_cast<int64_t>(amountB), state)) {
        return false;
    }
    
    // Update liquidity position (negative amounts)
    if (!UpdateLiquidityPosition(pairId, provider, -static_cast<int64_t>(liquidityTokens), 
                                -static_cast<int64_t>(amountA), -static_cast<int64_t>(amountB), state)) {
        return false;
    }
    
    LogPrint(BCLog::DEX, "Removed liquidity %d tokens from pair %s by %s\n", 
             liquidityTokens, pairId.ToString(), EncodeDestination(provider));
    return true;
}

bool CDEXManager::CreateSwapOrder(const uint256& pairId, const CTxDestination& trader,
                                bool isBuy, uint64_t amountIn, uint64_t amountOutMin,
                                uint32_t deadline, const uint256& txHash,
                                CValidationState& state) {
    LOCK(cs_dex);
    
    auto pairIt = mapTradingPairs.find(pairId);
    if (pairIt == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    if (!ValidateSwapParameters(amountIn, amountOutMin, deadline, state)) {
        return false;
    }
    
    // Generate order ID
    uint256 orderId = GenerateOrderId(trader, GetTime());
    
    // Create swap order
    CSwapOrder order(orderId, pairId, trader, isBuy, amountIn, amountOutMin, deadline, GetTime());
    mapSwapOrders[orderId] = order;
    
    LogPrint(BCLog::DEX, "Created swap order %s for pair %s by %s\n", 
             orderId.ToString(), pairId.ToString(), EncodeDestination(trader));
    return true;
}

bool CDEXManager::ExecuteSwapOrder(const uint256& orderId, const uint256& txHash,
                                 CValidationState& state) {
    LOCK(cs_dex);
    
    auto orderIt = mapSwapOrders.find(orderId);
    if (orderIt == mapSwapOrders.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "order-not-found", 
                           "Swap order does not exist");
    }
    
    CSwapOrder& order = orderIt->second;
    if (order.isExecuted) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "order-already-executed", 
                           "Swap order already executed");
    }
    
    // Check deadline
    if (GetTime() > order.deadline) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "order-expired", 
                           "Swap order expired");
    }
    
    // Calculate output amount
    uint64_t amountOut = CalculateSwapOutput(order.pairId, order.isBuy, order.amountIn);
    
    // Check slippage
    if (amountOut < order.amountOutMin) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "slippage-too-high", 
                           "Slippage too high");
    }
    
    // Update reserves
    if (order.isBuy) {
        // Buying token B with token A
        if (!UpdateReserves(order.pairId, static_cast<int64_t>(order.amountIn), 
                           -static_cast<int64_t>(amountOut), state)) {
            return false;
        }
    } else {
        // Selling token B for token A
        if (!UpdateReserves(order.pairId, -static_cast<int64_t>(amountOut), 
                           static_cast<int64_t>(order.amountIn), state)) {
            return false;
        }
    }
    
    // Mark order as executed
    order.isExecuted = true;
    
    LogPrint(BCLog::DEX, "Executed swap order %s: %d tokens swapped\n", 
             orderId.ToString(), amountOut);
    return true;
}

uint64_t CDEXManager::CalculateSwapOutput(const uint256& pairId, bool isBuy, uint64_t amountIn) const {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return 0;
    }
    
    const CTradingPair& pair = it->second;
    if (pair.reserveA == 0 || pair.reserveB == 0) {
        return 0;
    }
    
    // Constant Product Market Maker (CPMM) formula
    // (x + dx) * (y - dy) = x * y
    // dy = y * dx / (x + dx)
    
    uint64_t reserveIn, reserveOut;
    if (isBuy) {
        reserveIn = pair.reserveA;
        reserveOut = pair.reserveB;
    } else {
        reserveIn = pair.reserveB;
        reserveOut = pair.reserveA;
    }
    
    // Calculate fee
    uint64_t fee = (amountIn * DEX_SWAP_FEE_RATE) / 10000; // 0.1% fee
    uint64_t amountInAfterFee = amountIn - fee;
    
    // Calculate output using CPMM formula
    uint64_t numerator = amountInAfterFee * reserveOut;
    uint64_t denominator = reserveIn + amountInAfterFee;
    
    if (denominator == 0) {
        return 0;
    }
    
    return numerator / denominator;
}

uint64_t CDEXManager::CalculateLiquidityTokens(const uint256& pairId, uint64_t amountA, uint64_t amountB) const {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return 0;
    }
    
    const CTradingPair& pair = it->second;
    
    if (pair.totalLiquidity == 0) {
        // First liquidity provider
        return static_cast<uint64_t>(sqrt(static_cast<double>(amountA) * amountB));
    }
    
    // Calculate proportional to existing liquidity
    uint64_t liquidityA = (amountA * pair.totalLiquidity) / pair.reserveA;
    uint64_t liquidityB = (amountB * pair.totalLiquidity) / pair.reserveB;
    
    return std::min(liquidityA, liquidityB);
}

std::pair<uint64_t, uint64_t> CDEXManager::CalculateLiquidityRemoval(const uint256& pairId, 
                                                                    uint64_t liquidityTokens) const {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return {0, 0};
    }
    
    const CTradingPair& pair = it->second;
    
    if (pair.totalLiquidity == 0) {
        return {0, 0};
    }
    
    // Calculate proportional amounts
    uint64_t amountA = (liquidityTokens * pair.reserveA) / pair.totalLiquidity;
    uint64_t amountB = (liquidityTokens * pair.reserveB) / pair.totalLiquidity;
    
    return {amountA, amountB};
}

CTradingPair CDEXManager::GetTradingPair(const uint256& pairId) const {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it != mapTradingPairs.end()) {
        return it->second;
    }
    return CTradingPair();
}

CTradingPair CDEXManager::GetTradingPairBySymbol(const std::string& symbol) const {
    LOCK(cs_dex);
    
    auto it = mapSymbolToPairId.find(symbol);
    if (it != mapSymbolToPairId.end()) {
        return GetTradingPair(it->second);
    }
    return CTradingPair();
}

CLiquidityPosition CDEXManager::GetLiquidityPosition(const uint256& pairId, const CTxDestination& owner) const {
    LOCK(cs_dex);
    
    auto it = mapLiquidityPositions.find(std::make_pair(pairId, owner));
    if (it != mapLiquidityPositions.end()) {
        return it->second;
    }
    return CLiquidityPosition();
}

std::vector<CLiquidityPosition> CDEXManager::GetLiquidityPositions(const CTxDestination& owner) const {
    LOCK(cs_dex);
    
    std::vector<CLiquidityPosition> positions;
    for (const auto& pair : mapLiquidityPositions) {
        if (pair.first.second == owner) {
            positions.push_back(pair.second);
        }
    }
    return positions;
}

CSwapOrder CDEXManager::GetSwapOrder(const uint256& orderId) const {
    LOCK(cs_dex);
    
    auto it = mapSwapOrders.find(orderId);
    if (it != mapSwapOrders.end()) {
        return it->second;
    }
    return CSwapOrder();
}

std::vector<CSwapOrder> CDEXManager::GetSwapOrders(const CTxDestination& trader) const {
    LOCK(cs_dex);
    
    std::vector<CSwapOrder> orders;
    for (const auto& pair : mapSwapOrders) {
        if (pair.second.trader == trader) {
            orders.push_back(pair.second);
        }
    }
    return orders;
}

std::vector<CTradingPair> CDEXManager::GetAllTradingPairs() const {
    LOCK(cs_dex);
    
    std::vector<CTradingPair> pairs;
    for (const auto& pair : mapTradingPairs) {
        pairs.push_back(pair.second);
    }
    return pairs;
}

bool CDEXManager::IsValidTradingPair(const uint256& pairId) const {
    LOCK(cs_dex);
    return mapTradingPairs.find(pairId) != mapTradingPairs.end();
}

bool CDEXManager::HasSufficientLiquidity(const uint256& pairId, uint64_t amount) const {
    LOCK(cs_dex);
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return false;
    }
    
    return it->second.reserveA >= amount || it->second.reserveB >= amount;
}

bool CDEXManager::ValidateSwapOrder(const CSwapOrder& order, CValidationState& state) const {
    if (order.amountIn == 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "zero-amount", 
                           "Swap amount cannot be zero");
    }
    
    if (order.amountOutMin == 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "zero-min-out", 
                           "Minimum output cannot be zero");
    }
    
    if (order.deadline <= GetTime()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "past-deadline", 
                           "Deadline must be in the future");
    }
    
    return true;
}

bool CDEXManager::ValidateLiquidityAddition(const uint256& pairId, uint64_t amountA, uint64_t amountB,
                                          CValidationState& state) const {
    if (amountA == 0 || amountB == 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "zero-liquidity", 
                           "Liquidity amounts cannot be zero");
    }
    
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    // Check minimum liquidity
    if (amountA + amountB < MIN_LIQUIDITY_AMOUNT) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "insufficient-liquidity", 
                           "Insufficient liquidity amount");
    }
    
    return true;
}

void CDEXManager::Serialize(CAutoFile& file) const {
    LOCK(cs_dex);
    
    file << mapTradingPairs;
    file << mapLiquidityPositions;
    file << mapSwapOrders;
    file << mapSymbolToPairId;
    file << nextPairId;
    file << nextOrderId;
}

void CDEXManager::Unserialize(CAutoFile& file) {
    LOCK(cs_dex);
    
    file >> mapTradingPairs;
    file >> mapLiquidityPositions;
    file >> mapSwapOrders;
    file >> mapSymbolToPairId;
    file >> nextPairId;
    file >> nextOrderId;
}

uint256 CDEXManager::GeneratePairId(const uint256& tokenA, const uint256& tokenB) {
    CHashWriter ss(SER_GETHASH, 0);
    ss << tokenA;
    ss << tokenB;
    ss << GetTime();
    ss << nextPairId;
    
    nextPairId = ss.GetHash();
    return nextPairId;
}

uint256 CDEXManager::GenerateOrderId(const CTxDestination& trader, uint32_t timestamp) {
    CHashWriter ss(SER_GETHASH, 0);
    ss << trader;
    ss << timestamp;
    ss << nextOrderId;
    
    nextOrderId = ss.GetHash();
    return nextOrderId;
}

bool CDEXManager::UpdateReserves(const uint256& pairId, int64_t deltaA, int64_t deltaB,
                               CValidationState& state) {
    auto it = mapTradingPairs.find(pairId);
    if (it == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    CTradingPair& pair = it->second;
    
    // Check for negative reserves
    if (static_cast<int64_t>(pair.reserveA) + deltaA < 0 ||
        static_cast<int64_t>(pair.reserveB) + deltaB < 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "negative-reserves", 
                           "Cannot have negative reserves");
    }
    
    pair.reserveA = static_cast<uint64_t>(static_cast<int64_t>(pair.reserveA) + deltaA);
    pair.reserveB = static_cast<uint64_t>(static_cast<int64_t>(pair.reserveB) + deltaB);
    
    return true;
}

bool CDEXManager::UpdateLiquidityPosition(const uint256& pairId, const CTxDestination& owner,
                                        int64_t deltaLiquidity, int64_t deltaA, int64_t deltaB,
                                        CValidationState& state) {
    auto pairIt = mapTradingPairs.find(pairId);
    if (pairIt == mapTradingPairs.end()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "pair-not-found", 
                           "Trading pair does not exist");
    }
    
    auto posKey = std::make_pair(pairId, owner);
    auto posIt = mapLiquidityPositions.find(posKey);
    
    if (posIt == mapLiquidityPositions.end()) {
        if (deltaLiquidity < 0) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "no-position", 
                               "No liquidity position to remove");
        }
        if (deltaLiquidity > 0) {
            CLiquidityPosition position(pairId, owner, static_cast<uint64_t>(deltaLiquidity),
                                      static_cast<uint64_t>(deltaA), static_cast<uint64_t>(deltaB), GetTime());
            mapLiquidityPositions[posKey] = position;
        }
    } else {
        CLiquidityPosition& position = posIt->second;
        
        int64_t newLiquidity = static_cast<int64_t>(position.liquidityTokens) + deltaLiquidity;
        int64_t newAmountA = static_cast<int64_t>(position.tokenAAmount) + deltaA;
        int64_t newAmountB = static_cast<int64_t>(position.tokenBAmount) + deltaB;
        
        if (newLiquidity < 0 || newAmountA < 0 || newAmountB < 0) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "negative-position", 
                               "Cannot have negative position values");
        }
        
        if (newLiquidity == 0) {
            mapLiquidityPositions.erase(posIt);
        } else {
            position.liquidityTokens = static_cast<uint64_t>(newLiquidity);
            position.tokenAAmount = static_cast<uint64_t>(newAmountA);
            position.tokenBAmount = static_cast<uint64_t>(newAmountB);
        }
    }
    
    // Update total liquidity in trading pair
    CTradingPair& pair = pairIt->second;
    int64_t newTotalLiquidity = static_cast<int64_t>(pair.totalLiquidity) + deltaLiquidity;
    if (newTotalLiquidity < 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "negative-total-liquidity", 
                           "Cannot have negative total liquidity");
    }
    pair.totalLiquidity = static_cast<uint64_t>(newTotalLiquidity);
    
    return true;
}

// DEX validation functions
namespace DEXValidation {

bool ValidateDEXTx(const CTransaction& tx, const CCoinsViewCache& view,
                  CValidationState& state) {
    if (tx.vout.empty()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "no-outputs", 
                           "DEX transaction must have outputs");
    }
    
    // Check if first output contains DEX data
    CDEXTxData dexData;
    if (!ExtractDEXData(tx.vout[0].scriptPubKey, dexData)) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-dex-data", 
                           "Invalid DEX transaction data");
    }
    
    // Validate based on transaction type
    switch (dexData.type) {
        case DEX_TX_CREATE_PAIR:
            return ValidateTradingPairCreation(dexData.pairId, dexData.pairId, 
                                             dexData.extraData, state);
        case DEX_TX_SWAP:
            return ValidateSwapParameters(dexData.amountA, dexData.amountOutMin, 
                                        dexData.deadline, state);
        default:
            return true;
    }
}

bool ExtractDEXData(const CScript& script, CDEXTxData& data) {
    // Parse script to extract DEX transaction data
    // This is a simplified implementation - in practice, you'd need more sophisticated parsing
    
    if (script.size() < 10) {
        return false;
    }
    
    // Look for DEX transaction marker
    std::vector<unsigned char> dexMarker = {0x44, 0x45, 0x58}; // "DEX"
    auto pos = std::search(script.begin(), script.end(), dexMarker.begin(), dexMarker.end());
    
    if (pos == script.end()) {
        return false;
    }
    
    // Extract transaction type
    if (pos + 3 < script.end()) {
        data.type = static_cast<DEXTxType>(*(pos + 3));
    }
    
    // Extract other data (simplified)
    // In a real implementation, you'd parse the full script structure
    
    return true;
}

CScript CreateDEXScript(const CDEXTxData& data) {
    CScript script;
    
    // Add DEX transaction marker
    script << std::vector<unsigned char>{0x44, 0x45, 0x58}; // "DEX"
    
    // Add transaction type
    script << static_cast<uint8_t>(data.type);
    
    // Add pair ID
    script << data.pairId;
    
    // Add order ID
    script << data.orderId;
    
    // Add trader address
    script << GetScriptForDestination(data.trader);
    
    // Add amounts
    script << data.amountA;
    script << data.amountB;
    script << data.liquidityTokens;
    
    // Add swap parameters
    script << data.isBuy;
    script << data.amountOutMin;
    script << data.deadline;
    
    // Add extra data
    if (!data.extraData.empty()) {
        script << data.extraData;
    }
    
    return script;
}

bool IsDEXTransaction(const CTransaction& tx) {
    if (tx.vout.empty()) {
        return false;
    }
    
    // Check if any output contains DEX data
    for (const CTxOut& txout : tx.vout) {
        CDEXTxData dexData;
        if (ExtractDEXData(txout.scriptPubKey, dexData)) {
            return true;
        }
    }
    
    return false;
}

bool ValidateTradingPairCreation(const uint256& tokenA, const uint256& tokenB,
                               const std::string& symbol, CValidationState& state) {
    if (tokenA == tokenB) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "same-tokens", 
                           "Cannot create pair with same tokens");
    }
    
    if (symbol.empty() || symbol.length() > 32) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "invalid-symbol", 
                           "Invalid trading pair symbol");
    }
    
    return true;
}

bool ValidateSwapParameters(uint64_t amountIn, uint64_t amountOutMin, uint32_t deadline,
                          CValidationState& state) {
    if (amountIn == 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "zero-amount", 
                           "Swap amount cannot be zero");
    }
    
    if (amountOutMin == 0) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "zero-min-out", 
                           "Minimum output cannot be zero");
    }
    
    if (deadline <= GetTime()) {
        return state.Invalid(TxValidationResult::TX_CONSENSUS, "past-deadline", 
                           "Deadline must be in the future");
    }
    
    return true;
}

} // namespace DEXValidation
