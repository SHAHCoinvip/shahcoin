import axios, { AxiosInstance } from 'axios';
import WebSocket from 'ws';
import {
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

/**
 * Main Shahcoin dApp SDK class
 * Provides wallet connectivity, ShahSwap integration, and blockchain interactions
 */
export class ShahcoinSDK {
  private config: WalletConfig;
  private httpClient: AxiosInstance;
  private wsClient?: WebSocket;
  private connection: WalletConnection = { isConnected: false };
  private eventCallbacks: EventCallback[] = [];
  private connectionCallbacks: ConnectionCallback[] = [];
  private reconnectTimer?: NodeJS.Timeout;
  private heartbeatTimer?: NodeJS.Timeout;

  // Network configurations
  private static readonly NETWORKS: Record<string, NetworkInfo> = {
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
  };

  constructor(config: WalletConfig = {}) {
    this.config = {
      network: 'mainnet',
      timeout: 30000,
      ...config
    };

    const network = ShahcoinSDK.NETWORKS[this.config.network || 'mainnet'];
    
    this.httpClient = axios.create({
      baseURL: this.config.rpcUrl || network.rpcUrl,
      timeout: this.config.timeout,
      headers: {
        'Content-Type': 'application/json',
        ...(this.config.apiKey && { 'Authorization': `Bearer ${this.config.apiKey}` })
      }
    });

    this.setupInterceptors();
  }

  /**
   * Connect to the Shahcoin wallet
   */
  async connect(): Promise<WalletConnection> {
    try {
      // Simulate wallet connection (in real implementation, this would connect to Qt wallet or bridge)
      const response = await this.httpClient.post('/wallet/connect', {
        network: this.config.network
      });

      if (response.data.success) {
        this.connection = {
          isConnected: true,
          address: response.data.data.address,
          balance: response.data.data.balance,
          network: this.config.network
        };

        this.setupWebSocket();
        this.notifyConnectionCallbacks();
        this.emitEvent('connected', this.connection);
      }

      return this.connection;
    } catch (error) {
      throw new Error(`Failed to connect to wallet: ${error}`);
    }
  }

  /**
   * Disconnect from the wallet
   */
  async disconnect(): Promise<void> {
    this.connection = { isConnected: false };
    
    if (this.wsClient) {
      this.wsClient.close();
      this.wsClient = undefined;
    }

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = undefined;
    }

    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = undefined;
    }

    this.notifyConnectionCallbacks();
    this.emitEvent('disconnected', {});
  }

  /**
   * Get current wallet connection status
   */
  getConnection(): WalletConnection {
    return { ...this.connection };
  }

  /**
   * Get wallet balance
   */
  async getBalance(): Promise<number> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.get(`/wallet/balance/${this.connection.address}`);
      return response.data.data.balance;
    } catch (error) {
      throw new Error(`Failed to get balance: ${error}`);
    }
  }

  /**
   * Get transaction history
   */
  async getTransactionHistory(limit: number = 50, offset: number = 0): Promise<Transaction[]> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.get(`/wallet/transactions/${this.connection.address}`, {
        params: { limit, offset }
      });
      return response.data.data.transactions;
    } catch (error) {
      throw new Error(`Failed to get transaction history: ${error}`);
    }
  }

  /**
   * Send SHAH transaction
   */
  async sendTransaction(params: SendTransactionParams): Promise<string> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.post('/wallet/send', {
        from: this.connection.address,
        to: params.to,
        amount: params.amount,
        fee: params.fee,
        memo: params.memo
      });

      return response.data.data.txid;
    } catch (error) {
      throw new Error(`Failed to send transaction: ${error}`);
    }
  }

  /**
   * Get fee estimates
   */
  async getFeeEstimates(): Promise<FeeEstimate> {
    try {
      const response = await this.httpClient.get('/wallet/fee-estimates');
      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to get fee estimates: ${error}`);
    }
  }

  /**
   * Get staking information
   */
  async getStakingInfo(): Promise<StakingInfo> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.get(`/staking/info/${this.connection.address}`);
      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to get staking info: ${error}`);
    }
  }

  /**
   * Stake SHAH
   */
  async stake(amount: number): Promise<string> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.post('/staking/stake', {
        address: this.connection.address,
        amount
      });

      return response.data.data.txid;
    } catch (error) {
      throw new Error(`Failed to stake: ${error}`);
    }
  }

  /**
   * Get ShahSwap quote
   */
  async getShahSwapQuote(params: ShahSwapParams): Promise<ShahSwapQuote> {
    try {
      const response = await this.httpClient.post('/shahswap/quote', params);
      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to get ShahSwap quote: ${error}`);
    }
  }

  /**
   * Execute ShahSwap trade
   */
  async executeShahSwap(params: ShahSwapParams): Promise<ShahSwapTrade> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.post('/shahswap/trade', {
        ...params,
        from: this.connection.address
      });

      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to execute ShahSwap trade: ${error}`);
    }
  }

  /**
   * Create new token
   */
  async createToken(params: CreateTokenParams): Promise<TokenInfo> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.post('/tokens/create', {
        ...params,
        creator: this.connection.address
      });

      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to create token: ${error}`);
    }
  }

  /**
   * Create new NFT
   */
  async createNFT(params: CreateNFTParams): Promise<NFTInfo> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.post('/nfts/create', {
        ...params,
        owner: this.connection.address
      });

      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to create NFT: ${error}`);
    }
  }

  /**
   * Get token information
   */
  async getTokenInfo(contractAddress: string): Promise<TokenInfo> {
    try {
      const response = await this.httpClient.get(`/tokens/${contractAddress}`);
      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to get token info: ${error}`);
    }
  }

  /**
   * Get NFT information
   */
  async getNFTInfo(contractAddress: string, tokenId: string): Promise<NFTInfo> {
    try {
      const response = await this.httpClient.get(`/nfts/${contractAddress}/${tokenId}`);
      return response.data.data;
    } catch (error) {
      throw new Error(`Failed to get NFT info: ${error}`);
    }
  }

  /**
   * Get user's tokens
   */
  async getUserTokens(): Promise<TokenInfo[]> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.get(`/tokens/user/${this.connection.address}`);
      return response.data.data.tokens;
    } catch (error) {
      throw new Error(`Failed to get user tokens: ${error}`);
    }
  }

  /**
   * Get user's NFTs
   */
  async getUserNFTs(): Promise<NFTInfo[]> {
    if (!this.connection.isConnected) {
      throw new Error('Wallet not connected');
    }

    try {
      const response = await this.httpClient.get(`/nfts/user/${this.connection.address}`);
      return response.data.data.nfts;
    } catch (error) {
      throw new Error(`Failed to get user NFTs: ${error}`);
    }
  }

  /**
   * Subscribe to wallet events
   */
  on(event: string, callback: EventCallback): void {
    this.eventCallbacks.push(callback);
  }

  /**
   * Subscribe to connection changes
   */
  onConnectionChange(callback: ConnectionCallback): void {
    this.connectionCallbacks.push(callback);
  }

  /**
   * Get network information
   */
  getNetworkInfo(): NetworkInfo {
    return ShahcoinSDK.NETWORKS[this.config.network || 'mainnet'];
  }

  // Private methods

  private setupInterceptors(): void {
    this.httpClient.interceptors.response.use(
      (response) => response,
      (error) => {
        if (error.response?.status === 401) {
          this.emitEvent('disconnected', { reason: 'unauthorized' });
        }
        return Promise.reject(error);
      }
    );
  }

  private setupWebSocket(): void {
    const network = ShahcoinSDK.NETWORKS[this.config.network || 'mainnet'];
    const wsUrl = network.rpcUrl.replace('https://', 'wss://').replace('http://', 'ws://') + '/ws';

    this.wsClient = new WebSocket(wsUrl);

    this.wsClient.on('open', () => {
      console.log('WebSocket connected');
      this.setupHeartbeat();
    });

    this.wsClient.on('message', (data) => {
      try {
        const event = JSON.parse(data.toString());
        this.handleWebSocketEvent(event);
      } catch (error) {
        console.error('Failed to parse WebSocket message:', error);
      }
    });

    this.wsClient.on('close', () => {
      console.log('WebSocket disconnected');
      this.scheduleReconnect();
    });

    this.wsClient.on('error', (error) => {
      console.error('WebSocket error:', error);
    });
  }

  private setupHeartbeat(): void {
    this.heartbeatTimer = setInterval(() => {
      if (this.wsClient?.readyState === WebSocket.OPEN) {
        this.wsClient.send(JSON.stringify({ type: 'ping' }));
      }
    }, 30000);
  }

  private scheduleReconnect(): void {
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
    }

    this.reconnectTimer = setTimeout(() => {
      if (this.connection.isConnected) {
        this.setupWebSocket();
      }
    }, 5000);
  }

  private handleWebSocketEvent(event: any): void {
    switch (event.type) {
      case 'balance_changed':
        this.emitEvent('balanceChanged', event.data);
        break;
      case 'transaction_received':
        this.emitEvent('transactionReceived', event.data);
        break;
      case 'staking_reward':
        this.emitEvent('stakingReward', event.data);
        break;
      case 'pong':
        // Heartbeat response, no action needed
        break;
      default:
        console.log('Unknown WebSocket event:', event);
    }
  }

  private emitEvent(type: WalletEvent['type'], data: any): void {
    const event: WalletEvent = {
      type,
      data,
      timestamp: Date.now()
    };

    this.eventCallbacks.forEach(callback => {
      try {
        callback(event);
      } catch (error) {
        console.error('Error in event callback:', error);
      }
    });
  }

  private notifyConnectionCallbacks(): void {
    this.connectionCallbacks.forEach(callback => {
      try {
        callback(this.connection);
      } catch (error) {
        console.error('Error in connection callback:', error);
      }
    });
  }
}
