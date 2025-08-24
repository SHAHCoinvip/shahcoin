// Copyright (c) 2024 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHAHCOIN_DEX_DEX_H
#define SHAHCOIN_DEX_DEX_H

#include <primitives/transaction.h>
#include <script/standard.h>
#include <key.h>
#include <logging.h>
#include <util/time.h>
#include <consensus/amount.h>
#include <tokens/token.h>

#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>

class CWallet;
class CBlockIndex;

/**
 * DEX Fee Configuration
 * ShahSwap fee structure
 */
struct DEXFeeConfig {
    double swapFeePercent;          // Swap fee percentage (e.g., 0.3%)
    double liquidityFeePercent;     // Liquidity fee percentage (e.g., 0.25%)
    double protocolFeePercent;      // Protocol fee percentage (e.g., 0.05%)
    CAmount minSwapAmount;          // Minimum swap amount in SHAH
    CAmount maxSwapAmount;          // Maximum swap amount in SHAH
    bool dynamicFees;               // Enable dynamic fee adjustment
    int64_t feeUpdateInterval;      // Fee update interval in seconds
    
    DEXFeeConfig() : swapFeePercent(0.003), liquidityFeePercent(0.0025), protocolFeePercent(0.0005),
                     minSwapAmount(0.001 * COIN), maxSwapAmount(1000000 * COIN), 
                     dynamicFees(true), feeUpdateInterval(3600) {}
};

/**
 * Liquidity Pool Information
 * Represents a liquidity pool for token trading
 */
struct CLiquidityPool {
    uint256 poolHash;               // Unique pool hash
    uint256 tokenHash;              // Token being traded
    CAmount tokenReserve;           // Token reserve amount
    CAmount shahReserve;            // SHAH reserve amount
    CAmount totalLiquidity;         // Total liquidity tokens
    CTxDestination creator;         // Pool creator
    uint256 creationTxHash;         // Transaction that created the pool
    int64_t creationTime;           // When the pool was created
    bool isActive;                  // Whether pool is active
    double currentPrice;            // Current token price in SHAH
    double priceImpact;             // Price impact for large trades
    uint64_t totalSwaps;            // Total number of swaps
    CAmount totalVolume;            // Total trading volume
    
    CLiquidityPool() : tokenReserve(0), shahReserve(0), totalLiquidity(0), creationTime(0),
                       isActive(true), currentPrice(0.0), priceImpact(0.0), totalSwaps(0), totalVolume(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    std::string GetDisplayName() const;
    double CalculatePrice() const;
    double CalculatePriceImpact(CAmount swapAmount, bool isTokenToShah) const;
    CAmount CalculateSwapOutput(CAmount inputAmount, bool isTokenToShah) const;
    CAmount CalculateLiquidityTokens(CAmount tokenAmount, CAmount shahAmount) const;
};

/**
 * Liquidity Position Information
 * Represents a user's liquidity position
 */
struct CLiquidityPosition {
    uint256 positionHash;           // Unique position hash
    uint256 poolHash;               // Associated pool
    CTxDestination owner;           // Position owner
    CAmount liquidityTokens;        // Amount of liquidity tokens
    CAmount tokenAmount;            // Token amount in position
    CAmount shahAmount;             // SHAH amount in position
    int64_t creationTime;           // When position was created
    int64_t lastUpdateTime;         // Last update time
    bool isActive;                  // Whether position is active
    CAmount totalFeesEarned;        // Total fees earned
    
    CLiquidityPosition() : liquidityTokens(0), tokenAmount(0), shahAmount(0), 
                          creationTime(0), lastUpdateTime(0), isActive(true), totalFeesEarned(0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    double GetSharePercentage() const;
    CAmount CalculateFeesEarned() const;
};

/**
 * Swap Order Information
 * Represents a swap order
 */
struct CSwapOrder {
    uint256 orderHash;              // Unique order hash
    uint256 poolHash;               // Associated pool
    CTxDestination trader;          // Order trader
    bool isTokenToShah;             // Swap direction (true: token->SHAH, false: SHAH->token)
    CAmount inputAmount;            // Input amount
    CAmount outputAmount;           // Expected output amount
    double slippageTolerance;       // Slippage tolerance percentage
    int64_t creationTime;           // When order was created
    int64_t expiryTime;             // Order expiry time
    bool isExecuted;                // Whether order was executed
    uint256 executionTxHash;        // Execution transaction hash
    CAmount actualOutputAmount;     // Actual output amount received
    double executionPrice;          // Execution price
    
