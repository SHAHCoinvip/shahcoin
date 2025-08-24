# SHAHCOIN Unity Plugin

A comprehensive Unity plugin for integrating SHAHCOIN blockchain functionality into your games. This plugin provides wallet integration, NFT display, and token reward systems.

## Features

- **Wallet Integration**: Connect to SHAHCOIN wallets and manage balances
- **NFT Gallery**: Display and interact with NFTs in your game
- **Token Rewards**: Implement token-based reward systems for achievements
- **Transaction Management**: Send SHAH and interact with smart contracts
- **Real-time Updates**: Automatic balance and transaction updates

## Installation

1. Download the SHAHCOIN Unity Plugin package
2. Import the package into your Unity project
3. Add the `SHAHCOINWallet` component to a GameObject in your scene
4. Configure the network settings (mainnet, testnet, or regtest)

## Quick Start

### Basic Wallet Integration

```csharp
using SHAHCOIN;
using UnityEngine;

public class WalletManager : MonoBehaviour
{
    [SerializeField] private SHAHCOINWallet wallet;
    
    void Start()
    {
        // Subscribe to wallet events
        wallet.OnWalletConnected.AddListener(OnWalletConnected);
        wallet.OnBalanceUpdated.AddListener(OnBalanceUpdated);
        wallet.OnError.AddListener(OnError);
    }
    
    void OnWalletConnected(string address)
    {
        Debug.Log($"Wallet connected: {address}");
    }
    
    void OnBalanceUpdated(decimal balance)
    {
        Debug.Log($"Balance updated: {balance} SHAH");
    }
    
    void OnError(string error)
    {
        Debug.LogError($"Wallet error: {error}");
    }
    
    // Send SHAH to another address
    public void SendSHAH(string toAddress, decimal amount)
    {
        StartCoroutine(wallet.SendSHAH(toAddress, amount));
    }
}
```

### NFT Gallery Integration

```csharp
public class NFTGallery : MonoBehaviour
{
    [SerializeField] private SHAHCOINWallet wallet;
    
    void Start()
    {
        // Get NFT balance when wallet connects
        wallet.OnWalletConnected.AddListener((address) => {
            StartCoroutine(LoadNFTs());
        });
    }
    
    IEnumerator LoadNFTs()
    {
        yield return StartCoroutine(wallet.GetNFTBalance());
        // Handle NFT data display
    }
}
```

### Token Reward System

```csharp
public class AchievementSystem : MonoBehaviour
{
    [SerializeField] private SHAHCOINWallet wallet;
    
    public void AwardTokens(decimal amount)
    {
        // In a real implementation, this would mint tokens to the player
        Debug.Log($"Awarded {amount} tokens to player");
    }
    
    public void OnEnemyDefeated()
    {
        // Award tokens for defeating enemies
        AwardTokens(10.0m);
    }
}
```

## Configuration

### Network Settings

- **RPC URL**: The SHAHCOIN node RPC endpoint (default: `http://localhost:8368`)
- **Network**: Choose between mainnet, testnet, or regtest
- **Auto Connect**: Automatically connect to wallet on start
- **Balance Update Interval**: How often to update balance (in seconds)

### Wallet Events

The plugin provides several events you can subscribe to:

- `OnWalletConnected(string address)`: Fired when wallet connects
- `OnWalletDisconnected(string reason)`: Fired when wallet disconnects
- `OnBalanceUpdated(decimal balance)`: Fired when balance changes
- `OnTransactionSent(string txid)`: Fired when transaction is sent
- `OnError(string error)`: Fired when an error occurs

## Samples

The plugin includes several sample scenes:

1. **Basic Wallet**: Simple wallet connection and balance display
2. **NFT Gallery**: Display NFTs in a gallery interface
3. **Token Rewards**: Token reward system for game achievements

## API Reference

### SHAHCOINWallet Class

#### Properties
- `bool IsConnected`: Current connection status
- `string CurrentAddress`: Connected wallet address
- `decimal CurrentBalance`: Current wallet balance

#### Methods
- `IEnumerator ConnectWallet()`: Connect to wallet
- `void DisconnectWallet()`: Disconnect from wallet
- `IEnumerator GetBalance()`: Get current balance
- `IEnumerator SendSHAH(string toAddress, decimal amount)`: Send SHAH
- `IEnumerator GetNFTBalance()`: Get NFT balance

#### Events
- `UnityEvent<string> OnWalletConnected`
- `UnityEvent<string> OnWalletDisconnected`
- `UnityEvent<decimal> OnBalanceUpdated`
- `UnityEvent<string> OnTransactionSent`
- `UnityEvent<string> OnError`

## Best Practices

1. **Error Handling**: Always subscribe to the `OnError` event to handle errors gracefully
2. **UI Updates**: Use the balance update events to refresh your UI
3. **Network Calls**: Use coroutines for all blockchain operations
4. **Testing**: Use testnet for development and testing
5. **Security**: Never store private keys in your game

## Troubleshooting

### Common Issues

1. **Connection Failed**: Check your RPC URL and network settings
2. **Balance Not Updating**: Ensure the wallet is connected and auto-update is enabled
3. **Transaction Failed**: Verify sufficient balance and correct address format

### Debug Mode

Enable debug logging in the Unity Console to see detailed information about wallet operations.

## Support

For support and questions:
- Documentation: https://docs.shahcoin.network
- Discord: https://discord.gg/shahcoin
- Email: dev@shahcoin.network

## License

This plugin is licensed under the Apache 2.0 License. See the LICENSE file for details.

## Contributing

We welcome contributions! Please see our contributing guidelines for more information.
