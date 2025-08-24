// Copyright (C) 2025 The SHAHCOIN Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_SECURITY_SECURITY_H
#define SHAHCOIN_SECURITY_SECURITY_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <net.h>
#include <uint256.h>
#include <consensus/consensus.h>

/** SHAHCOIN Core security validation and protection measures */
class CSecurityValidator
{
public:
    // Anti-Sybil protection
    static bool CheckConnectionLimits(const CAddress& addr);
    static bool CheckMinimumChainWork(const uint256& chainWork);
    
    // Anti-spam measures
    static bool CheckTransactionSpam(const CTransaction& tx);
    static bool CheckBlockSpam(const CBlock& block);
    
    // Replay protection
    static bool CheckReplayProtection(const CTransaction& tx);
    
    // Honeytrap detection
    static bool CheckHoneytrapSignatures(const CTransaction& tx);
    
    // Reorg protection
    static bool CheckReorgDepth(int depth);
    
    // Mining whitelist (for early blocks)
    static bool CheckMiningWhitelist(const CTransaction& coinbase, int height);
    
    // Rate limiting
    static bool CheckRateLimit(const CAddress& addr, const std::string& operation);
    
    // Misbehaving node detection
    static void Misbehaving(NodeId nodeid, int howmuch, const std::string& reason);
    
    // Check for suspicious patterns
    static bool CheckSuspiciousPatterns(const CBlock& block);
    
    // Validate OP_RETURN data
    static bool ValidateOpReturnData(const CScript& scriptPubKey);
    
    // Check transaction size limits
    static bool CheckTransactionSize(const CTransaction& tx);
    
    // Check block size limits
    static bool CheckBlockSize(const CBlock& block);
    
    // Validate fee requirements
    static bool CheckMinimumFee(const CTransaction& tx, const CFeeRate& minFee);
    
private:
    static std::set<CAddress> bannedAddresses;
    static std::map<CAddress, int> connectionAttempts;
    static std::map<CAddress, std::chrono::steady_clock::time_point> lastConnectionAttempt;
    
    // Honeytrap signatures to detect
    static const std::vector<std::vector<unsigned char>> honeytrapSignatures;
    
    // Mining whitelist addresses (for early blocks)
    static const std::vector<std::string> miningWhitelist;
};

#endif // SHAHCOIN_SECURITY_SECURITY_H