    CSwapOrder() : inputAmount(0), outputAmount(0), slippageTolerance(0.01), creationTime(0),
                   expiryTime(0), isExecuted(false), actualOutputAmount(0), executionPrice(0.0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    bool IsExpired() const;
    bool IsExecutable() const;
    double CalculateSlippage() const;
};

/**
 * DEX Transaction Types
 * Different types of DEX operations
 */
enum class DEXTxType {
    CREATE_POOL = 1,        // Create liquidity pool
    ADD_LIQUIDITY = 2,      // Add liquidity to pool
    REMOVE_LIQUIDITY = 3,   // Remove liquidity from pool
    SWAP_TOKENS = 4,        // Swap tokens
    SWAP_SHAH = 5,          // Swap SHAH for tokens
    UPDATE_FEES = 6,        // Update pool fees
    CLAIM_FEES = 7,         // Claim earned fees
};

/**
 * DEX Transaction
 * Special transaction type for DEX operations
 */
struct CDEXTx {
    DEXTxType type;
    uint256 poolHash;               // Associated pool
    CTxDestination trader;          // Transaction trader
    CAmount amount1;                // First amount (token or SHAH)
    CAmount amount2;                // Second amount (SHAH or token)
    uint256 tokenHash;              // Token being traded
    double slippageTolerance;       // Slippage tolerance
    int64_t timestamp;
    std::vector<unsigned char> signature;
    
    // Pool creation specific fields
    std::string poolName;
    std::string poolDescription;
    double initialPrice;
    
    // Swap specific fields
    bool isTokenToShah;
    CAmount expectedOutput;
    double executionPrice;
    
    CDEXTx() : type(DEXTxType::CREATE_POOL), amount1(0), amount2(0), slippageTolerance(0.01),
               timestamp(0), initialPrice(0.0), isTokenToShah(true), expectedOutput(0), executionPrice(0.0) {}
    
    uint256 GetHash() const;
    bool IsValid() const;
    bool IsPoolCreationTx() const;
    bool IsSwapTx() const;
    bool IsLiquidityTx() const;
};

/**
 * ShahSwap DEX Manager
 * Manages the decentralized exchange operations
 */
class CDEXManager
{
public:
    CDEXManager();
    ~CDEXManager();
    
    // Configuration
    void SetFeeConfig(const DEXFeeConfig& config);
    DEXFeeConfig GetFeeConfig() const { return m_feeConfig; }
    
    // Pool management
    bool CreatePool(const CTxDestination& creator, const uint256& tokenHash,
                   CAmount initialTokenAmount, CAmount initialShahAmount);
    bool ValidatePoolCreation(const CDEXTx& dexTx);
    
    // Liquidity management
    bool AddLiquidity(const uint256& poolHash, const CTxDestination& provider,
                     CAmount tokenAmount, CAmount shahAmount);
    bool RemoveLiquidity(const uint256& poolHash, const CTxDestination& provider,
                        CAmount liquidityTokens);
    bool ValidateLiquidityOperation(const CDEXTx& dexTx);
    
    // Swap operations
    bool SwapTokensForShah(const uint256& poolHash, const CTxDestination& trader,
                          CAmount tokenAmount, double slippageTolerance);
    bool SwapShahForTokens(const uint256& poolHash, const CTxDestination& trader,
                          CAmount shahAmount, double slippageTolerance);
    bool ValidateSwapOperation(const CDEXTx& dexTx);
    
    // Pool queries
    CLiquidityPool GetPool(const uint256& poolHash) const;
    std::vector<CLiquidityPool> GetPoolsByToken(const uint256& tokenHash) const;
    std::vector<CLiquidityPool> GetPoolsByCreator(const CTxDestination& creator) const;
    std::vector<CLiquidityPool> GetAllPools() const;
    bool IsValidPool(const uint256& poolHash) const;
    
    // Position queries
    CLiquidityPosition GetPosition(const uint256& positionHash) const;
    std::vector<CLiquidityPosition> GetPositionsByOwner(const CTxDestination& owner) const;
    std::vector<CLiquidityPosition> GetPositionsByPool(const uint256& poolHash) const;
    bool IsPositionOwner(const uint256& positionHash, const CTxDestination& owner) const;
    
