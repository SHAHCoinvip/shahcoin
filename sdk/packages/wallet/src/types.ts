/**
 * @fileoverview TypeScript types for SHAHCOIN Wallet SDK
 * @license Apache-2.0
 */

// Wallet Types
export interface WalletInfo {
  id: string;
  name: string;
  type: 'hd' | 'imported' | 'watch-only';
  createdAt: Date;
  updatedAt: Date;
  isEncrypted: boolean;
  isLocked: boolean;
  balance: number;
  unconfirmedBalance: number;
  addressCount: number;
  lastUsedAddress?: string;
}

// HD Wallet Types
export interface HDWalletInfo extends WalletInfo {
  type: 'hd';
  mnemonic?: string; // Only available if not encrypted
  passphrase?: string;
  derivationPath: string;
  accountIndex: number;
  addressIndex: number;
  xpub: string;
  xpriv?: string; // Only available if not encrypted
}

// Imported Wallet Types
export interface ImportedWalletInfo extends WalletInfo {
  type: 'imported';
  addresses: string[];
  privateKeys: Record<string, string>; // address -> privateKey
}

// Watch-Only Wallet Types
export interface WatchOnlyWalletInfo extends WalletInfo {
  type: 'watch-only';
  addresses: string[];
}

// Address Types
export interface AddressInfo {
  address: string;
  index: number;
  derivationPath: string;
  publicKey: string;
  privateKey?: string; // Only available if wallet is unlocked
  balance: number;
  unconfirmedBalance: number;
  txCount: number;
  lastUsed?: Date;
}

// Transaction Types
export interface WalletTransaction {
  txid: string;
  hash: string;
  version: number;
  size: number;
  vsize: number;
  weight: number;
  locktime: number;
  vin: WalletTransactionInput[];
  vout: WalletTransactionOutput[];
  blockhash?: string;
  confirmations?: number;
  blocktime?: number;
  time?: number;
  fee?: number;
  category: 'send' | 'receive' | 'generate' | 'immature' | 'orphan';
  amount: number;
  address?: string;
  label?: string;
}

export interface WalletTransactionInput {
  txid: string;
  vout: number;
  scriptSig: {
    asm: string;
    hex: string;
  };
  sequence: number;
  txinwitness?: string[];
  address?: string;
  amount?: number;
}

export interface WalletTransactionOutput {
  value: number;
  n: number;
  scriptPubKey: {
    asm: string;
    hex: string;
    reqSigs?: number;
    type: string;
    addresses?: string[];
  };
  address?: string;
  isChange?: boolean;
}

// Transaction Creation Types
export interface TransactionInput {
  txid: string;
  vout: number;
  address: string;
  amount: number;
  scriptPubKey: string;
}

export interface TransactionOutput {
  address: string;
  amount: number;
  isChange?: boolean;
}

export interface TransactionOptions {
  feeRate?: number; // satoshis per byte
  changeAddress?: string;
  subtractFeeFromOutputs?: number[];
  replaceable?: boolean;
  locktime?: number;
}

export interface TransactionResult {
  txid: string;
  hex: string;
  fee: number;
  size: number;
  vsize: number;
  weight: number;
}

// Encryption Types
export interface EncryptionOptions {
  password: string;
  iterations?: number;
  salt?: string;
}

export interface EncryptedData {
  encrypted: string;
  salt: string;
  iterations: number;
  iv: string;
}

// Backup Types
export interface WalletBackup {
  version: string;
  timestamp: Date;
  wallet: WalletInfo;
  addresses: AddressInfo[];
  transactions: WalletTransaction[];
  encryptedData?: EncryptedData;
}

// Network Types
export interface NetworkConfig {
  name: 'mainnet' | 'testnet' | 'regtest';
  bech32: string;
  pubKeyHash: number;
  scriptHash: number;
  wif: number;
  bip32: {
    public: number;
    private: number;
  };
  messagePrefix: string;
  defaultPort: number;
  seeds: string[];
}

// Error Types
export class WalletError extends Error {
  constructor(
    message: string,
    public code: string,
    public details?: any
  ) {
    super(message);
    this.name = 'WalletError';
  }
}

export class EncryptionError extends WalletError {
  constructor(message: string, details?: any) {
    super(message, 'ENCRYPTION_ERROR', details);
    this.name = 'EncryptionError';
  }
}

export class TransactionError extends WalletError {
  constructor(message: string, details?: any) {
    super(message, 'TRANSACTION_ERROR', details);
    this.name = 'TransactionError';
  }
}

export class AddressError extends WalletError {
  constructor(message: string, details?: any) {
    super(message, 'ADDRESS_ERROR', details);
    this.name = 'AddressError';
  }
}

// Event Types
export interface WalletEvent {
  type: 'balance-changed' | 'transaction-received' | 'transaction-sent' | 'address-generated' | 'wallet-locked' | 'wallet-unlocked';
  data: any;
}

// Configuration Types
export interface WalletConfig {
  network: NetworkConfig;
  defaultDerivationPath: string;
  addressGapLimit: number;
  maxAddresses: number;
  autoLockTimeout: number; // minutes
  encryptionIterations: number;
}

// Utility Types
export type PrivateKey = string;
export type PublicKey = string;
export type Address = string;
export type TransactionHash = string;
export type Mnemonic = string;
export type Passphrase = string;
export type Password = string;
