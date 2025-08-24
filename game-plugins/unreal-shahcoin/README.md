# Unreal Engine Shahcoin Plugin

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.0%2B-green.svg)](https://www.unrealengine.com/)

Unreal Engine plugin for integrating Shahcoin blockchain functionality into your games.

## Features

- 🔗 **Wallet Integration** - Connect to Shahcoin wallets from Unreal Engine
- 💰 **Balance Checking** - Check SHAH and token balances in real-time
- 🎮 **NFT Ownership** - Verify NFT ownership for in-game items
- 🎯 **Blueprint Support** - Full Blueprint visual scripting integration
- 🎁 **ShahSwap Rewards** - Integrate with ShahSwap for rewards/boosts
- 🔑 **C++ API** - High-performance native C++ implementation
- 🎨 **Editor Tools** - In-editor wallet management and testing
- 🌐 **Multiplayer Support** - Network-synchronized blockchain state

## Installation

### Method 1: Marketplace Installation

1. Open Unreal Engine and go to Marketplace
2. Search for "Shahcoin Plugin"
3. Click "Add to Project" and install

### Method 2: Manual Installation

1. Download the plugin files
2. Copy the `ShahcoinPlugin` folder to your project's `Plugins` directory
3. Restart Unreal Engine
4. Enable the plugin in Edit > Plugins > Shahcoin Plugin

## Quick Start

### 1. Setup API Key

#### Blueprint Method
```cpp
// In Blueprint, call the static function
UShahcoinManager::Initialize("your-api-key-here");
```

#### C++ Method
```cpp
// In C++ code
UShahcoinManager::Initialize("your-api-key-here");
```

### 2. Connect Wallet

#### Blueprint Method
1. Create a Blueprint class
2. Add "Connect Wallet" node
3. Connect to "On Wallet Connected" event

#### C++ Method
```cpp
// Connect to wallet
UShahcoinManager::GetInstance()->ConnectWallet(
    FShahcoinWalletCallback::CreateLambda([](bool bSuccess, const FString& Address) {
        if (bSuccess) {
            UE_LOG(LogTemp, Log, TEXT("Connected to wallet: %s"), *Address);
        } else {
            UE_LOG(LogTemp, Error, TEXT("Failed to connect wallet"));
        }
    })
);
```

### 3. Check Balance

#### Blueprint Method
1. Add "Get Balance" node
2. Connect to "On Balance Received" event

#### C++ Method
```cpp
// Check SHAH balance
UShahcoinManager::GetInstance()->GetBalance(
    FShahcoinBalanceCallback::CreateLambda([](float Balance) {
        UE_LOG(LogTemp, Log, TEXT("SHAH Balance: %f"), Balance);
    })
);
```

### 4. Check NFT Ownership

#### Blueprint Method
1. Add "Has NFT" node
2. Set Contract Address and Token ID
3. Connect to "On NFT Check Complete" event

#### C++ Method
```cpp
// Check if player owns a specific NFT
UShahcoinManager::GetInstance()->HasNFT(
    "NFT_CONTRACT_ADDRESS",
    "TOKEN_ID",
    FShahcoinNFTCallback::CreateLambda([](bool bHasNFT) {
        if (bHasNFT) {
            UE_LOG(LogTemp, Log, TEXT("Player owns this NFT!"));
            // Unlock special content
        }
    })
);
```

## API Reference

### UShahcoinManager

Main singleton class for managing Shahcoin integration.

#### Static Methods

```cpp
// Initialize the plugin with API key
static void Initialize(const FString& ApiKey);

// Set the network to use
static void SetNetwork(EShahcoinNetwork Network);

// Get the singleton instance
static UShahcoinManager* GetInstance();
```

#### Instance Methods

##### Wallet Operations

```cpp
// Connect to wallet
void ConnectWallet(const FShahcoinWalletCallback& Callback);

// Disconnect wallet
void DisconnectWallet();

// Check if wallet is connected
bool IsWalletConnected() const;

// Get wallet address
FString GetWalletAddress() const;
```

##### Balance Operations

```cpp
// Get SHAH balance
void GetBalance(const FShahcoinBalanceCallback& Callback);

// Get token balance
void GetTokenBalance(const FString& ContractAddress, const FShahcoinBalanceCallback& Callback);

// Get NFT count
void GetNFTCount(const FString& ContractAddress, const FShahcoinNFTCountCallback& Callback);
```

##### NFT Operations

```cpp
// Check NFT ownership
void HasNFT(const FString& ContractAddress, const FString& TokenId, const FShahcoinNFTCallback& Callback);

// Get owned NFTs
void GetOwnedNFTs(const FString& ContractAddress, const FShahcoinNFTArrayCallback& Callback);

// Get NFT metadata
void GetNFTMetadata(const FString& ContractAddress, const FString& TokenId, const FShahcoinNFTMetadataCallback& Callback);
```

##### ShahSwap Integration

```cpp
// Get token price
void GetTokenPrice(const FString& TokenSymbol, const FShahcoinPriceCallback& Callback);

// Check if player can afford item
void CanAffordItem(const FString& TokenSymbol, float Amount, const FShahcoinAffordabilityCallback& Callback);

// Get swap quote
void GetSwapQuote(const FString& InputToken, const FString& OutputToken, float Amount, const FShahcoinSwapQuoteCallback& Callback);
```

### Data Structures

#### FShahcoinNFTInfo

```cpp
USTRUCT(BlueprintType)
struct FShahcoinNFTInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString TokenId;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString ImageUrl;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    TMap<FString, FString> Attributes;
};
```

#### FShahcoinSwapQuote

```cpp
USTRUCT(BlueprintType)
struct FShahcoinSwapQuote
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString InputToken;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    FString OutputToken;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    float InputAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    float OutputAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    float PriceImpact;

    UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
    float Fee;
};
```

## Blueprint Examples

### VIP System

```cpp
// In Blueprint
void APlayerController::CheckVIPStatus()
{
    // Get balance
    UShahcoinManager::GetInstance()->GetBalance(
        FShahcoinBalanceCallback::CreateLambda([this](float Balance) {
            if (Balance >= 1000.0f) {
                EnableVIPFeatures();
            }
        })
    );
}

void APlayerController::EnableVIPFeatures()
{
    // Enable VIP-only features
    UE_LOG(LogTemp, Log, TEXT("VIP features enabled!"));
}
```

### NFT-Based Items

```cpp
// In Blueprint
void AItemManager::CheckItemAccess(const FString& NFTContractAddress, const FString& RequiredTokenId)
{
    UShahcoinManager::GetInstance()->HasNFT(
        NFTContractAddress,
        RequiredTokenId,
        FShahcoinNFTCallback::CreateLambda([this](bool bHasNFT) {
            if (bHasNFT) {
                UnlockItem();
            } else {
                ShowPurchasePrompt();
            }
        })
    );
}

void AItemManager::UnlockItem()
{
    // Unlock the item for the player
    UE_LOG(LogTemp, Log, TEXT("Item unlocked!"));
}

void AItemManager::ShowPurchasePrompt()
{
    // Show purchase prompt
    UE_LOG(LogTemp, Log, TEXT("Purchase this NFT to unlock the item!"));
}
```

### Token-Based Economy

```cpp
// In Blueprint
void AShopManager::PurchaseItem(const FString& GameTokenAddress, float ItemCost)
{
    UShahcoinManager::GetInstance()->GetTokenBalance(
        GameTokenAddress,
        FShahcoinBalanceCallback::CreateLambda([this, ItemCost](float Balance) {
            if (Balance >= ItemCost) {
                ProcessPurchase();
            } else {
                ShowInsufficientFunds();
            }
        })
    );
}

void AShopManager::ProcessPurchase()
{
    // Process the purchase
    UE_LOG(LogTemp, Log, TEXT("Purchase successful!"));
}

void AShopManager::ShowInsufficientFunds()
{
    UE_LOG(LogTemp, Log, TEXT("Insufficient funds!"));
}
```

## C++ Examples

### Advanced Wallet Management

```cpp
// Custom wallet manager class
UCLASS(BlueprintType)
class UCustomWalletManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Shahcoin")
    void InitializeWallet();

    UFUNCTION(BlueprintCallable, Category = "Shahcoin")
    void CheckPlayerStatus();

private:
    void OnWalletConnected(bool bSuccess, const FString& Address);
    void OnBalanceReceived(float Balance);
    void OnNFTReceived(const TArray<FShahcoinNFTInfo>& NFTs);
};

void UCustomWalletManager::InitializeWallet()
{
    // Initialize with API key
    UShahcoinManager::Initialize("your-api-key");
    
    // Connect wallet
    UShahcoinManager::GetInstance()->ConnectWallet(
        FShahcoinWalletCallback::CreateUObject(this, &UCustomWalletManager::OnWalletConnected)
    );
}

void UCustomWalletManager::OnWalletConnected(bool bSuccess, const FString& Address)
{
    if (bSuccess) {
        UE_LOG(LogTemp, Log, TEXT("Wallet connected: %s"), *Address);
        CheckPlayerStatus();
    }
}

void UCustomWalletManager::CheckPlayerStatus()
{
    // Check balance
    UShahcoinManager::GetInstance()->GetBalance(
        FShahcoinBalanceCallback::CreateUObject(this, &UCustomWalletManager::OnBalanceReceived)
    );
}

void UCustomWalletManager::OnBalanceReceived(float Balance)
{
    if (Balance >= 100.0f) {
        // Player has VIP status
        EnableVIPFeatures();
    }
}
```

## Editor Integration

### In-Editor Wallet Testing

1. Open the Shahcoin Plugin panel in the editor
2. Enter your API key
3. Test wallet connections
4. Verify NFT ownership
5. Check token balances

### Blueprint Debugging

- Use the Shahcoin Debug panel to monitor API calls
- View real-time wallet status
- Test NFT verification
- Monitor transaction history

## Configuration

### Network Settings

```cpp
// Set network (default: Mainnet)
UShahcoinManager::SetNetwork(EShahcoinNetwork::Mainnet);
UShahcoinManager::SetNetwork(EShahcoinNetwork::Testnet);
UShahcoinManager::SetNetwork(EShahcoinNetwork::Regtest);
```

### Error Handling

```cpp
// Subscribe to error events
UShahcoinManager::GetInstance()->OnError.AddDynamic(this, &AMyGameMode::OnShahcoinError);

// Subscribe to connection events
UShahcoinManager::GetInstance()->OnWalletConnected.AddDynamic(this, &AMyGameMode::OnWalletConnected);
UShahcoinManager::GetInstance()->OnWalletDisconnected.AddDynamic(this, &AMyGameMode::OnWalletDisconnected);
```

## Multiplayer Support

### Network Synchronization

```cpp
// Replicate wallet state across network
UFUNCTION(Server, Reliable)
void Server_UpdateWalletState(const FString& WalletAddress, float Balance);

// Client receives wallet updates
UFUNCTION(Client, Reliable)
void Client_ReceiveWalletUpdate(const FString& WalletAddress, float Balance);
```

### Authority Management

```cpp
// Only server can perform wallet operations
if (HasAuthority()) {
    UShahcoinManager::GetInstance()->ConnectWallet(Callback);
}
```

## Performance Optimization

### Caching

```cpp
// Cache balance for performance
UPROPERTY(Replicated)
float CachedBalance;

// Update cache periodically
void APlayerController::UpdateBalanceCache()
{
    UShahcoinManager::GetInstance()->GetBalance(
        FShahcoinBalanceCallback::CreateLambda([this](float Balance) {
            CachedBalance = Balance;
        })
    );
}
```

### Async Operations

```cpp
// Use async operations for non-blocking calls
void AMyGameMode::CheckPlayerNFTs()
{
    UShahcoinManager::GetInstance()->GetOwnedNFTs(
        "NFT_CONTRACT",
        FShahcoinNFTArrayCallback::CreateLambda([this](const TArray<FShahcoinNFTInfo>& NFTs) {
            // Process NFTs asynchronously
            ProcessNFTs(NFTs);
        })
    );
}
```

## Best Practices

1. **Always check wallet connection** before making API calls
2. **Handle errors gracefully** and provide user feedback
3. **Cache results** when possible to reduce API calls
4. **Use appropriate networks** (testnet for development)
5. **Secure your API keys** and never expose them in client builds
6. **Test thoroughly** in multiplayer environments
7. **Optimize for performance** with proper caching and async operations

## Support

- 📖 **Documentation**: [https://shah.vip/dev](https://shah.vip/dev)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)
- 🎮 **Unreal Forums**: [Unreal Engine Community](https://forums.unrealengine.com/)

## License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