    // Swap queries
    CSwapOrder GetSwapOrder(const uint256& orderHash) const;
    std::vector<CSwapOrder> GetOrdersByTrader(const CTxDestination& trader) const;
    std::vector<CSwapOrder> GetPendingOrders(const uint256& poolHash) const;
    
    // Price and calculation functions
    double GetTokenPrice(const uint256& poolHash) const;
    double CalculatePriceImpact(const uint256& poolHash, CAmount swapAmount, bool isTokenToShah) const;
    CAmount CalculateSwapOutput(const uint256& poolHash, CAmount inputAmount, bool isTokenToShah) const;
    CAmount CalculateLiquidityTokens(const uint256& poolHash, CAmount tokenAmount, CAmount shahAmount) const;
    CAmount CalculateFees(const uint256& poolHash, CAmount swapAmount) const;
    
    // Transaction creation
    CTransactionRef CreatePoolCreationTx(const CTxDestination& creator, const uint256& tokenHash,
                                       CAmount initialTokenAmount, CAmount initialShahAmount);
    CTransactionRef CreateSwapTx(const uint256& poolHash, const CTxDestination& trader,
                               CAmount inputAmount, bool isTokenToShah, double slippageTolerance);
    CTransactionRef CreateLiquidityTx(const uint256& poolHash, const CTxDestination& provider,
                                    CAmount tokenAmount, CAmount shahAmount, bool isAdd);
    
    // Validation
    bool ValidateDEXTransaction(const CTransaction& tx) const;
    bool IsDEXTransaction(const CTransaction& tx) const;
    bool IsValidPool(const uint256& poolHash) const;
    bool IsValidPosition(const uint256& positionHash) const;
    
    // Statistics and monitoring
    struct DEXStats {
        uint64_t totalPools;
        uint64_t activePools;
        uint64_t totalPositions;
        uint64_t activePositions;
        uint64_t totalSwaps;
        CAmount totalVolume;
        CAmount totalFees;
        int64_t lastUpdateTime;
    };
    
    DEXStats GetStats() const;
    void ResetStats();
    
    // Logging
    void LogPoolCreation(const CLiquidityPool& pool);
    void LogSwapExecution(const CSwapOrder& order);
    void LogLiquidityOperation(const CLiquidityPosition& position);
    void LogDEXTransaction(const CDEXTx& dexTx);
    void LogDEXStats();

private:
    // DEX storage
    std::map<uint256, CLiquidityPool> m_pools;
    std::map<uint256, CLiquidityPosition> m_positions;
    std::map<uint256, CSwapOrder> m_swapOrders;
    std::map<CTxDestination, std::vector<uint256>> m_creatorPools;
    std::map<CTxDestination, std::vector<uint256>> m_ownerPositions;
    std::map<CTxDestination, std::vector<uint256>> m_traderOrders;
    std::map<uint256, std::vector<uint256>> m_poolPositions;
    std::map<uint256, std::vector<uint256>> m_poolOrders;
    
    // Configuration and statistics
    DEXFeeConfig m_feeConfig;
    DEXStats m_stats;
    
    // Helper functions
    void UpdatePoolIndexes(const CLiquidityPool& pool, bool add);
    void RemovePoolIndexes(const CLiquidityPool& pool);
    void UpdatePositionIndexes(const CLiquidityPosition& position, bool add);
    void RemovePositionIndexes(const CLiquidityPosition& position);
    void UpdateOrderIndexes(const CSwapOrder& order, bool add);
    void RemoveOrderIndexes(const CSwapOrder& order);
    bool ValidatePoolParameters(const uint256& tokenHash, CAmount tokenAmount, CAmount shahAmount) const;
    bool ValidateSwapParameters(const uint256& poolHash, CAmount amount, double slippage) const;
    bool ValidateLiquidityParameters(const uint256& poolHash, CAmount tokenAmount, CAmount shahAmount) const;
    void UpdateStats();
    void CleanupExpiredOrders();
    
    // Pool operations
    bool UpdatePoolReserves(const uint256& poolHash, CAmount tokenDelta, CAmount shahDelta);
    bool UpdatePoolFees(const uint256& poolHash, CAmount feeAmount);
    bool UpdatePoolPrice(const uint256& poolHash);
    
