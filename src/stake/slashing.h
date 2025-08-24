// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_STAKE_SLASHING_H
#define SHAHCOIN_STAKE_SLASHING_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <stake/stake.h>
#include <consensus/consensus.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>

#include <vector>
#include <map>
#include <set>
#include <memory>

class CBlockIndex;
class CCoinsViewCache;

/**
 * Slashing Evidence
 * Represents evidence of validator misbehavior
 */
struct CSlashingEvidence
{
    enum EvidenceType {
        DOUBLE_SIGNING = 1,        // Validator signed two different blocks at same height
        INVALID_BLOCK = 2,         // Validator created invalid block
        MALICIOUS_BEHAVIOR = 3,    // Other malicious behavior
        INACTIVITY = 4,            // Validator inactive for too long
    };
    
    EvidenceType type;
    CTxDestination validatorAddress;
    uint256 blockHash1;            // First block hash (for double signing)
    uint256 blockHash2;            // Second block hash (for double signing)
    uint256 invalidBlockHash;      // Invalid block hash
    int64_t timestamp;             // When the evidence was created
    std::vector<unsigned char> proof; // Cryptographic proof of misbehavior
    
    CSlashingEvidence() : type(DOUBLE_SIGNING), timestamp(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
};

/**
 * Slashing Penalty
 * Defines the penalty for different types of misbehavior
 */
struct CSlashingPenalty
{
    enum PenaltyType {
        STAKE_SLASH = 1,           // Slash a percentage of stake
        TEMPORARY_BAN = 2,         // Ban from staking temporarily
        PERMANENT_BAN = 3,         // Ban from staking permanently
        REWARD_REDUCTION = 4,      // Reduce future rewards
    };
    
    PenaltyType type;
    double stakeSlashPercentage;   // Percentage of stake to slash (0.0-1.0)
    int64_t banDuration;           // Duration of ban in seconds (0 for permanent)
    double rewardReductionFactor;  // Factor to reduce rewards (0.0-1.0)
    
    CSlashingPenalty() : type(STAKE_SLASH), stakeSlashPercentage(0.0), 
                        banDuration(0), rewardReductionFactor(1.0) {}
};

/**
 * Slashing Manager
 * Manages slashing detection, evidence collection, and penalty enforcement
 */
class CSlashingManager
{
public:
    CSlashingManager();
    ~CSlashingManager();
    
    // Evidence management
    bool AddEvidence(const CSlashingEvidence& evidence);
    bool ValidateEvidence(const CSlashingEvidence& evidence);
    std::vector<CSlashingEvidence> GetEvidenceForValidator(const CTxDestination& address) const;
    
    // Penalty enforcement
    bool EnforcePenalty(const CTxDestination& address, const CSlashingPenalty& penalty);
    bool IsValidatorBanned(const CTxDestination& address) const;
    int64_t GetBanEndTime(const CTxDestination& address) const;
    
    // Detection methods
    bool DetectDoubleSigning(const CBlock& block1, const CBlock& block2);
    bool DetectInvalidBlock(const CBlock& block, const CBlockIndex* pindexPrev);
    bool DetectInactivity(const CTxDestination& address, int64_t currentTime);
    
    // Queries
    std::vector<CTxDestination> GetBannedValidators() const;
    std::vector<CSlashingEvidence> GetAllEvidence() const;
    CAmount GetTotalSlashedAmount() const;
    
    // Configuration
    void SetSlashingEnabled(bool enabled) { m_slashingEnabled = enabled; }
    bool IsSlashingEnabled() const { return m_slashingEnabled; }
    
    // Penalty configuration
    void SetDoubleSigningPenalty(const CSlashingPenalty& penalty);
    void SetInvalidBlockPenalty(const CSlashingPenalty& penalty);
    void SetInactivityPenalty(const CSlashingPenalty& penalty);
    
    CSlashingPenalty GetDoubleSigningPenalty() const { return m_doubleSigningPenalty; }
    CSlashingPenalty GetInvalidBlockPenalty() const { return m_invalidBlockPenalty; }
    CSlashingPenalty GetInactivityPenalty() const { return m_inactivityPenalty; }

private:
    // Evidence storage
    std::vector<CSlashingEvidence> m_evidence;
    std::map<CTxDestination, std::vector<CSlashingEvidence>> m_validatorEvidence;
    
    // Banned validators
    std::map<CTxDestination, int64_t> m_bannedValidators; // address -> ban end time
    
    // Slashed amounts
    std::map<CTxDestination, CAmount> m_slashedAmounts;
    CAmount m_totalSlashedAmount;
    
