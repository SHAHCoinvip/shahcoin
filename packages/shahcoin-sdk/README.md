# Shahcoin dApp SDK

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Version](https://img.shields.io/npm/v/shahcoin-sdk.svg)](https://www.npmjs.com/package/shahcoin-sdk)
[![License](https://img.shields.io/npm/l/shahcoin-sdk.svg)](https://opensource.org/licenses/MIT)

Official Shahcoin dApp SDK for wallet connectivity, ShahSwap integration, and blockchain interactions.

## Features

- 🔗 **Wallet Connectivity** - Connect to Shahcoin Qt Wallet or Bridge
- 💱 **ShahSwap Integration** - Get quotes and execute token swaps
- 🎨 **NFT Management** - Create, view, and manage NFTs
- 🪙 **Token Operations** - Create and manage custom tokens
- 📊 **Staking** - Stake SHAH and earn rewards
- 📈 **Transaction History** - Get detailed transaction information
- 🔔 **Real-time Events** - WebSocket-based event system
- 🌐 **Multi-network Support** - Mainnet, Testnet, and Regtest

## Installation

```bash
npm install shahcoin-sdk
```

## Quick Start

### Connect to Wallet

```typescript
import { ShahcoinSDK, NETWORKS } from 'shahcoin-sdk';

// Initialize SDK
const sdk = new ShahcoinSDK({
  network: 'mainnet',
  apiKey: 'your-api-key' // Optional
});

// Connect to wallet
const connection = await sdk.connect();
console.log('Connected to wallet:', connection.address);
```

### Check Balance

```typescript
// Get SHAH balance
const balance = await sdk.getBalance();
console.log('SHAH Balance:', balance);

// Get staking information
const stakingInfo = await sdk.getStakingInfo();
console.log('Staking Status:', stakingInfo.isStaking);
```

### Send Transaction

```typescript
// Send SHAH to another address
const txid = await sdk.sendTransaction({
  to: 'S...', // Recipient address
  amount: 100, // Amount in SHAH
  fee: 0.001, // Optional fee
  memo: 'Payment for services' // Optional memo
});

console.log('Transaction sent:', txid);
```

### ShahSwap Integration

```typescript
// Get swap quote
const quote = await sdk.getShahSwapQuote({
  inputToken: 'SHAH',
  outputToken: 'TOKEN_A',
  amount: 10
});

console.log('Swap Quote:', quote);

// Execute swap
const trade = await sdk.executeShahSwap({
  inputToken: 'SHAH',
  outputToken: 'TOKEN_A',
  amount: 10,
  slippageTolerance: 0.5 // 0.5%
});

console.log('Swap executed:', trade.txid);
```

### Create Token

```typescript
// Create a new token
const token = await sdk.createToken({
  name: 'My Token',
  symbol: 'MTK',
  decimals: 18,
  totalSupply: 1000000,
  description: 'My custom token',
  website: 'https://mytoken.com'
});

console.log('Token created:', token.contractAddress);
```

### Create NFT

```typescript
// Create a new NFT
const nft = await sdk.createNFT({
  name: 'My NFT',
  description: 'A unique digital asset',
  imageUrl: 'https://example.com/image.png',
  attributes: [
    { trait_type: 'Rarity', value: 'Legendary' },
    { trait_type: 'Level', value: 100 }
  ]
});

console.log('NFT created:', nft.tokenId);
```

### Event Handling

```typescript
// Listen for wallet events
sdk.on('balanceChanged', (event) => {
  console.log('Balance changed:', event.data);
});

sdk.on('transactionReceived', (event) => {
  console.log('New transaction:', event.data);
});

sdk.on('stakingReward', (event) => {
  console.log('Staking reward received:', event.data);
});

// Listen for connection changes
sdk.onConnectionChange((connection) => {
  console.log('Connection status:', connection.isConnected);
});
```

## API Reference

### ShahcoinSDK

#### Constructor

```typescript
new ShahcoinSDK(config?: WalletConfig)
```

**Config Options:**
- `rpcUrl?: string` - Custom RPC URL
- `apiKey?: string` - API key for authentication
- `network?: 'mainnet' | 'testnet' | 'regtest'` - Network to connect to
- `timeout?: number` - Request timeout in milliseconds

#### Methods

##### Wallet Operations

- `connect(): Promise<WalletConnection>` - Connect to wallet
- `disconnect(): Promise<void>` - Disconnect from wallet
- `getConnection(): WalletConnection` - Get current connection status
- `getBalance(): Promise<number>` - Get SHAH balance
- `getTransactionHistory(limit?: number, offset?: number): Promise<Transaction[]>` - Get transaction history
- `sendTransaction(params: SendTransactionParams): Promise<string>` - Send SHAH transaction

##### Staking

- `getStakingInfo(): Promise<StakingInfo>` - Get staking information
- `stake(amount: number): Promise<string>` - Stake SHAH

##### ShahSwap

- `getShahSwapQuote(params: ShahSwapParams): Promise<ShahSwapQuote>` - Get swap quote
- `executeShahSwap(params: ShahSwapParams): Promise<ShahSwapTrade>` - Execute swap

##### Tokens

- `createToken(params: CreateTokenParams): Promise<TokenInfo>` - Create new token
- `getTokenInfo(contractAddress: string): Promise<TokenInfo>` - Get token information
- `getUserTokens(): Promise<TokenInfo[]>` - Get user's tokens

##### NFTs

- `createNFT(params: CreateNFTParams): Promise<NFTInfo>` - Create new NFT
- `getNFTInfo(contractAddress: string, tokenId: string): Promise<NFTInfo>` - Get NFT information
- `getUserNFTs(): Promise<NFTInfo[]>` - Get user's NFTs

##### Utilities

- `getFeeEstimates(): Promise<FeeEstimate>` - Get fee estimates
- `getNetworkInfo(): NetworkInfo` - Get network information

##### Events

- `on(event: string, callback: EventCallback): void` - Subscribe to events
- `onConnectionChange(callback: ConnectionCallback): void` - Subscribe to connection changes

## Networks

The SDK supports multiple networks:

```typescript
import { NETWORKS } from 'shahcoin-sdk';

// Mainnet
const mainnet = NETWORKS.mainnet;

// Testnet
const testnet = NETWORKS.testnet;

// Regtest (local development)
const regtest = NETWORKS.regtest;
```

## Error Handling

The SDK throws descriptive errors for various scenarios:

```typescript
try {
  const balance = await sdk.getBalance();
} catch (error) {
  if (error.message.includes('Wallet not connected')) {
    // Handle connection error
  } else if (error.message.includes('Failed to get balance')) {
    // Handle API error
  }
}
```

## Development

### Building

```bash
npm run build
```

### Testing

```bash
npm test
```

### Linting

```bash
npm run lint
```

## Examples

Check out the [examples directory](./examples) for more detailed usage examples.

## Support

- 📖 **Documentation**: [https://shah.vip/dev](https://shah.vip/dev)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin-core/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