    // Position operations
    bool UpdatePosition(const uint256& positionHash, CAmount liquidityDelta, CAmount tokenDelta, CAmount shahDelta);
    bool UpdatePositionFees(const uint256& positionHash, CAmount feeAmount);
    
    // Order operations
    bool ExecuteSwapOrder(const uint256& orderHash, CAmount actualOutput, double executionPrice);
    bool CancelSwapOrder(const uint256& orderHash);
    
    // Transaction helpers
    bool AddDEXTransaction(CTransaction& tx, const CDEXTx& dexTx);
    bool ParseDEXTransaction(const CTransaction& tx, CDEXTx& dexTx) const;
    
    // Hash generation
    uint256 GeneratePoolHash(const CTxDestination& creator, const uint256& tokenHash, int64_t timestamp);
    uint256 GeneratePositionHash(const uint256& poolHash, const CTxDestination& owner, int64_t timestamp);
    uint256 GenerateOrderHash(const uint256& poolHash, const CTxDestination& trader, int64_t timestamp);
};

/**
 * DEX Transaction Builder
 * Handles creation of DEX transactions
 */
class CDEXTxBuilder
{
public:
    CDEXTxBuilder();
    
    // Transaction building
    CTransactionRef BuildPoolCreationTx(const CTxDestination& creator, const uint256& tokenHash,
                                      CAmount initialTokenAmount, CAmount initialShahAmount);
    CTransactionRef BuildSwapTx(const uint256& poolHash, const CTxDestination& trader,
                              CAmount inputAmount, bool isTokenToShah, double slippageTolerance);
    CTransactionRef BuildLiquidityTx(const uint256& poolHash, const CTxDestination& provider,
                                   CAmount tokenAmount, CAmount shahAmount, bool isAdd);
    
    // Script creation
    CScript CreateDEXScript(const CDEXTx& dexTx);
    CScript CreatePoolCreationScript(const CDEXTx& dexTx);
    CScript CreateSwapScript(const CDEXTx& dexTx);
    CScript CreateLiquidityScript(const CDEXTx& dexTx);
    
    // Validation
    bool ValidateDEXScript(const CScript& script);
    bool ValidatePoolCreationScript(const CScript& script);
    bool ValidateSwapScript(const CScript& script);
    bool ValidateLiquidityScript(const CScript& script);

private:
    // Script parsing
    bool ParseDEXScript(const CScript& script, CDEXTx& dexTx);
    bool ParsePoolCreationScript(const CScript& script, CDEXTx& dexTx);
    bool ParseSwapScript(const CScript& script, CDEXTx& dexTx);
    bool ParseLiquidityScript(const CScript& script, CDEXTx& dexTx);
    
    // Transaction helpers
    void AddDEXOutput(CTransaction& tx, const CDEXTx& dexTx);
    void AddPoolCreationOutput(CTransaction& tx, const CDEXTx& dexTx);
    void AddSwapOutput(CTransaction& tx, const CDEXTx& dexTx);
    void AddLiquidityOutput(CTransaction& tx, const CDEXTx& dexTx);
};

// Global DEX manager instance
extern std::unique_ptr<CDEXManager> g_dexManager;

// Utility functions
namespace DEXUtils {
    // Initialize DEX system
    bool InitializeDEXSystem();
    
    // Shutdown DEX system
    void ShutdownDEXSystem();
    
    // Check if DEX system is enabled
    bool IsDEXSystemEnabled();
    
    // Enable/disable DEX system
    void SetDEXSystemEnabled(bool enabled);
    
    // DEX helpers
    bool CanCreatePool(const CTxDestination& creator, const uint256& tokenHash, CAmount tokenAmount, CAmount shahAmount);
    bool CanAddLiquidity(const uint256& poolHash, const CTxDestination& provider, CAmount tokenAmount, CAmount shahAmount);
    bool CanSwap(const uint256& poolHash, const CTxDestination& trader, CAmount amount, bool isTokenToShah);
    double GetSwapFee();
    double GetLiquidityFee();
    
    // Transaction validation
    bool IsDEXTransaction(const CTransaction& tx);
    bool ValidateDEXTransaction(const CTransaction& tx);
    
    // Get human-readable transaction type name
    std::string GetDEXTxTypeName(DEXTxType type);
    
    // Price formatting
    std::string FormatPrice(double price);
    std::string FormatAmount(CAmount amount);
    std::string FormatPercentage(double percentage);
}

#endif // SHAHCOIN_DEX_DEX_H
