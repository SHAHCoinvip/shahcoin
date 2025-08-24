# Unity Shahcoin Plugin

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Unity Version](https://img.shields.io/badge/Unity-2022.3%2B-green.svg)](https://unity.com/)

Unity plugin for integrating Shahcoin blockchain functionality into your games.

## Features

- 🔗 **Wallet Integration** - Connect to Shahcoin wallets
- 💰 **Balance Checking** - Check SHAH and token balances
- 🎮 **NFT Ownership** - Verify NFT ownership for in-game items
- 🎯 **In-Game Actions** - Trigger game events based on wallet state
- 🎁 **ShahSwap Rewards** - Integrate with ShahSwap for rewards/boosts
- 🔑 **API Key Authentication** - Secure integration with API keys

## Installation

### Method 1: Unity Package Manager

1. Open Unity Package Manager (Window > Package Manager)
2. Click the "+" button and select "Add package from git URL"
3. Enter: `https://github.com/SHAHCoinvip/shahcoin.git`

### Method 2: Manual Installation

1. Download the plugin files
2. Copy the `ShahcoinPlugin` folder to your Unity project's `Assets` folder
3. Import the package in Unity

## Quick Start

### 1. Setup API Key

```csharp
using ShahcoinPlugin;

// Initialize the plugin with your API key
ShahcoinManager.Initialize("your-api-key-here");
```

### 2. Connect Wallet

```csharp
// Connect to a wallet
ShahcoinManager.Instance.ConnectWallet((success, address) => {
    if (success) {
        Debug.Log($"Connected to wallet: {address}");
    } else {
        Debug.Log("Failed to connect wallet");
    }
});
```

### 3. Check Balance

```csharp
// Check SHAH balance
ShahcoinManager.Instance.GetBalance((balance) => {
    Debug.Log($"SHAH Balance: {balance}");
});

// Check token balance
ShahcoinManager.Instance.GetTokenBalance("TOKEN_CONTRACT_ADDRESS", (balance) => {
    Debug.Log($"Token Balance: {balance}");
});
```

### 4. Check NFT Ownership

```csharp
// Check if player owns a specific NFT
ShahcoinManager.Instance.HasNFT("NFT_CONTRACT_ADDRESS", "TOKEN_ID", (hasNFT) => {
    if (hasNFT) {
        Debug.Log("Player owns this NFT!");
        // Unlock special content
        UnlockSpecialContent();
    }
});
```

### 5. In-Game Actions

```csharp
// Trigger game action based on wallet state
public void CheckPlayerStatus() {
    ShahcoinManager.Instance.GetBalance((balance) => {
        if (balance >= 100) {
            // Player has VIP status
            EnableVIPFeatures();
        }
    });
}
```

## API Reference

### ShahcoinManager

Main class for managing Shahcoin integration.

#### Static Methods

- `Initialize(string apiKey)` - Initialize the plugin with API key
- `Instance` - Get the singleton instance

#### Instance Methods

##### Wallet Operations

```csharp
// Connect to wallet
void ConnectWallet(Action<bool, string> callback)

// Disconnect wallet
void DisconnectWallet()

// Check if wallet is connected
bool IsWalletConnected()

// Get wallet address
string GetWalletAddress()
```

##### Balance Operations

```csharp
// Get SHAH balance
void GetBalance(Action<decimal> callback)

// Get token balance
void GetTokenBalance(string contractAddress, Action<decimal> callback)

// Get NFT count
void GetNFTCount(string contractAddress, Action<int> callback)
```

##### NFT Operations

```csharp
// Check NFT ownership
void HasNFT(string contractAddress, string tokenId, Action<bool> callback)

// Get owned NFTs
void GetOwnedNFTs(string contractAddress, Action<NFTInfo[]> callback)

// Get NFT metadata
void GetNFTMetadata(string contractAddress, string tokenId, Action<NFTMetadata> callback)
```

##### ShahSwap Integration

```csharp
// Get token price
void GetTokenPrice(string tokenSymbol, Action<decimal> callback)

// Check if player can afford item
void CanAffordItem(string tokenSymbol, decimal amount, Action<bool> callback)

// Get swap quote
void GetSwapQuote(string inputToken, string outputToken, decimal amount, Action<SwapQuote> callback)
```

### Data Structures

#### NFTInfo

```csharp
[System.Serializable]
public class NFTInfo {
    public string tokenId;
    public string name;
    public string description;
    public string imageUrl;
    public Dictionary<string, object> attributes;
}
```

#### NFTMetadata

```csharp
[System.Serializable]
public class NFTMetadata {
    public string name;
    public string description;
    public string image;
    public Dictionary<string, object> attributes;
}
```

#### SwapQuote

```csharp
[System.Serializable]
public class SwapQuote {
    public string inputToken;
    public string outputToken;
    public decimal inputAmount;
    public decimal outputAmount;
    public decimal priceImpact;
    public decimal fee;
}
```

## Examples

### VIP System

```csharp
public class VIPSystem : MonoBehaviour {
    void Start() {
        CheckVIPStatus();
    }

    void CheckVIPStatus() {
        ShahcoinManager.Instance.GetBalance((balance) => {
            if (balance >= 1000) {
                EnableVIPFeatures();
            }
        });
    }

    void EnableVIPFeatures() {
        // Enable VIP-only features
        Debug.Log("VIP features enabled!");
    }
}
```

### NFT-Based Items

```csharp
public class NFTItemSystem : MonoBehaviour {
    [SerializeField] private string nftContractAddress = "NFT_CONTRACT_ADDRESS";
    [SerializeField] private string requiredTokenId = "1";

    public void CheckItemAccess() {
        ShahcoinManager.Instance.HasNFT(nftContractAddress, requiredTokenId, (hasNFT) => {
            if (hasNFT) {
                UnlockItem();
            } else {
                ShowPurchasePrompt();
            }
        });
    }

    void UnlockItem() {
        // Unlock the item for the player
        Debug.Log("Item unlocked!");
    }

    void ShowPurchasePrompt() {
        // Show purchase prompt
        Debug.Log("Purchase this NFT to unlock the item!");
    }
}
```

### Token-Based Economy

```csharp
public class TokenEconomy : MonoBehaviour {
    [SerializeField] private string gameTokenAddress = "GAME_TOKEN_ADDRESS";
    [SerializeField] private decimal itemCost = 10.0m;

    public void PurchaseItem() {
        ShahcoinManager.Instance.GetTokenBalance(gameTokenAddress, (balance) => {
            if (balance >= itemCost) {
                ProcessPurchase();
            } else {
                ShowInsufficientFunds();
            }
        });
    }

    void ProcessPurchase() {
        // Process the purchase
        Debug.Log("Purchase successful!");
    }

    void ShowInsufficientFunds() {
        Debug.Log("Insufficient funds!");
    }
}
```

## Configuration

### Network Settings

The plugin supports multiple networks:

```csharp
// Set network (default: mainnet)
ShahcoinManager.SetNetwork(ShahcoinNetwork.Mainnet);
ShahcoinManager.SetNetwork(ShahcoinNetwork.Testnet);
ShahcoinManager.SetNetwork(ShahcoinNetwork.Regtest);
```

### Error Handling

```csharp
// Subscribe to error events
ShahcoinManager.Instance.OnError += (error) => {
    Debug.LogError($"Shahcoin Error: {error}");
};

// Subscribe to connection events
ShahcoinManager.Instance.OnWalletConnected += (address) => {
    Debug.Log($"Wallet connected: {address}");
};

ShahcoinManager.Instance.OnWalletDisconnected += () => {
    Debug.Log("Wallet disconnected");
};
```

## Best Practices

1. **Always check wallet connection** before making API calls
2. **Handle errors gracefully** and provide user feedback
3. **Cache results** when possible to reduce API calls
4. **Use appropriate networks** (testnet for development)
5. **Secure your API keys** and never expose them in client builds

## Support

- 📖 **Documentation**: [https://shah.vip/dev](https://shah.vip/dev)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
