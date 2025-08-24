/**
 * @fileoverview SHAHCOIN RPC Client - Handles all blockchain RPC interactions
 * @license Apache-2.0
 */

import axios, { AxiosInstance, AxiosRequestConfig } from 'axios';
import { EventEmitter } from 'events';
import {
  RPCConfig,
  RPCResponse,
  RPCError,
  ConnectionEvent,
  SDKConfig
} from '../types';

/**
 * SHAHCOIN RPC Client for blockchain interactions
 */
export class SHAHCOINRPCClient extends EventEmitter {
  private config: RPCConfig;
  private client: AxiosInstance;
  private requestId = 0;
  private connected = false;
  private reconnectAttempts = 0;
  private maxReconnectAttempts: number;
  private reconnectInterval: number;
  private autoReconnect: boolean;

  constructor(config: RPCConfig, sdkConfig?: Partial<SDKConfig>) {
    super();
    this.config = config;
    this.maxReconnectAttempts = sdkConfig?.maxReconnectAttempts || 5;
    this.reconnectInterval = sdkConfig?.reconnectInterval || 5000;
    this.autoReconnect = sdkConfig?.autoReconnect ?? true;

    // Create axios instance with authentication
    const auth = config.username && config.password 
      ? { username: config.username, password: config.password }
      : {};

    this.client = axios.create({
      baseURL: `${config.ssl ? 'https' : 'http'}://${config.host}:${config.port}`,
      timeout: config.timeout || 30000,
      auth,
      headers: {
        'Content-Type': 'application/json',
      },
    });

    // Add request/response interceptors
    this.setupInterceptors();
  }

  /**
   * Setup axios interceptors for error handling and logging
   */
  private setupInterceptors(): void {
    // Request interceptor
    this.client.interceptors.request.use(
      (config) => {
        this.emit('request', config);
        return config;
      },
      (error) => {
        this.emit('error', error);
        return Promise.reject(error);
      }
    );

    // Response interceptor
    this.client.interceptors.response.use(
      (response) => {
        this.connected = true;
        this.reconnectAttempts = 0;
        this.emit('connected', { type: 'connected' } as ConnectionEvent);
        return response;
      },
      (error) => {
        this.connected = false;
        this.emit('disconnected', { type: 'disconnected' } as ConnectionEvent);
        
        if (this.autoReconnect && this.reconnectAttempts < this.maxReconnectAttempts) {
          this.scheduleReconnect();
        }
        
        this.emit('error', { type: 'error', data: error } as ConnectionEvent);
        return Promise.reject(error);
      }
    );
  }

