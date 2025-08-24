using System;
using System.Collections.Generic;

namespace ShahcoinPlugin
{
    /// <summary>
    /// Information about an NFT
    /// </summary>
    [System.Serializable]
    public class NFTInfo
    {
        public string tokenId;
        public string name;
        public string description;
        public string imageUrl;
        public string owner;
        public string contractAddress;
        public long creationTime;
        public Dictionary<string, object> attributes;
        public Dictionary<string, object> metadata;
    }

    /// <summary>
    /// NFT metadata information
    /// </summary>
    [System.Serializable]
    public class NFTMetadata
    {
        public string name;
        public string description;
        public string image;
        public Dictionary<string, object> attributes;
        public Dictionary<string, object> properties;
    }

    /// <summary>
    /// ShahSwap quote information
    /// </summary>
    [System.Serializable]
    public class SwapQuote
    {
        public string inputToken;
        public string outputToken;
        public decimal inputAmount;
        public decimal outputAmount;
        public decimal priceImpact;
        public decimal fee;
        public string[] route;
        public long validUntil;
    }

    /// <summary>
    /// Token information
    /// </summary>
    [System.Serializable]
    public class TokenInfo
    {
        public string symbol;
        public string name;
        public int decimals;
        public decimal totalSupply;
        public string contractAddress;
        public string creator;
        public long creationTime;
    }

    /// <summary>
    /// Transaction information
    /// </summary>
    [System.Serializable]
    public class TransactionInfo
    {
        public string txid;
        public decimal amount;
        public decimal fee;
        public int confirmations;
        public long blocktime;
        public string address;
        public string category;
    }

    /// <summary>
    /// Staking information
    /// </summary>
    [System.Serializable]
    public class StakingInfo
    {
        public bool isStaking;
        public decimal stakedAmount;
        public decimal rewardsEarned;
        public long? lastRewardTime;
        public long? nextRewardEstimate;
        public string stakingAddress;
    }

    /// <summary>
    /// Network information
    /// </summary>
    [System.Serializable]
    public class NetworkInfo
    {
        public int chainId;
        public string name;
        public string rpcUrl;
        public string explorerUrl;
        public string shahSwapUrl;
    }

    /// <summary>
    /// API response wrapper
    /// </summary>
    [System.Serializable]
    public class ApiResponse<T>
    {
        public bool success;
        public T data;
        public string error;
        public string message;
    }

    /// <summary>
    /// Wallet connection information
    /// </summary>
    [System.Serializable]
    public class WalletConnection
    {
        public bool isConnected;
        public string address;
        public decimal balance;
        public string network;
    }

    /// <summary>
    /// Fee estimate information
    /// </summary>
    [System.Serializable]
    public class FeeEstimate
    {
        public decimal low;
        public decimal medium;
        public decimal high;
        public decimal? custom;
    }

    /// <summary>
    /// ShahSwap trade information
    /// </summary>
    [System.Serializable]
    public class ShahSwapTrade
    {
        public string quoteId;
        public string inputToken;
        public string outputToken;
        public decimal inputAmount;
        public decimal outputAmount;
        public string txid;
        public string status;
        public long timestamp;
    }

    /// <summary>
    /// Create token parameters
    /// </summary>
    [System.Serializable]
    public class CreateTokenParams
    {
        public string name;
        public string symbol;
        public int decimals;
        public decimal totalSupply;
        public string description;
        public string website;
    }

    /// <summary>
    /// Create NFT parameters
    /// </summary>
    [System.Serializable]
    public class CreateNFTParams
    {
        public string name;
        public string description;
        public string imageUrl;
        public Dictionary<string, object> metadata;
        public NFTAttribute[] attributes;
    }

    /// <summary>
    /// NFT attribute
    /// </summary>
    [System.Serializable]
    public class NFTAttribute
    {
        public string trait_type;
        public object value;
    }

    /// <summary>
    /// Send transaction parameters
    /// </summary>
    [System.Serializable]
    public class SendTransactionParams
    {
        public string to;
        public decimal amount;
        public decimal? fee;
        public string memo;
    }

    /// <summary>
    /// ShahSwap parameters
    /// </summary>
    [System.Serializable]
    public class ShahSwapParams
    {
        public string inputToken;
        public string outputToken;
        public decimal amount;
        public decimal? slippageTolerance;
        public long? deadline;
    }

    /// <summary>
    /// Wallet event information
    /// </summary>
    [System.Serializable]
    public class WalletEvent
    {
        public string type;
        public object data;
        public long timestamp;
    }

    /// <summary>
    /// Game-specific configuration
    /// </summary>
    [System.Serializable]
    public class GameConfig
    {
        public string gameId;
        public string apiKey;
        public ShahcoinNetwork network;
        public string customRpcUrl;
        public float requestTimeout;
        public bool enableLogging;
        public bool enableCaching;
        public int cacheTimeout;
    }

    /// <summary>
    /// In-game item information
    /// </summary>
    [System.Serializable]
    public class GameItem
    {
        public string itemId;
        public string name;
        public string description;
        public string imageUrl;
        public decimal price;
        public string currency; // "SHAH" or token contract address
        public bool requiresNFT;
        public string nftContractAddress;
        public string nftTokenId;
        public Dictionary<string, object> metadata;
    }

    /// <summary>
    /// Player wallet status
    /// </summary>
    [System.Serializable]
    public class PlayerWalletStatus
    {
        public bool isConnected;
        public string address;
        public decimal shahBalance;
        public Dictionary<string, decimal> tokenBalances;
        public Dictionary<string, NFTInfo[]> ownedNFTs;
        public StakingInfo stakingInfo;
        public bool isVIP;
        public decimal vipThreshold;
    }

    /// <summary>
    /// Game reward information
    /// </summary>
    [System.Serializable]
    public class GameReward
    {
        public string rewardId;
        public string name;
        public string description;
        public decimal amount;
        public string currency;
        public string type; // "SHAH", "TOKEN", "NFT"
        public string nftContractAddress;
        public string nftTokenId;
        public bool isClaimed;
        public long? claimedAt;
        public long expiresAt;
    }

    /// <summary>
    /// Game achievement information
    /// </summary>
    [System.Serializable]
    public class GameAchievement
    {
        public string achievementId;
        public string name;
        public string description;
        public string imageUrl;
        public bool isUnlocked;
        public long? unlockedAt;
        public GameReward reward;
        public Dictionary<string, object> requirements;
    }
}
