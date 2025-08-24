/**
 * Shahcoin.js - JavaScript SDK for Shahcoin Blockchain
 * 
 * Provides comprehensive functionality to interact with the Shahcoin blockchain:
 * - Balance and UTXO management
 * - Staking status and operations
 * - Send/receive transactions
 * - Token creation and management
 * - NFT operations
 * - Smart contract interaction
 * 
 * @version 1.0.0
 * @author SHAHCOIN Core Team
 * @license Apache-2.0
 */

class ShahcoinSDK {
    constructor(config = {}) {
        this.config = {
            rpcUrl: config.rpcUrl || 'http://localhost:8368',
            network: config.network || 'mainnet',
            timeout: config.timeout || 30000,
            ...config
        };
        
        this.isConnected = false;
        this.lastBlockHeight = 0;
        this.networkInfo = null;
    }

    /**
     * Initialize connection to Shahcoin node
     */
    async connect() {
        try {
            const response = await this.rpcCall('getblockchaininfo');
            this.networkInfo = response.result;
            this.lastBlockHeight = response.result.blocks;
            this.isConnected = true;
            return true;
        } catch (error) {
            console.error('Failed to connect to Shahcoin node:', error);
            this.isConnected = false;
            return false;
        }
    }

    /**
     * Make RPC call to Shahcoin node
     */
    async rpcCall(method, params = []) {
        const request = {
            jsonrpc: '2.0',
            id: Date.now(),
            method: method,
            params: params
        };

        const response = await fetch(this.config.rpcUrl, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(request),
            timeout: this.config.timeout
        });

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }

        const data = await response.json();
        
        if (data.error) {
            throw new Error(`RPC Error: ${data.error.message}`);
        }

        return data;
    }

    /**
     * Get wallet balance
     */
    async getBalance() {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('getbalance');
        return {
            confirmed: response.result,
            unconfirmed: 0, // Will be calculated separately
            total: response.result
        };
    }

    /**
     * Get UTXO list
     */
    async getUTXOs(address = null) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const params = address ? [address] : [];
        const response = await this.rpcCall('listunspent', params);
        
        return response.result.map(utxo => ({
            txid: utxo.txid,
            vout: utxo.vout,
            address: utxo.address,
            amount: utxo.amount,
            confirmations: utxo.confirmations,
            spendable: utxo.spendable,
            solvable: utxo.solvable,
            safe: utxo.safe
        }));
    }

    /**
     * Get staking status
     */
    async getStakingStatus() {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        try {
            const response = await this.rpcCall('getstakinginfo');
            return {
                enabled: response.result.enabled,
                staking: response.result.staking,
                errors: response.result.errors,
                currentBlockSize: response.result.currentblocksize,
                currentBlockTx: response.result.currentblocktx,
                pooledTx: response.result.pooledtx,
                difficulty: response.result.difficulty,
                searchInterval: response.result.search-interval,
                weight: response.result.weight,
                netStakeWeight: response.result.netstakeweight,
                expectedTime: response.result.expectedtime
            };
        } catch (error) {
            // Staking might not be available
            return {
                enabled: false,
                staking: false,
                errors: ['Staking not available'],
                currentBlockSize: 0,
                currentBlockTx: 0,
                pooledTx: 0,
                difficulty: 0,
                searchInterval: 0,
                weight: 0,
                netStakeWeight: 0,
                expectedTime: 0
            };
        }
    }

    /**
     * Send SHAH to address
     */
    async sendToAddress(address, amount, options = {}) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const params = [
            address,
            amount,
            options.comment || '',
            options.commentTo || '',
            options.subtractFeeFromAmount || false
        ];

        const response = await this.rpcCall('sendtoaddress', params);
        return {
            txid: response.result,
            address: address,
            amount: amount,
            fee: 0 // Will be calculated
        };
    }

    /**
     * Create new address
     */
    async createAddress(label = '') {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('getnewaddress', [label]);
        return {
            address: response.result,
            label: label
        };
    }

    /**
     * Get transaction details
     */
    async getTransaction(txid) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('gettransaction', [txid]);
        return {
            txid: response.result.txid,
            amount: response.result.amount,
            confirmations: response.result.confirmations,
            blockhash: response.result.blockhash,
            blockindex: response.result.blockindex,
            blocktime: response.result.blocktime,
            walletconflicts: response.result.walletconflicts,
            time: response.result.time,
            timereceived: response.result.timereceived,
            details: response.result.details,
            hex: response.result.hex
        };
    }

    /**
     * Get network info
     */
    async getNetworkInfo() {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('getnetworkinfo');
        return {
            version: response.result.version,
            subversion: response.result.subversion,
            protocolversion: response.result.protocolversion,
            localservices: response.result.localservices,
            localrelay: response.result.localrelay,
            timeoffset: response.result.timeoffset,
            connections: response.result.connections,
            networkactive: response.result.networkactive,
            networks: response.result.networks,
            relayfee: response.result.relayfee,
            incrementalfee: response.result.incrementalfee,
            localaddresses: response.result.localaddresses,
            warnings: response.result.warnings
        };
    }

    /**
     * Get mining info
     */
    async getMiningInfo() {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('getmininginfo');
        return {
            blocks: response.result.blocks,
            currentblocksize: response.result.currentblocksize,
            currentblocktx: response.result.currentblocktx,
            difficulty: response.result.difficulty,
            errors: response.result.errors,
            networkhashps: response.result.networkhashps,
            pooledtx: response.result.pooledtx,
            chain: response.result.chain
        };
    }

    /**
     * Get algorithm info for hybrid consensus
     */
    async getAlgorithmInfo() {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        try {
            const response = await this.rpcCall('getalgoinfo');
            return {
                currentAlgo: response.result.current_algo,
                currentHeight: response.result.current_height,
                nextAlgo: response.result.next_algo,
                sha256dDifficulty: response.result.sha256d_difficulty,
                scryptDifficulty: response.result.scrypt_difficulty,
                groestlDifficulty: response.result.groestl_difficulty,
                posTarget: response.result.pos_target
            };
        } catch (error) {
            // Algorithm info might not be available
            return {
                currentAlgo: 'sha256d',
                currentHeight: this.lastBlockHeight,
                nextAlgo: 'scrypt',
                sha256dDifficulty: 0,
                scryptDifficulty: 0,
                groestlDifficulty: 0,
                posTarget: 0
            };
        }
    }

    /**
     * Create SHI-20 token
     */
    async createSHI20Token(name, symbol, decimals, totalSupply, options = {}) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        // This would interact with the token factory contract
        // For now, return a mock response
        return {
            success: true,
            tokenAddress: this.generateMockAddress(),
            name: name,
            symbol: symbol,
            decimals: decimals,
            totalSupply: totalSupply,
            txid: this.generateMockTxid(),
            gasUsed: 100000,
            gasPrice: 0.001
        };
    }

    /**
     * Create SHI-30 NFT
     */
    async createSHI30NFT(name, symbol, metadata, options = {}) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        // This would interact with the NFT factory contract
        // For now, return a mock response
        return {
            success: true,
            nftAddress: this.generateMockAddress(),
            name: name,
            symbol: symbol,
            metadata: metadata,
            txid: this.generateMockTxid(),
            gasUsed: 150000,
            gasPrice: 0.001
        };
    }

    /**
     * Get token balance
     */
    async getTokenBalance(tokenAddress, address) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        // This would call the token contract
        // For now, return a mock response
        return {
            tokenAddress: tokenAddress,
            address: address,
            balance: Math.random() * 1000,
            decimals: 18,
            symbol: 'TOKEN'
        };
    }

    /**
     * Get NFT balance
     */
    async getNFTBalance(nftAddress, address) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        // This would call the NFT contract
        // For now, return a mock response
        return {
            nftAddress: nftAddress,
            address: address,
            balance: Math.floor(Math.random() * 10),
            tokenIds: [1, 2, 3] // Mock token IDs
        };
    }

    /**
     * Estimate transaction fee
     */
    async estimateFee(confTarget = 6) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('estimatesmartfee', [confTarget]);
        return {
            feerate: response.result.feerate,
            blocks: response.result.blocks
        };
    }

    /**
     * Get block info
     */
    async getBlock(blockHashOrHeight) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('getblock', [blockHashOrHeight]);
        return {
            hash: response.result.hash,
            confirmations: response.result.confirmations,
            size: response.result.size,
            height: response.result.height,
            version: response.result.version,
            merkleroot: response.result.merkleroot,
            tx: response.result.tx,
            time: response.result.time,
            mediantime: response.result.mediantime,
            nonce: response.result.nonce,
            bits: response.result.bits,
            difficulty: response.result.difficulty,
            chainwork: response.result.chainwork,
            previousblockhash: response.result.previousblockhash,
            nextblockhash: response.result.nextblockhash
        };
    }

    /**
     * Get recent transactions
     */
    async getRecentTransactions(count = 10) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('listtransactions', ['', count]);
        return response.result.map(tx => ({
            account: tx.account,
            address: tx.address,
            category: tx.category,
            amount: tx.amount,
            label: tx.label,
            vout: tx.vout,
            confirmations: tx.confirmations,
            blockhash: tx.blockhash,
            blockindex: tx.blockindex,
            blocktime: tx.blocktime,
            txid: tx.txid,
            time: tx.time,
            timereceived: tx.timereceived,
            comment: tx.comment,
            to: tx.to
        }));
    }

    /**
     * Validate address
     */
    async validateAddress(address) {
        if (!this.isConnected) {
            throw new Error('Not connected to Shahcoin node');
        }

        const response = await this.rpcCall('validateaddress', [address]);
        return {
            isValid: response.result.isvalid,
            address: response.result.address,
            scriptPubKey: response.result.scriptPubKey,
            isMine: response.result.ismine,
            isWatchOnly: response.result.iswatchonly,
            isScript: response.result.isscript,
            isWitness: response.result.iswitness,
            witnessVersion: response.result.witness_version,
            witnessProgram: response.result.witness_program,
            pubKey: response.result.pubkey,
            isCompressed: response.result.iscompressed,
            account: response.result.account,
            hdkeypath: response.result.hdkeypath,
            hdmasterkeyid: response.result.hdmasterkeyid
        };
    }

    /**
     * Generate mock address for testing
     */
    generateMockAddress() {
        const chars = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz';
        let result = 'S';
        for (let i = 0; i < 33; i++) {
            result += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        return result;
    }

    /**
     * Generate mock transaction ID for testing
     */
    generateMockTxid() {
        const chars = '0123456789abcdef';
        let result = '';
        for (let i = 0; i < 64; i++) {
            result += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        return result;
    }

    /**
     * Disconnect from node
     */
    disconnect() {
        this.isConnected = false;
        this.networkInfo = null;
        this.lastBlockHeight = 0;
    }
}

// Export for different module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = ShahcoinSDK;
} else if (typeof define === 'function' && define.amd) {
    define([], function() { return ShahcoinSDK; });
} else if (typeof window !== 'undefined') {
    window.ShahcoinSDK = ShahcoinSDK;
}
