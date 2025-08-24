/**
 * @fileoverview SHAHCOIN Core SDK - Blockchain RPC interactions, UTXO management, and hybrid consensus
 * @license Apache-2.0
 * @author SHAHCOIN Team
 */

export * from './rpc/client';
export * from './rpc/methods';
export * from './utxo/manager';
export * from './consensus/hybrid';
export * from './types';
export * from './utils';

// Main SDK class for easy initialization
export { SHAHCOINCore } from './core';
