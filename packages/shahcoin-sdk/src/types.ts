/**
 * Core types and interfaces for Shahcoin dApp SDK
 */

export interface WalletConfig {
  rpcUrl?: string;
  apiKey?: string;
  network?: 'mainnet' | 'testnet' | 'regtest';
  timeout?: number;
}

export interface WalletConnection {
  isConnected: boolean;
  address?: string;
  balance?: number;
  network?: string;
}

export interface Transaction {
  txid: string;
  amount: number;
  fee: number;
  confirmations: number;
  blocktime: number;
  address: string;
  category: 'send' | 'receive' | 'stake' | 'mint';
  confirmations: number;
}

export interface TokenInfo {
  symbol: string;
  name: string;
  decimals: number;
  totalSupply: number;
  contractAddress: string;
  creator: string;
  creationTime: number;
}

export interface NFTInfo {
  tokenId: string;
  name: string;
  description?: string;
  imageUrl?: string;
  metadata?: Record<string, any>;
  owner: string;
  contractAddress: string;
  creationTime: number;
}

export interface ShahSwapQuote {
  inputToken: string;
  outputToken: string;
  inputAmount: number;
  outputAmount: number;
  priceImpact: number;
  fee: number;
  route: string[];
  validUntil: number;
}

export interface ShahSwapTrade {
  quoteId: string;
  inputToken: string;
  outputToken: string;
  inputAmount: number;
  outputAmount: number;
  txid?: string;
  status: 'pending' | 'confirmed' | 'failed';
  timestamp: number;
}

export interface FeeEstimate {
  low: number;
  medium: number;
  high: number;
  custom?: number;
}

export interface StakingInfo {
  isStaking: boolean;
  stakedAmount: number;
  rewardsEarned: number;
  lastRewardTime?: number;
  nextRewardEstimate?: number;
  stakingAddress?: string;
}

export interface NetworkInfo {
  chainId: number;
  name: string;
  rpcUrl: string;
  explorerUrl: string;
  shahSwapUrl: string;
}

export interface ApiResponse<T = any> {
  success: boolean;
  data?: T;
  error?: string;
  message?: string;
}

export interface WalletEvent {
  type: 'connected' | 'disconnected' | 'balanceChanged' | 'transactionReceived' | 'stakingReward';
  data: any;
  timestamp: number;
}

export interface CreateTokenParams {
  name: string;
  symbol: string;
  decimals: number;
  totalSupply: number;
  description?: string;
  website?: string;
}

export interface CreateNFTParams {
  name: string;
  description?: string;
  imageUrl?: string;
  metadata?: Record<string, any>;
  attributes?: Array<{
    trait_type: string;
    value: string | number;
  }>;
}

export interface SendTransactionParams {
  to: string;
  amount: number;
  fee?: number;
  memo?: string;
}

export interface ShahSwapParams {
  inputToken: string;
  outputToken: string;
  amount: number;
  slippageTolerance?: number;
  deadline?: number;
}

export type EventCallback = (event: WalletEvent) => void;
export type ConnectionCallback = (connection: WalletConnection) => void;
