/**
 * @fileoverview TypeScript types for SHAHCOIN NFTs SDK
 * @license Apache-2.0
 */

// NFT Types
export interface NFTInfo {
  tokenId: string;
  contractAddress: string;
  owner: string;
  creator: string;
  name: string;
  symbol: string;
  description?: string;
  image?: string;
  externalUrl?: string;
  attributes?: NFTAttribute[];
  metadata?: NFTMetadata;
  mintedAt: Date;
  lastTransferredAt?: Date;
  verified: boolean;
  royaltyPercentage?: number;
  royaltyRecipient?: string;
}

export interface NFTMetadata {
  name: string;
  description?: string;
  image?: string;
  externalUrl?: string;
  attributes?: NFTAttribute[];
  animationUrl?: string;
  background_color?: string;
  youtube_url?: string;
  [key: string]: any;
}

export interface NFTAttribute {
  trait_type: string;
  value: string | number;
  display_type?: 'number' | 'boost_number' | 'boost_percentage' | 'date';
  max_value?: number;
}

// NFT Collection Types
export interface NFTCollection {
  address: string;
  name: string;
  symbol: string;
  description?: string;
  image?: string;
  bannerImage?: string;
  externalUrl?: string;
  creator: string;
  createdAt: Date;
  verified: boolean;
  totalSupply: number;
  maxSupply?: number;
  mintPrice?: string;
  royaltyPercentage?: number;
  royaltyRecipient?: string;
  metadata?: NFTMetadata;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
    website?: string;
  };
  stats: {
    totalSales: number;
    totalVolume: number;
    floorPrice?: number;
    averagePrice?: number;
    uniqueOwners: number;
    totalListed: number;
  };
}

// NFT Creation Types
export interface CreateNFTOptions {
  name: string;
  symbol: string;
  description?: string;
  image?: string;
  externalUrl?: string;
  attributes?: NFTAttribute[];
  maxSupply?: number;
  mintPrice?: string;
  royaltyPercentage?: number;
  royaltyRecipient?: string;
  metadata?: NFTMetadata;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
    website?: string;
  };
}