  /**
   * Schedule reconnection attempt
   */
  private scheduleReconnect(): void {
    this.reconnectAttempts++;
    setTimeout(() => {
      this.emit('reconnecting', { attempt: this.reconnectAttempts });
      // Attempt to reconnect by making a simple request
      this.getBlockchainInfo().catch(() => {
        // If reconnection fails, schedule another attempt
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
          this.scheduleReconnect();
        }
      });
    }, this.reconnectInterval);
  }

  /**
   * Make RPC call to the blockchain node
   */
  async call<T = any>(method: string, params: any[] = []): Promise<T> {
    const id = ++this.requestId;
    const payload = {
      jsonrpc: '2.0',
      id,
      method,
      params,
    };

    try {
      const response = await this.client.post<RPCResponse<T>>('', payload);
      const { result, error } = response.data;

      if (error) {
        throw new RPCError(
          `RPC Error: ${error.message}`,
          error.code,
          error.message
        );
      }

      return result;
    } catch (error) {
      if (error instanceof RPCError) {
        throw error;
      }
      
      if (axios.isAxiosError(error)) {
        throw new RPCError(
          `Network Error: ${error.message}`,
          error.response?.status || -1,
          error.message
        );
      }
      
      throw new RPCError(
        `Unknown Error: ${error instanceof Error ? error.message : String(error)}`,
        -1,
        String(error)
      );
    }
  }

  /**
   * Get blockchain information
   */
  async getBlockchainInfo(): Promise<any> {
    return this.call('getblockchaininfo');
  }

  /**
   * Get network information
   */
  async getNetworkInfo(): Promise<any> {
    return this.call('getnetworkinfo');
  }

  /**
   * Get mining information
   */
  async getMiningInfo(): Promise<any> {
    return this.call('getmininginfo');
  }

  /**
   * Get wallet information
   */
  async getWalletInfo(): Promise<any> {
    return this.call('getwalletinfo');
  }

  /**
   * Get staking information
   */
  async getStakingInfo(): Promise<any> {
    return this.call('getstakinginfo');
  }

  /**
   * Get algorithm information for hybrid consensus
   */
  async getAlgoInfo(): Promise<any> {
    return this.call('getalgoinfo');
  }

  /**
   * Get block by hash
   */
  async getBlock(hash: string, verbosity: number = 1): Promise<any> {
    return this.call('getblock', [hash, verbosity]);
  }

  /**
   * Get block by height
   */
  async getBlockByHeight(height: number, verbosity: number = 1): Promise<any> {
    return this.call('getblock', [height, verbosity]);
  }

  /**
   * Get transaction by hash
   */
  async getTransaction(txid: string, includeWatchonly: boolean = false): Promise<any> {
    return this.call('gettransaction', [txid, includeWatchonly]);
  }

  /**
   * Get raw transaction
   */
  async getRawTransaction(txid: string, verbose: boolean = true): Promise<any> {
    return this.call('getrawtransaction', [txid, verbose]);
  }

  /**
   * Get UTXOs for an address
   */
  async getUTXOs(address: string): Promise<any[]> {
    return this.call('getutxos', [address]);
  }

  /**
   * Get balance for an address
   */
  async getBalance(address?: string, minconf: number = 1): Promise<number> {
    const params = address ? [address, minconf] : [minconf];
    return this.call('getbalance', params);
  }

  /**
   * Send raw transaction
   */
  async sendRawTransaction(hex: string, allowhighfees: boolean = false): Promise<string> {
    return this.call('sendrawtransaction', [hex, allowhighfees]);
  }

  /**
   * Create raw transaction
   */
  async createRawTransaction(
    inputs: Array<{ txid: string; vout: number }>,
    outputs: Record<string, number>
  ): Promise<string> {
    return this.call('createrawtransaction', [inputs, outputs]);
  }

  /**
   * Sign raw transaction
   */
  async signRawTransaction(
    hex: string,
    inputs?: any[],
    privateKeys?: string[],
    sighashType: string = 'ALL'
  ): Promise<any> {
    const params = [hex];
    if (inputs) params.push(inputs);
    if (privateKeys) params.push(privateKeys);
    params.push(sighashType);
    
    return this.call('signrawtransaction', params);
  }

  /**
   * Get new address
   */
  async getNewAddress(label: string = '', addressType: string = 'legacy'): Promise<string> {
    return this.call('getnewaddress', [label, addressType]);
  }

  /**
   * Validate address
   */
  async validateAddress(address: string): Promise<any> {
    return this.call('validateaddress', [address]);
  }

  /**
   * Get connection status
   */
  isConnected(): boolean {
    return this.connected;
  }

  /**
   * Get current configuration
   */
  getConfig(): RPCConfig {
    return { ...this.config };
  }

  /**
   * Update configuration
   */
  updateConfig(config: Partial<RPCConfig>): void {
    this.config = { ...this.config, ...config };
    
    // Update axios instance if host/port/auth changed
    if (config.host || config.port || config.username || config.password) {
      const auth = this.config.username && this.config.password 
        ? { username: this.config.username, password: this.config.password }
        : {};

      this.client.defaults.baseURL = `${this.config.ssl ? 'https' : 'http'}://${this.config.host}:${this.config.port}`;
      this.client.defaults.auth = auth;
    }
  }

  /**
   * Disconnect and cleanup
   */
  disconnect(): void {
    this.connected = false;
    this.autoReconnect = false;
    this.removeAllListeners();
  }
}
