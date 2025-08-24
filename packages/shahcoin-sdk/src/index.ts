/**
 * Shahcoin dApp SDK - Main Entry Point
 * 
 * Official SDK for integrating with Shahcoin blockchain
 * Built on Shahcoin Core
 */

export { ShahcoinSDK } from './shahcoin-sdk';

// Export all types
export type {
  WalletConfig,
  WalletConnection,
  Transaction,
  TokenInfo,
  NFTInfo,
  ShahSwapQuote,
  ShahSwapTrade,
  FeeEstimate,
  StakingInfo,
  NetworkInfo,
  ApiResponse,
  WalletEvent,
  CreateTokenParams,
  CreateNFTParams,
  SendTransactionParams,
  ShahSwapParams,
  EventCallback,
  ConnectionCallback
} from './types';

// Export version
export const VERSION = '1.0.0';

// Export default instance factory
export function createShahcoinSDK(config?: WalletConfig): ShahcoinSDK {
  return new ShahcoinSDK(config);
}

// Export network configurations
export const NETWORKS = {
  mainnet: {
    chainId: 1,
    name: 'Shahcoin Mainnet',
    rpcUrl: 'https://rpc.shah.vip',
    explorerUrl: 'https://explorer.shah.vip',
    shahSwapUrl: 'https://swap.shah.vip'
  },
  testnet: {
    chainId: 2,
    name: 'Shahcoin Testnet',
    rpcUrl: 'https://testnet-rpc.shah.vip',
    explorerUrl: 'https://testnet-explorer.shah.vip',
    shahSwapUrl: 'https://testnet-swap.shah.vip'
  },
  regtest: {
    chainId: 3,
    name: 'Shahcoin Regtest',
    rpcUrl: 'http://localhost:8332',
    explorerUrl: 'http://localhost:3001',
    shahSwapUrl: 'http://localhost:3002'
  }
} as const;
