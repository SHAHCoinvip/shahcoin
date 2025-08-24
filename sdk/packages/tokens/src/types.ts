/**
 * @fileoverview TypeScript types for SHAHCOIN Tokens SDK
 * @license Apache-2.0
 */

// Token Types
export interface TokenInfo {
  address: string;
  name: string;
  symbol: string;
  decimals: number;
  totalSupply: string;
  creator: string;
  createdAt: Date;
  verified: boolean;
  logo?: string;
  description?: string;
  website?: string;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
  };
}

export interface TokenMetadata {
  name: string;
  symbol: string;
  decimals: number;
  totalSupply: string;
  description?: string;
  logo?: string;
  website?: string;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
  };
}

// Token Creation Types
export interface CreateTokenOptions {
  name: string;
  symbol: string;
  decimals: number;
  totalSupply: string;
  description?: string;
  logo?: string;
  website?: string;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
  };
  initialDistribution?: {
    address: string;
    amount: string;
  }[];
  mintable?: boolean;
  burnable?: boolean;
  pausable?: boolean;
  capped?: boolean;
  maxSupply?: string;
}

export interface CreateTokenResult {
  txid: string;
  tokenAddress: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// Token Transfer Types
export interface TransferOptions {
  from: string;
  to: string;
  amount: string;
  gasLimit?: number;
  gasPrice?: string;
  data?: string;
}

export interface TransferResult {
  txid: string;
  from: string;
  to: string;
  amount: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// Token Balance Types
export interface TokenBalance {
  tokenAddress: string;
  address: string;
  balance: string;
  symbol: string;
  decimals: number;
  value: number; // In SHAH equivalent
  lastUpdated: Date;
}

// Token Transaction Types
export interface TokenTransaction {
  txid: string;
  tokenAddress: string;
  from: string;
  to: string;
  amount: string;
  blockNumber: number;
  blockHash: string;
  timestamp: Date;
  gasUsed: number;
  gasPrice: string;
  status: 'pending' | 'confirmed' | 'failed';
  confirmations: number;
}

// Token Approval Types
export interface ApprovalOptions {
  spender: string;
  amount: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface ApprovalResult {
  txid: string;
  owner: string;
  spender: string;
  amount: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// Token Allowance Types
export interface TokenAllowance {
  tokenAddress: string;
  owner: string;
  spender: string;
  allowance: string;
  symbol: string;
  decimals: number;
}

// Token Mint/Burn Types
export interface MintOptions {
  to: string;
  amount: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface BurnOptions {
  amount: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface MintBurnResult {
  txid: string;
  amount: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// Token Events
export interface TokenEvent {
  type: 'Transfer' | 'Approval' | 'Mint' | 'Burn' | 'Pause' | 'Unpause';
  tokenAddress: string;
  from?: string;
  to?: string;
  amount?: string;
  spender?: string;
  owner?: string;
  blockNumber: number;
  blockHash: string;
  timestamp: Date;
  txid: string;
  logIndex: number;
}

// Token Registry Types
export interface TokenRegistryEntry {
  address: string;
  name: string;
  symbol: string;
  decimals: number;
  totalSupply: string;
  creator: string;
  createdAt: Date;
  verified: boolean;
  logo?: string;
  description?: string;
  website?: string;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
  };
  metadata?: TokenMetadata;
  transactions: number;
  holders: number;
  marketCap?: number;
  volume24h?: number;
  price?: number;
  priceChange24h?: number;
}

// Token Search/Filter Types
export interface TokenSearchOptions {
  query?: string;
  creator?: string;
  verified?: boolean;
  sortBy?: 'name' | 'symbol' | 'totalSupply' | 'holders' | 'transactions' | 'marketCap' | 'volume24h';
  sortOrder?: 'asc' | 'desc';
  limit?: number;
  offset?: number;
}

export interface TokenSearchResult {
  tokens: TokenRegistryEntry[];
  total: number;
  page: number;
  limit: number;
  hasMore: boolean;
}

// Token Analytics Types
export interface TokenAnalytics {
  address: string;
  totalTransactions: number;
  uniqueHolders: number;
  totalVolume: number;
  averageTransactionSize: number;
  largestTransaction: number;
  smallestTransaction: number;
  topHolders: {
    address: string;
    balance: string;
    percentage: number;
  }[];
  transactionHistory: {
    date: string;
    transactions: number;
    volume: number;
    holders: number;
  }[];
  priceHistory?: {
    date: string;
    price: number;
    marketCap: number;
    volume: number;
  }[];
}

// Error Types
export class TokenError extends Error {
  constructor(
    message: string,
    public code: string,
    public details?: any
  ) {
    super(message);
    this.name = 'TokenError';
  }
}

export class TokenCreationError extends TokenError {
  constructor(message: string, details?: any) {
    super(message, 'TOKEN_CREATION_ERROR', details);
    this.name = 'TokenCreationError';
  }
}

export class TokenTransferError extends TokenError {
  constructor(message: string, details?: any) {
    super(message, 'TOKEN_TRANSFER_ERROR', details);
    this.name = 'TokenTransferError';
  }
}

export class TokenApprovalError extends TokenError {
  constructor(message: string, details?: any) {
    super(message, 'TOKEN_APPROVAL_ERROR', details);
    this.name = 'TokenApprovalError';
  }
}

// Configuration Types
export interface TokenConfig {
  factoryAddress: string;
  registryAddress: string;
  gasLimit: {
    create: number;
    transfer: number;
    approve: number;
    mint: number;
    burn: number;
  };
  gasPrice: string;
  maxDecimals: number;
  minNameLength: number;
  maxNameLength: number;
  minSymbolLength: number;
  maxSymbolLength: number;
  maxTotalSupply: string;
}

// Utility Types
export type TokenAddress = string;
export type TokenAmount = string;
export type TokenSymbol = string;
export type TokenName = string;
