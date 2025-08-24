/**
 * @fileoverview TypeScript types for SHAHCOIN Core SDK
 * @license Apache-2.0
 */

// RPC Client Configuration
export interface RPCConfig {
  host: string;
  port: number;
  username?: string;
  password?: string;
  timeout?: number;
  ssl?: boolean;
}

// Blockchain Information
export interface BlockchainInfo {
  chain: string;
  blocks: number;
  headers: number;
  bestblockhash: string;
  difficulty: number;
  mediantime: number;
  verificationprogress: number;
  chainwork: string;
  pruned: boolean;
  pruneheight?: number;
  automatic_pruning?: boolean;
  prune_target_size?: number;
  softforks: Record<string, any>;
  bip9_softforks: Record<string, any>;
}

// UTXO Information
export interface UTXO {
  txid: string;
  vout: number;
  address: string;
  scriptPubKey: string;
  amount: number;
  confirmations: number;
  spendable: boolean;
  solvable: boolean;
  safe: boolean;
}

// Transaction Information
export interface Transaction {
  txid: string;
  hash: string;
  version: number;
  size: number;
  vsize: number;
  weight: number;
  locktime: number;
  vin: TransactionInput[];
  vout: TransactionOutput[];
  blockhash?: string;
  confirmations?: number;
  blocktime?: number;
  time?: number;
}

export interface TransactionInput {
  txid: string;
  vout: number;
  scriptSig: {
    asm: string;
    hex: string;
  };
  sequence: number;
  txinwitness?: string[];
}

export interface TransactionOutput {
  value: number;
  n: number;
  scriptPubKey: {
    asm: string;
    hex: string;
    reqSigs?: number;
    type: string;
    addresses?: string[];
  };
}

// Hybrid Consensus Types
export enum AlgoType {
  SHA256D = 0,
  SCRYPT = 1,
  GROESTL = 2,
  POS = 3
}

export interface ConsensusInfo {
  currentAlgo: AlgoType;
  currentHeight: number;
  nextAlgo: AlgoType;
  sha256dDifficulty: number;
  scryptDifficulty: number;
  groestlDifficulty: number;
  posTarget: number;
}

// Block Information
export interface Block {
  hash: string;
  confirmations: number;
  size: number;
  weight: number;
  height: number;
  version: number;
  versionHex: string;
  merkleroot: string;
  tx: string[];
  time: number;
  mediantime: number;
  nonce: number;
  bits: string;
  difficulty: number;
  chainwork: string;
  nTx: number;
  previousblockhash?: string;
  nextblockhash?: string;
  algoType?: AlgoType;
  isProofOfStake?: boolean;
}

// Network Information
export interface NetworkInfo {
  version: number;
  subversion: string;
  protocolversion: number;
  localservices: string;
  localservicesnames: string[];
  localrelay: boolean;
  timeoffset: number;
  networkactive: boolean;
  connections: number;
  connections_in: number;
  connections_out: number;
  networks: Network[];
  relayfee: number;
  incrementalfee: number;
  localaddresses: LocalAddress[];
  warnings: string;
}

export interface Network {
  name: string;
  limited: boolean;
  reachable: boolean;
  proxy: string;
  proxy_randomize_credentials: boolean;
}

export interface LocalAddress {
  address: string;
  port: number;
  score: number;
}

// Mining Information
export interface MiningInfo {
  blocks: number;
  currentblockweight: number;
  currentblocktx: number;
  difficulty: number;
  networkhashps: number;
  pooledtx: number;
  chain: string;
  warnings: string;
}

// Wallet Information
export interface WalletInfo {
  walletname: string;
  walletversion: number;
  balance: number;
  unconfirmed_balance: number;
  immature_balance: number;
  txcount: number;
  keypoololdest: number;
  keypoolsize: number;
  keypoolsize_hd_internal: number;
  unlocked_until: number;
  paytxfee: number;
  hdseedid?: string;
  private_keys_enabled: boolean;
  avoid_reuse: boolean;
  scanning?: {
    duration: number;
    progress: number;
  };
}

// Error Types
export class SHAHCOINError extends Error {
  constructor(
    message: string,
    public code?: string,
    public details?: any
  ) {
    super(message);
    this.name = 'SHAHCOINError';
  }
}

export class RPCError extends SHAHCOINError {
  constructor(
    message: string,
    public rpcCode: number,
    public rpcMessage: string
  ) {
    super(message, 'RPC_ERROR', { rpcCode, rpcMessage });
    this.name = 'RPCError';
  }
}

// Event Types
export interface BlockchainEvent {
  type: 'block' | 'transaction' | 'mempool';
  data: Block | Transaction | string;
}

export interface ConnectionEvent {
  type: 'connected' | 'disconnected' | 'error';
  data?: any;
}

// Configuration Types
export interface SDKConfig {
  rpc: RPCConfig;
  network: 'mainnet' | 'testnet' | 'regtest';
  autoReconnect?: boolean;
  reconnectInterval?: number;
  maxReconnectAttempts?: number;
}

// Response Types
export interface RPCResponse<T = any> {
  result: T;
  error: null | {
    code: number;
    message: string;
  };
  id: string | number;
}

// Utility Types
export type Address = string;
export type TransactionHash = string;
export type BlockHash = string;
export type ScriptPubKey = string;
export type PrivateKey = string;
export type PublicKey = string;

// Staking Types
export interface StakeInfo {
  enabled: boolean;
  staking: boolean;
  errors: string;
  currentblocksize: number;
  currentblocktx: number;
  pooledtx: number;
  difficulty: number;
  searchInterval: number;
  weight: number;
  netstakeweight: number;
  expectedtime: number;
}

export interface StakeInput {
  txid: string;
  vout: number;
  amount: number;
  confirmations: number;
  age: number;
  weight: number;
  address: string;
}

// Smart Unit Display Types
export interface UnitConversion {
  value: number;
  unit: string;
  display: string;
}

export interface UnitBreakdown {
  units: UnitConversion[];
  total: number;
  formatted: string;
}
