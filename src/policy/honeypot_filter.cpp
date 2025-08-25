// Copyright (C) 2025 The SHAHCOIN Core Developers// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <policy/honeypot_filter.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <script/standard.h>
#include <logging.h>
#include <util/time.h>
#include <util/strencodings.h>
#include <crypto/sha256.h>

#include <algorithm>
#include <sstream>

// Global honeypot filter manager instance
std::unique_ptr<CHoneypotFilterManager> g_honeypotFilter = std::make_unique<CHoneypotFilterManager>();

// CHoneypotFilterManager implementation
CHoneypotFilterManager::CHoneypotFilterManager()
{
    m_stats = {0, 0, 0, {}, GetTime()};
    
    // Initialize known spam patterns
    m_knownSpamPatterns = {
        "OP_DUP OP_HASH160",           // Common spam pattern
        "OP_EQUALVERIFY OP_CHECKSIG",  // Another common pattern
        "OP_RETURN OP_0",              // Empty OP_RETURN
        "OP_RETURN OP_1",              // Single byte OP_RETURN
    };
    
    // Initialize known exploit patterns
    m_knownExploitPatterns = {
        "OP_VERIFY OP_VERIFY",         // Double verify exploit
        "OP_IF OP_VERIFY OP_ENDIF",    // Conditional verify exploit
        "OP_DUP OP_DUP OP_DUP",        // Excessive duplication
        "OP_HASH160 OP_HASH160",       // Double hashing exploit
    };
}

CHoneypotFilterManager::~CHoneypotFilterManager()
{
}

void CHoneypotFilterManager::SetConfig(const HoneypotFilterConfig& config)
{
    m_config = config;
    LogPrint(BCLog::POLICY, "Honeypot filter configuration updated\n");
}