    // Configuration
    bool m_slashingEnabled;
    CSlashingPenalty m_doubleSigningPenalty;
    CSlashingPenalty m_invalidBlockPenalty;
    CSlashingPenalty m_inactivityPenalty;
    
    // Inactivity tracking
    std::map<CTxDestination, int64_t> m_lastActivityTime;
    int64_t m_inactivityThreshold; // Time threshold for inactivity detection
    
    // Helper methods
    bool ValidateDoubleSigningEvidence(const CSlashingEvidence& evidence);
    bool ValidateInvalidBlockEvidence(const CSlashingEvidence& evidence);
    bool ValidateInactivityEvidence(const CSlashingEvidence& evidence);
    
    void ApplyStakeSlash(const CTxDestination& address, double percentage);
    void ApplyTemporaryBan(const CTxDestination& address, int64_t duration);
    void ApplyPermanentBan(const CTxDestination& address);
    void ApplyRewardReduction(const CTxDestination& address, double factor);
    
    void CleanupExpiredBans(int64_t currentTime);
};

/**
 * NFT Staking Boost
 * Provides staking bonuses for NFT holders
 */
struct CNFTStakingBoost
{
    uint256 nftId;                 // NFT identifier
    CTxDestination nftOwner;       // NFT owner address
    double boostMultiplier;        // Staking boost multiplier (1.0 = no boost)
    int64_t boostStartTime;        // When boost started
    int64_t boostEndTime;          // When boost ends (0 for permanent)
    std::string boostType;         // Type of boost (e.g., "legendary", "rare")
    
    CNFTStakingBoost() : boostMultiplier(1.0), boostStartTime(0), boostEndTime(0) {}
    
    bool IsActive(int64_t currentTime) const;
    bool IsPermanent() const { return boostEndTime == 0; }
    double GetEffectiveMultiplier(int64_t currentTime) const;
};

/**
 * NFT Staking Boost Manager
 * Manages NFT-based staking boosts
 */
class CNFTStakingBoostManager
{
public:
    CNFTStakingBoostManager();
    ~CNFTStakingBoostManager();
    
    // Boost management
    bool AddBoost(const CNFTStakingBoost& boost);
    bool RemoveBoost(const uint256& nftId);
    bool UpdateBoost(const CNFTStakingBoost& boost);
    
    // Boost queries
    CNFTStakingBoost GetBoost(const uint256& nftId) const;
    std::vector<CNFTStakingBoost> GetBoostsForAddress(const CTxDestination& address) const;
    std::vector<CNFTStakingBoost> GetActiveBoosts(int64_t currentTime) const;
    
    // Boost calculation
    double CalculateTotalBoost(const CTxDestination& address, int64_t currentTime) const;
    double CalculateEffectiveStake(const CTxDestination& address, CAmount actualStake, int64_t currentTime) const;
    
    // Configuration
    void SetMaxBoostMultiplier(double maxMultiplier) { m_maxBoostMultiplier = maxMultiplier; }
    void SetBoostStackingEnabled(bool enabled) { m_boostStackingEnabled = enabled; }
    
    double GetMaxBoostMultiplier() const { return m_maxBoostMultiplier; }
    bool IsBoostStackingEnabled() const { return m_boostStackingEnabled; }

private:
    std::map<uint256, CNFTStakingBoost> m_boosts;
    std::map<CTxDestination, std::vector<uint256>> m_addressBoosts;
    
    double m_maxBoostMultiplier;
    bool m_boostStackingEnabled;
    
    void CleanupExpiredBoosts(int64_t currentTime);
};

/**
 * Governance Proposal
 * Represents a governance proposal for staking-related decisions
 */
struct CGovernanceProposal
{
    enum ProposalType {
        SLASHING_PARAMETER_CHANGE = 1,
        STAKING_REWARD_CHANGE = 2,
        MINIMUM_STAKE_CHANGE = 3,
        VALIDATOR_LIMIT_CHANGE = 4,
        NFT_BOOST_CHANGE = 5,
        GENERAL_PROPOSAL = 6,
    };
    
    uint256 proposalId;
    ProposalType type;
    std::string title;
    std::string description;
    CTxDestination proposer;
    int64_t creationTime;
    int64_t votingStartTime;
    int64_t votingEndTime;
    int64_t executionTime;
    bool executed;
    
    // Proposal parameters
    std::map<std::string, std::string> parameters;
    
    CGovernanceProposal() : type(GENERAL_PROPOSAL), creationTime(0), votingStartTime(0), 
                           votingEndTime(0), executionTime(0), executed(false) {}
    