export interface CreateNFTResult {
  txid: string;
  contractAddress: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// NFT Minting Types
export interface MintNFTOptions {
  to: string;
  tokenId?: string; // Auto-generated if not provided
  metadata?: NFTMetadata;
  gasLimit?: number;
  gasPrice?: string;
}

export interface MintNFTResult {
  txid: string;
  tokenId: string;
  contractAddress: string;
  owner: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// NFT Transfer Types
export interface TransferNFTOptions {
  from: string;
  to: string;
  tokenId: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface TransferNFTResult {
  txid: string;
  from: string;
  to: string;
  tokenId: string;
  contractAddress: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// NFT Approval Types
export interface ApproveNFTOptions {
  to: string;
  tokenId: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface ApproveAllNFTOptions {
  to: string;
  approved: boolean;
  gasLimit?: number;
  gasPrice?: string;
}

export interface ApprovalResult {
  txid: string;
  owner: string;
  approved: string;
  tokenId?: string;
  isApproved: boolean;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// NFT Burn Types
export interface BurnNFTOptions {
  tokenId: string;
  gasLimit?: number;
  gasPrice?: string;
}

export interface BurnNFTResult {
  txid: string;
  tokenId: string;
  contractAddress: string;
  gasUsed: number;
  gasPrice: string;
  totalCost: string;
}

// NFT Balance Types
export interface NFTBalance {
  contractAddress: string;
  tokenId: string;
  owner: string;
  name: string;
  symbol: string;
  image?: string;
  metadata?: NFTMetadata;
  lastTransferredAt?: Date;
}

// NFT Transaction Types
export interface NFTTransaction {
  txid: string;
  contractAddress: string;
  tokenId: string;
  from: string;
  to: string;
  blockNumber: number;
  blockHash: string;
  timestamp: Date;
  gasUsed: number;
  gasPrice: string;
  status: 'pending' | 'confirmed' | 'failed';
  confirmations: number;
  eventType: 'Transfer' | 'Approval' | 'ApprovalForAll' | 'Mint' | 'Burn';
}

// NFT Events
export interface NFTEvent {
  type: 'Transfer' | 'Approval' | 'ApprovalForAll' | 'Mint' | 'Burn';
  contractAddress: string;
  tokenId?: string;
  from?: string;
  to?: string;
  approved?: string;
  owner?: string;
  operator?: string;
  blockNumber: number;
  blockHash: string;
  timestamp: Date;
  txid: string;
  logIndex: number;
}

// NFT Registry Types
export interface NFTRegistryEntry {
  contractAddress: string;
  name: string;
  symbol: string;
  description?: string;
  image?: string;
  bannerImage?: string;
  externalUrl?: string;
  creator: string;
  createdAt: Date;
  verified: boolean;
  totalSupply: number;
  maxSupply?: number;
  mintPrice?: string;
  royaltyPercentage?: number;
  royaltyRecipient?: string;
  metadata?: NFTMetadata;
  social?: {
    twitter?: string;
    telegram?: string;
    discord?: string;
    website?: string;
  };
  stats: {
    totalSales: number;
    totalVolume: number;
    floorPrice?: number;
    averagePrice?: number;
    uniqueOwners: number;
    totalListed: number;
  };
  transactions: number;
  holders: number;
}

// NFT Search/Filter Types
export interface NFTSearchOptions {
  query?: string;
  creator?: string;
  verified?: boolean;
  collection?: string;
  owner?: string;
  sortBy?: 'name' | 'symbol' | 'totalSupply' | 'holders' | 'transactions' | 'floorPrice' | 'volume';
  sortOrder?: 'asc' | 'desc';
  limit?: number;
  offset?: number;
}

export interface NFTSearchResult {
  nfts: NFTRegistryEntry[];
  total: number;
  page: number;
  limit: number;
  hasMore: boolean;
}

// NFT Analytics Types
export interface NFTAnalytics {
  contractAddress: string;
  totalTransactions: number;
  uniqueOwners: number;
  totalVolume: number;
  averagePrice: number;
  floorPrice?: number;
  highestSale: number;
  lowestSale: number;
  topOwners: {
    address: string;
    tokenCount: number;
    percentage: number;
  }[];
  transactionHistory: {
    date: string;
    transactions: number;
    volume: number;
    owners: number;
  }[];
  priceHistory?: {
    date: string;
    floorPrice: number;
    averagePrice: number;
    volume: number;
  }[];
}

// IPFS Types
export interface IPFSConfig {
  gateway: string;
  apiEndpoint?: string;
  apiKey?: string;
  timeout?: number;
}

export interface IPFSResult {
  hash: string;
  size: number;
  url: string;
}

// Metadata Storage Types
export interface MetadataStorage {
  type: 'ipfs' | 'arweave' | 'centralized';
  url: string;
  hash?: string;
  timestamp: Date;
}

// Error Types
export class NFTError extends Error {
  constructor(
    message: string,
    public code: string,
    public details?: any
  ) {
    super(message);
    this.name = 'NFTError';
  }
}

export class NFTCreationError extends NFTError {
  constructor(message: string, details?: any) {
    super(message, 'NFT_CREATION_ERROR', details);
    this.name = 'NFTCreationError';
  }
}

export class NFTMintError extends NFTError {
  constructor(message: string, details?: any) {
    super(message, 'NFT_MINT_ERROR', details);
    this.name = 'NFTMintError';
  }
}

export class NFTTransferError extends NFTError {
  constructor(message: string, details?: any) {
    super(message, 'NFT_TRANSFER_ERROR', details);
    this.name = 'NFTTransferError';
  }
}

export class MetadataError extends NFTError {
  constructor(message: string, details?: any) {
    super(message, 'METADATA_ERROR', details);
    this.name = 'MetadataError';
  }
}

// Configuration Types
export interface NFTConfig {
  factoryAddress: string;
  registryAddress: string;
  gasLimit: {
    create: number;
    mint: number;
    transfer: number;
    approve: number;
    burn: number;
  };
  gasPrice: string;
  maxSupply: number;
  maxNameLength: number;
  maxSymbolLength: number;
  maxDescriptionLength: number;
  ipfs: IPFSConfig;
}

// Utility Types
export type NFTAddress = string;
export type TokenId = string;
export type NFTName = string;
export type NFTSymbol = string;