HoneypotFilterResult CHoneypotFilterManager::CheckTransaction(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.timestamp = GetTime();
    
    m_stats.totalTransactions++;
    
    // Check OP_RETURN outputs
    result = CheckOpReturnOutputs(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check script validation
    result = CheckScriptValidation(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check dust outputs
    result = CheckDustOutputs(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check input validation
    result = CheckInputValidation(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check spam patterns
    result = CheckSpamPatterns(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check exploit attempts
    result = CheckExploitAttempts(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check transaction size
    result = CheckTransactionSize(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    // Check rate limiting
    result = CheckRateLimiting(tx);
    if (result.isSuspicious) {
        m_stats.suspiciousTransactions++;
        m_stats.filterTypeCounts[result.filterType]++;
        LogSuspiciousTransaction(tx, result);
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckOpReturnOutputs(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::EXCESSIVE_OP_RETURN;
    
    size_t opReturnCount = 0;
    size_t totalOpReturnSize = 0;
    
    for (const auto& output : tx.vout) {
        if (IsOpReturnOutput(output.scriptPubKey)) {
            opReturnCount++;
            totalOpReturnSize += output.scriptPubKey.size();
            
            // Check individual OP_RETURN size
            if (output.scriptPubKey.size() > m_config.maxOpReturnSize) {
                result.isSuspicious = true;
                result.reason = "OP_RETURN output exceeds maximum size";
                result.details.push_back("Size: " + std::to_string(output.scriptPubKey.size()) + 
                                       ", Max: " + std::to_string(m_config.maxOpReturnSize));
                return result;
            }
        }
    }
    
    // Check OP_RETURN count
    if (opReturnCount > m_config.maxOpReturnOutputs) {
        result.isSuspicious = true;
        result.reason = "Transaction has too many OP_RETURN outputs";
        result.details.push_back("Count: " + std::to_string(opReturnCount) + 
                               ", Max: " + std::to_string(m_config.maxOpReturnOutputs));
        return result;
    }
    
    // Check total OP_RETURN size
    if (totalOpReturnSize > m_config.maxTotalOpReturnSize) {
        result.isSuspicious = true;
        result.reason = "Total OP_RETURN data size exceeds limit";
        result.details.push_back("Total size: " + std::to_string(totalOpReturnSize) + 
                               ", Max: " + std::to_string(m_config.maxTotalOpReturnSize));
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckScriptValidation(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::INVALID_SCRIPT;
    
    if (!m_config.strictScriptValidation) {
        return result;
    }
    
    // Check input scripts
    for (size_t i = 0; i < tx.vin.size(); i++) {
        const auto& input = tx.vin[i];
        
        if (input.scriptSig.size() > m_config.maxScriptSize) {
            result.isSuspicious = true;
            result.reason = "Input script size exceeds limit";
            result.details.push_back("Input " + std::to_string(i) + ": " + 
                                   std::to_string(input.scriptSig.size()) + " bytes");
            return result;
        }
        
        if (IsSuspiciousScript(input.scriptSig)) {
            result.isSuspicious = true;
            result.reason = "Suspicious input script detected";
            result.details.push_back("Input " + std::to_string(i) + " matches suspicious pattern");
            return result;
        }
    }
    
    // Check output scripts
    for (size_t i = 0; i < tx.vout.size(); i++) {
        const auto& output = tx.vout[i];
        
        if (output.scriptPubKey.size() > m_config.maxScriptSize) {
            result.isSuspicious = true;
            result.reason = "Output script size exceeds limit";
            result.details.push_back("Output " + std::to_string(i) + ": " + 
                                   std::to_string(output.scriptPubKey.size()) + " bytes");
            return result;
        }
        
        if (IsSuspiciousScript(output.scriptPubKey)) {
            result.isSuspicious = true;
            result.reason = "Suspicious output script detected";
            result.details.push_back("Output " + std::to_string(i) + " matches suspicious pattern");
            return result;
        }
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckDustOutputs(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::SUSPICIOUS_DUST;
    
    size_t dustCount = 0;
    
    for (const auto& output : tx.vout) {
        if (IsDustAmount(output.nValue)) {
            dustCount++;
        }
    }
    
    if (dustCount > m_config.maxDustOutputs) {
        result.isSuspicious = true;
        result.reason = "Transaction has too many dust outputs";
        result.details.push_back("Dust outputs: " + std::to_string(dustCount) + 
                               ", Max: " + std::to_string(m_config.maxDustOutputs));
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckInputValidation(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::MALFORMED_INPUT;
    
    // Check input count
    if (tx.vin.size() > m_config.maxInputCount) {
        result.isSuspicious = true;
        result.reason = "Transaction has too many inputs";
        result.details.push_back("Inputs: " + std::to_string(tx.vin.size()) + 
                               ", Max: " + std::to_string(m_config.maxInputCount));
        return result;
    }
    
    // Check for null inputs (coinbase transactions are exempt)
    if (!tx.IsCoinBase()) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            const auto& input = tx.vin[i];
            if (input.prevout.IsNull()) {
                result.isSuspicious = true;
                result.reason = "Transaction has null input";
                result.details.push_back("Input " + std::to_string(i) + " is null");
                return result;
            }
        }
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckSpamPatterns(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::SPAM_PATTERN;
    
    if (IsKnownSpamPattern(tx)) {
        result.isSuspicious = true;
        result.reason = "Transaction matches known spam pattern";
        result.details.push_back("Pattern detected in transaction structure");
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckExploitAttempts(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::EXPLOIT_ATTEMPT;
    
    if (IsKnownExploitPattern(tx)) {
        result.isSuspicious = true;
        result.reason = "Transaction matches known exploit pattern";
        result.details.push_back("Exploit pattern detected in transaction");
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckTransactionSize(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::EXCESSIVE_SIZE;
    
    size_t txSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
    
    if (txSize > m_config.maxTransactionSize) {
        result.isSuspicious = true;
        result.reason = "Transaction size exceeds limit";
        result.details.push_back("Size: " + std::to_string(txSize) + 
                               " bytes, Max: " + std::to_string(m_config.maxTransactionSize));
        return result;
    }
    
    if (tx.vout.size() > m_config.maxOutputCount) {
        result.isSuspicious = true;
        result.reason = "Transaction has too many outputs";
        result.details.push_back("Outputs: " + std::to_string(tx.vout.size()) + 
                               ", Max: " + std::to_string(m_config.maxOutputCount));
        return result;
    }
    
    return result;
}

HoneypotFilterResult CHoneypotFilterManager::CheckRateLimiting(const CTransaction& tx)
{
    HoneypotFilterResult result;
    result.filterType = HoneypotFilterType::RATE_LIMIT_VIOLATION;
    
    std::string source = GetTransactionSource(tx);
    int64_t currentTime = GetTime();
    
    UpdateRateLimitTracker(source, currentTime);
    
    if (IsRateLimitExceeded(source)) {
        result.isSuspicious = true;
        result.reason = "Rate limit exceeded for transaction source";
        result.details.push_back("Source: " + source);
        result.details.push_back("Window: " + std::to_string(m_config.rateLimitWindow) + " seconds");
        return result;
    }
    
    // Cleanup old entries
    CleanupRateLimitTracker(currentTime);
    
    return result;
}

CHoneypotFilterManager::FilterStats CHoneypotFilterManager::GetStats() const
{
    return m_stats;
}

void CHoneypotFilterManager::ResetStats()
{
    m_stats = {0, 0, 0, {}, GetTime()};
    LogPrint(BCLog::POLICY, "Honeypot filter statistics reset\n");
}

void CHoneypotFilterManager::LogSuspiciousTransaction(const CTransaction& tx, const HoneypotFilterResult& result)
{
    LogPrint(BCLog::POLICY, "Honeypot filter: Suspicious transaction detected\n");
    LogPrint(BCLog::POLICY, "  TXID: %s\n", tx.GetHash().ToString());
    LogPrint(BCLog::POLICY, "  Type: %s\n", HoneypotUtils::GetFilterTypeName(result.filterType));
    LogPrint(BCLog::POLICY, "  Reason: %s\n", result.reason);
    
    for (const auto& detail : result.details) {
        LogPrint(BCLog::POLICY, "  Detail: %s\n", detail);
    }
}

void CHoneypotFilterManager::LogFilterStats()
{
    LogPrint(BCLog::POLICY, "Honeypot filter statistics:\n");
    LogPrint(BCLog::POLICY, "  Total transactions: %d\n", m_stats.totalTransactions);
    LogPrint(BCLog::POLICY, "  Suspicious transactions: %d\n", m_stats.suspiciousTransactions);
    LogPrint(BCLog::POLICY, "  Blocked transactions: %d\n", m_stats.blockedTransactions);
    
    for (const auto& pair : m_stats.filterTypeCounts) {
        LogPrint(BCLog::POLICY, "  %s: %d\n", 
                HoneypotUtils::GetFilterTypeName(pair.first), pair.second);
    }
}

bool CHoneypotFilterManager::IsOpReturnOutput(const CScript& script) const
{
    return script.size() >= 1 && script[0] == OP_RETURN;
}

bool CHoneypotFilterManager::IsDustAmount(CAmount amount) const
{
    return amount < m_config.minDustThreshold;
}

bool CHoneypotFilterManager::IsSuspiciousScript(const CScript& script) const
{
    // Check for excessive OP codes
    size_t opCount = 0;
    for (const auto& op : script) {
        if (op >= 0 && op <= OP_PUSHDATA4) {
            opCount++;
        }
    }
    
    if (opCount > 100) { // Arbitrary limit
        return true;
    }
    
    // Check for suspicious patterns
    std::string pattern = ExtractScriptPattern(script);
    return MatchesPattern(script, m_knownSpamPatterns) || 
           MatchesPattern(script, m_knownExploitPatterns);
}

bool CHoneypotFilterManager::IsKnownSpamPattern(const CTransaction& tx) const
{
    // Check for common spam patterns
    for (const auto& output : tx.vout) {
        if (MatchesPattern(output.scriptPubKey, m_knownSpamPatterns)) {
            return true;
        }
    }
    
    for (const auto& input : tx.vin) {
        if (MatchesPattern(input.scriptSig, m_knownSpamPatterns)) {
            return true;
        }
    }
    
    return false;
}

bool CHoneypotFilterManager::IsKnownExploitPattern(const CTransaction& tx) const
{
    // Check for known exploit patterns
    for (const auto& output : tx.vout) {
        if (MatchesPattern(output.scriptPubKey, m_knownExploitPatterns)) {
            return true;
        }
    }
    
    for (const auto& input : tx.vin) {
        if (MatchesPattern(input.scriptSig, m_knownExploitPatterns)) {
            return true;
        }
    }
    
    return false;
}

std::string CHoneypotFilterManager::GetTransactionSource(const CTransaction& tx) const
{
    // For simplicity, use the first input's previous transaction hash as source
    // In a real implementation, you might want to use the actual source address
    if (!tx.vin.empty() && !tx.IsCoinBase()) {
        return tx.vin[0].prevout.hash.ToString();
    }
    return "unknown";
}

void CHoneypotFilterManager::UpdateRateLimitTracker(const std::string& source, int64_t timestamp)
{
    m_rateLimitTracker[source].push_back(timestamp);
}

bool CHoneypotFilterManager::IsRateLimitExceeded(const std::string& source) const
{
    auto it = m_rateLimitTracker.find(source);
    if (it == m_rateLimitTracker.end()) {
        return false;
    }
    
    int64_t currentTime = GetTime();
    const auto& timestamps = it->second;
    
    // Count transactions within the rate limit window
    size_t recentCount = 0;
    for (int64_t ts : timestamps) {
        if (currentTime - ts <= m_config.rateLimitWindow) {
            recentCount++;
        }
    }
    
    return recentCount > m_config.maxTransactionsPerBlock;
}

void CHoneypotFilterManager::CleanupRateLimitTracker(int64_t currentTime)
{
    for (auto& pair : m_rateLimitTracker) {
        auto& timestamps = pair.second;
        timestamps.erase(
            std::remove_if(timestamps.begin(), timestamps.end(),
                [currentTime, this](int64_t ts) {
                    return currentTime - ts > m_config.rateLimitWindow;
                }),
            timestamps.end()
        );
    }
}

bool CHoneypotFilterManager::MatchesPattern(const CScript& script, const std::set<std::string>& patterns) const
{
    std::string scriptPattern = ExtractScriptPattern(script);
    return patterns.find(scriptPattern) != patterns.end();
}

std::string CHoneypotFilterManager::ExtractScriptPattern(const CScript& script) const
{
    std::stringstream ss;
    for (const auto& op : script) {
        if (op >= 0 && op <= OP_PUSHDATA4) {
            ss << "OP_" << GetOpName(op) << " ";
        }
    }
    return ss.str();
}

// Utility functions implementation
namespace HoneypotUtils {
    bool InitializeHoneypotFiltering()
    {
        if (!g_honeypotFilter) {
            g_honeypotFilter = std::make_unique<CHoneypotFilterManager>();
        }
        LogPrint(BCLog::POLICY, "Honeypot filtering system initialized\n");
        return true;
    }
    
    void ShutdownHoneypotFiltering()
    {
        if (g_honeypotFilter) {
            g_honeypotFilter->LogFilterStats();
            g_honeypotFilter.reset();
        }
        LogPrint(BCLog::POLICY, "Honeypot filtering system shutdown\n");
    }
    
    bool ShouldRejectTransaction(const CTransaction& tx)
    {
        if (!g_honeypotFilter) {
            return false;
        }
        
        HoneypotFilterResult result = g_honeypotFilter->CheckTransaction(tx);
        return result.isSuspicious;
    }
    
    std::string GetFilterTypeName(HoneypotFilterType type)
    {
        switch (type) {
            case HoneypotFilterType::EXCESSIVE_OP_RETURN: return "EXCESSIVE_OP_RETURN";
            case HoneypotFilterType::INVALID_SCRIPT: return "INVALID_SCRIPT";
            case HoneypotFilterType::SUSPICIOUS_DUST: return "SUSPICIOUS_DUST";
            case HoneypotFilterType::MALFORMED_INPUT: return "MALFORMED_INPUT";
            case HoneypotFilterType::SPAM_PATTERN: return "SPAM_PATTERN";
            case HoneypotFilterType::EXPLOIT_ATTEMPT: return "EXPLOIT_ATTEMPT";
            case HoneypotFilterType::EXCESSIVE_SIZE: return "EXCESSIVE_SIZE";
            case HoneypotFilterType::RATE_LIMIT_VIOLATION: return "RATE_LIMIT_VIOLATION";
            default: return "UNKNOWN";
        }
    }
    
    std::string FormatFilterResult(const HoneypotFilterResult& result)
    {
        std::stringstream ss;
        ss << "Type: " << GetFilterTypeName(result.filterType) << ", ";
        ss << "Reason: " << result.reason;
        
        for (const auto& detail : result.details) {
            ss << ", Detail: " << detail;
        }
        
        return ss.str();
    }
    
    bool IsHoneypotFilteringEnabled()
    {
        return g_honeypotFilter != nullptr;
    }
    
    void SetHoneypotFilteringEnabled(bool enabled)
    {
        if (enabled && !g_honeypotFilter) {
            InitializeHoneypotFiltering();
        } else if (!enabled && g_honeypotFilter) {
            ShutdownHoneypotFiltering();
        }
    }
}