    uint256 GetHash() const;
    bool IsActive(int64_t currentTime) const;
    bool CanVote(int64_t currentTime) const;
    bool CanExecute(int64_t currentTime) const;
};

/**
 * Governance Vote
 * Represents a vote on a governance proposal
 */
struct CGovernanceVote
{
    uint256 proposalId;
    CTxDestination voter;
    bool vote; // true = yes, false = no
    CAmount stakeWeight; // Weight of voter's stake
    int64_t votingTime;
    
    CGovernanceVote() : vote(false), stakeWeight(0), votingTime(0) {}
    
    uint256 GetHash() const;
};

/**
 * Governance Manager
 * Manages governance proposals and voting
 */
class CGovernanceManager
{
public:
    CGovernanceManager();
    ~CGovernanceManager();
    
    // Proposal management
    bool CreateProposal(const CGovernanceProposal& proposal);
    bool ExecuteProposal(const uint256& proposalId);
    bool CancelProposal(const uint256& proposalId);
    
    // Voting
    bool Vote(const CGovernanceVote& vote);
    bool HasVoted(const uint256& proposalId, const CTxDestination& voter) const;
    
    // Queries
    CGovernanceProposal GetProposal(const uint256& proposalId) const;
    std::vector<CGovernanceProposal> GetActiveProposals(int64_t currentTime) const;
    std::vector<CGovernanceProposal> GetProposalsByType(CGovernanceProposal::ProposalType type) const;
    
    // Vote counting
    struct VoteResult {
        CAmount yesVotes;
        CAmount noVotes;
        CAmount totalVotes;
        double yesPercentage;
        bool passed;
    };
    
    VoteResult GetVoteResult(const uint256& proposalId) const;
    std::vector<CGovernanceVote> GetVotesForProposal(const uint256& proposalId) const;
    
    // Configuration
    void SetMinimumProposalStake(CAmount minStake) { m_minimumProposalStake = minStake; }
    void SetVotingPeriod(int64_t period) { m_votingPeriod = period; }
    void SetExecutionDelay(int64_t delay) { m_executionDelay = delay; }
    void SetQuorumThreshold(double threshold) { m_quorumThreshold = threshold; }
    void SetPassThreshold(double threshold) { m_passThreshold = threshold; }
    
    CAmount GetMinimumProposalStake() const { return m_minimumProposalStake; }
    int64_t GetVotingPeriod() const { return m_votingPeriod; }
    int64_t GetExecutionDelay() const { return m_executionDelay; }
    double GetQuorumThreshold() const { return m_quorumThreshold; }
    double GetPassThreshold() const { return m_passThreshold; }

private:
    std::map<uint256, CGovernanceProposal> m_proposals;
    std::map<uint256, std::vector<CGovernanceVote>> m_votes;
    std::map<uint256, std::set<CTxDestination>> m_voters;
    
    CAmount m_minimumProposalStake;
    int64_t m_votingPeriod;
    int64_t m_executionDelay;
    double m_quorumThreshold;
    double m_passThreshold;
    
    uint256 GenerateProposalId(const CGovernanceProposal& proposal) const;
    bool ValidateProposal(const CGovernanceProposal& proposal) const;
    bool ValidateVote(const CGovernanceVote& vote) const;
    void CleanupExpiredProposals(int64_t currentTime);
};

// Global instances
extern std::unique_ptr<CSlashingManager> g_slashingManager;
extern std::unique_ptr<CNFTStakingBoostManager> g_nftBoostManager;
extern std::unique_ptr<CGovernanceManager> g_governanceManager;

// Utility functions
namespace SlashingUtils {
    bool IsDoubleSigning(const CBlock& block1, const CBlock& block2);
    bool IsInvalidBlock(const CBlock& block, const CBlockIndex* pindexPrev);
    bool IsValidatorInactive(const CTxDestination& address, int64_t currentTime);
    
    CSlashingEvidence CreateDoubleSigningEvidence(const CBlock& block1, const CBlock& block2);
    CSlashingEvidence CreateInvalidBlockEvidence(const CBlock& block);
    CSlashingEvidence CreateInactivityEvidence(const CTxDestination& address);
}

namespace NFTBoostUtils {
    double CalculateStakingBoost(const CTxDestination& address, CAmount stake, int64_t currentTime);
    std::vector<CNFTStakingBoost> GetActiveBoostsForAddress(const CTxDestination& address, int64_t currentTime);
    bool IsEligibleForBoost(const CTxDestination& address, const uint256& nftId);
}

namespace GovernanceUtils {
    bool CanCreateProposal(const CTxDestination& address, CAmount stake);
    bool CanVoteOnProposal(const CTxDestination& address, const uint256& proposalId);
    CAmount CalculateVoteWeight(const CTxDestination& address);
    bool ExecuteProposalIfPassed(const uint256& proposalId);
}

#endif // SHAHCOIN_STAKE_SLASHING_H
