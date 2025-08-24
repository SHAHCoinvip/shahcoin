// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_POLICY_HONEYPOT_FILTER_H
#define SHAHCOIN_POLICY_HONEYPOT_FILTER_H

#include <primitives/transaction.h>
#include <script/script.h>
#include <script/standard.h>
#include <logging.h>
#include <util/time.h>

#include <vector>
#include <map>
#include <set>
#include <string>

class CTransaction;
class CScript;

/**
 * Honeypot Filter Types
 * Different categories of suspicious transaction patterns
 */
enum class HoneypotFilterType {
    EXCESSIVE_OP_RETURN = 1,    // Too many OP_RETURN outputs
    INVALID_SCRIPT = 2,          // Malformed scriptSig/scriptPubKey
    SUSPICIOUS_DUST = 3,         // Dust amounts in suspicious patterns
    MALFORMED_INPUT = 4,         // Invalid input structure
    SPAM_PATTERN = 5,            // Known spam transaction patterns
    EXPLOIT_ATTEMPT = 6,         // Potential exploit attempts
    EXCESSIVE_SIZE = 7,          // Transaction too large
    RATE_LIMIT_VIOLATION = 8,    // Too many transactions from same source
};

/**
 * Honeypot Filter Result
 * Result of honeypot filter analysis
 */
struct HoneypotFilterResult {
    bool isSuspicious;                           // Whether transaction is suspicious
    HoneypotFilterType filterType;              // Type of filter that triggered
    std::string reason;                         // Human-readable reason
    int64_t timestamp;                          // When the check was performed
    std::vector<std::string> details;           // Additional details
    
    HoneypotFilterResult() : isSuspicious(false), filterType(HoneypotFilterType::EXCESSIVE_OP_RETURN), timestamp(0) {}
};

/**
 * Honeypot Filter Configuration
 * Configurable parameters for honeypot detection
 */
struct HoneypotFilterConfig {
    // OP_RETURN limits
    size_t maxOpReturnOutputs;                  // Maximum OP_RETURN outputs per transaction
    size_t maxOpReturnSize;                     // Maximum OP_RETURN data size per output
    size_t maxTotalOpReturnSize;                // Maximum total OP_RETURN data size
    
    // Dust limits
    CAmount minDustThreshold;                   // Minimum dust amount
    size_t maxDustOutputs;                      // Maximum dust outputs per transaction
    
    // Size limits
    size_t maxTransactionSize;                  // Maximum transaction size in bytes
    size_t maxInputCount;                       // Maximum number of inputs
    size_t maxOutputCount;                      // Maximum number of outputs
    
    // Rate limiting
    size_t maxTransactionsPerBlock;             // Maximum transactions per block from same source
    int64_t rateLimitWindow;                    // Rate limit window in seconds
    
    // Script validation
    bool strictScriptValidation;                // Enable strict script validation
    size_t maxScriptSize;                       // Maximum script size
    
    HoneypotFilterConfig() : maxOpReturnOutputs(10), maxOpReturnSize(80), maxTotalOpReturnSize(400),
                            minDustThreshold(546), maxDustOutputs(100), maxTransactionSize(1000000),
                            maxInputCount(1000), maxOutputCount(1000), maxTransactionsPerBlock(100),
                            rateLimitWindow(3600), strictScriptValidation(true), maxScriptSize(10000) {}
};

/**
 * Honeypot Filter Manager
 * Manages honeypot detection and filtering
 */
class CHoneypotFilterManager
{
public:
    CHoneypotFilterManager();
    ~CHoneypotFilterManager();
    
    // Configuration
    void SetConfig(const HoneypotFilterConfig& config);
    HoneypotFilterConfig GetConfig() const { return m_config; }
    
    // Main filtering function
    HoneypotFilterResult CheckTransaction(const CTransaction& tx);
    
    // Individual filter checks
    HoneypotFilterResult CheckOpReturnOutputs(const CTransaction& tx);
    HoneypotFilterResult CheckScriptValidation(const CTransaction& tx);
    HoneypotFilterResult CheckDustOutputs(const CTransaction& tx);
    HoneypotFilterResult CheckInputValidation(const CTransaction& tx);
    HoneypotFilterResult CheckSpamPatterns(const CTransaction& tx);
    HoneypotFilterResult CheckExploitAttempts(const CTransaction& tx);
    HoneypotFilterResult CheckTransactionSize(const CTransaction& tx);
    HoneypotFilterResult CheckRateLimiting(const CTransaction& tx);
    
    // Statistics and monitoring
    struct FilterStats {
        uint64_t totalTransactions;
        uint64_t suspiciousTransactions;
        uint64_t blockedTransactions;
        std::map<HoneypotFilterType, uint64_t> filterTypeCounts;
        int64_t lastResetTime;
    };
    
    FilterStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogSuspiciousTransaction(const CTransaction& tx, const HoneypotFilterResult& result);
    void LogFilterStats();

private:
    HoneypotFilterConfig m_config;
    FilterStats m_stats;
    
    // Rate limiting tracking
    std::map<std::string, std::vector<int64_t>> m_rateLimitTracker; // address -> timestamps
    
    // Known spam patterns
    std::set<std::string> m_knownSpamPatterns;
    std::set<std::string> m_knownExploitPatterns;
    
    // Helper functions
    bool IsOpReturnOutput(const CScript& script) const;
    bool IsDustAmount(CAmount amount) const;
    bool IsSuspiciousScript(const CScript& script) const;
    bool IsKnownSpamPattern(const CTransaction& tx) const;
    bool IsKnownExploitPattern(const CTransaction& tx) const;
    std::string GetTransactionSource(const CTransaction& tx) const;
    void UpdateRateLimitTracker(const std::string& source, int64_t timestamp);
    bool IsRateLimitExceeded(const std::string& source) const;
    void CleanupRateLimitTracker(int64_t currentTime);
    
    // Pattern matching
    bool MatchesPattern(const CScript& script, const std::set<std::string>& patterns) const;
    std::string ExtractScriptPattern(const CScript& script) const;
};

// Global honeypot filter manager instance
extern std::unique_ptr<CHoneypotFilterManager> g_honeypotFilter;

// Utility functions
namespace HoneypotUtils {
    // Initialize honeypot filtering system
    bool InitializeHoneypotFiltering();
    
    // Shutdown honeypot filtering system
    void ShutdownHoneypotFiltering();
    
    // Check if transaction should be rejected
    bool ShouldRejectTransaction(const CTransaction& tx);
    
    // Get human-readable filter type name
    std::string GetFilterTypeName(HoneypotFilterType type);
    
    // Format filter result for logging
    std::string FormatFilterResult(const HoneypotFilterResult& result);
    
    // Check if honeypot filtering is enabled
    bool IsHoneypotFilteringEnabled();
    
    // Enable/disable honeypot filtering
    void SetHoneypotFilteringEnabled(bool enabled);
}

#endif // SHAHCOIN_POLICY_HONEYPOT_FILTER_H
