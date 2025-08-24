// Shahcoin NFT Marketplace - Main Integration File

import React from 'react';
import MobileMarketplace from './MobileMarketplace';
import WebMarketplace from './WebMarketplace';
import { marketplaceAPI } from './marketplace-api';

// Main Marketplace Component that automatically detects platform
const ShahcoinMarketplace = (props) => {
    // Detect if running in React Native environment
    const isReactNative = typeof navigator === 'undefined' || 
                         (navigator.product && navigator.product === 'ReactNative') ||
                         typeof window === 'undefined';

    if (isReactNative) {
        return <MobileMarketplace {...props} />;
    } else {
        return <WebMarketplace {...props} />;
    }
};

// Export individual components for specific use cases
export { MobileMarketplace, WebMarketplace, ShahcoinMarketplace as default };

// Export the API for direct usage
export { marketplaceAPI };

// Export types and utilities
export const MarketplaceUtils = {
    // Format price from satoshis to SHAH
    formatPrice: (satoshis) => {
        const shah = satoshis / 100000000;
        return `${shah.toFixed(8)} SHAH`;
    },

    // Format address for display
    formatAddress: (address) => {
        if (!address || address.length < 8) return address;
        return `${address.slice(0, 6)}...${address.slice(-4)}`;
    },

    // Validate NFT data
    validateNFTData: (nft) => {
        const required = ['id', 'name', 'description', 'imageUrl', 'creator', 'owner'];
        const missing = required.filter(field => !nft[field]);
        
        if (missing.length > 0) {
            throw new Error(`Missing required NFT fields: ${missing.join(', ')}`);
        }
        
        return true;
    },

    // Calculate NFT rarity score
    calculateRarityScore: (nft) => {
        if (!nft.traits || nft.traits.length === 0) return 0;
        
        // Simple rarity calculation based on trait count and uniqueness
        const traitCount = nft.traits.length;
        const uniqueTraits = new Set(nft.traits.map(t => `${t.type}:${t.value}`)).size;
        
        return (traitCount * uniqueTraits) / 100;
    },

    // Sort NFTs by various criteria
    sortNFTs: (nfts, sortBy = 'price', sortOrder = 'asc') => {
        const sorted = [...nfts];
        
        sorted.sort((a, b) => {
            let aValue, bValue;
            
            switch (sortBy) {
                case 'price':
                    aValue = a.listPriceSatoshis || 0;
                    bValue = b.listPriceSatoshis || 0;
                    break;
                case 'name':
                    aValue = a.name.toLowerCase();
                    bValue = b.name.toLowerCase();
                    break;
                case 'mintDate':
                    aValue = new Date(a.mintDate);
                    bValue = new Date(b.mintDate);
                    break;
                case 'creator':
                    aValue = a.creator.toLowerCase();
                    bValue = b.creator.toLowerCase();
                    break;
                case 'rarity':
                    aValue = MarketplaceUtils.calculateRarityScore(a);
                    bValue = MarketplaceUtils.calculateRarityScore(b);
                    break;
                default:
                    aValue = a.listPriceSatoshis || 0;
                    bValue = b.listPriceSatoshis || 0;
            }

            if (sortOrder === 'asc') {
                return aValue > bValue ? 1 : -1;
            } else {
                return aValue < bValue ? 1 : -1;
            }
        });

        return sorted;
    },

    // Filter NFTs by various criteria
    filterNFTs: (nfts, filters = {}) => {
        let filtered = [...nfts];

        // Search filter
        if (filters.search) {
            const query = filters.search.toLowerCase();
            filtered = filtered.filter(nft => 
                nft.name.toLowerCase().includes(query) ||
                nft.description.toLowerCase().includes(query) ||
                nft.creator.toLowerCase().includes(query) ||
                nft.category?.toLowerCase().includes(query)
            );
        }

        // Category filter
        if (filters.category) {
            filtered = filtered.filter(nft => nft.category === filters.category);
        }

        // Creator filter
        if (filters.creator) {
            filtered = filtered.filter(nft => nft.creator === filters.creator);
        }

        // Price range filter
        if (filters.minPrice) {
            filtered = filtered.filter(nft => nft.listPriceSatoshis >= filters.minPrice);
        }
        if (filters.maxPrice) {
            filtered = filtered.filter(nft => nft.listPriceSatoshis <= filters.maxPrice);
        }

        // Verification filter
        if (filters.verifiedOnly) {
            filtered = filtered.filter(nft => nft.isVerified);
        }

        // Owner filter
        if (filters.owner) {
            filtered = filtered.filter(nft => nft.owner === filters.owner);
        }

        // Favorited filter
        if (filters.favoritedOnly) {
            filtered = filtered.filter(nft => nft.isFavorited);
        }

        // Unlockable content filter
        if (filters.hasUnlockableContent) {
            filtered = filtered.filter(nft => nft.hasUnlockableContent);
        }

        // Trade lock filter
        if (filters.isTradeLocked) {
            filtered = filtered.filter(nft => nft.isTradeLocked);
        }

        // Date range filter
        if (filters.minMintDate) {
            filtered = filtered.filter(nft => new Date(nft.mintDate) >= filters.minMintDate);
        }
        if (filters.maxMintDate) {
            filtered = filtered.filter(nft => new Date(nft.mintDate) <= filters.maxMintDate);
        }

        // Traits filter
        if (filters.traits && filters.traits.length > 0) {
            filtered = filtered.filter(nft => 
                filters.traits.every(trait => 
                    nft.traits && nft.traits.some(nftTrait => 
                        nftTrait.type === trait.type && nftTrait.value === trait.value
                    )
                )
            );
        }

        return filtered;
    },

    // Generate analytics data
    generateAnalytics: (nfts, transactions = []) => {
        const now = new Date();
        const oneDayAgo = new Date(now.getTime() - 24 * 60 * 60 * 1000);
        const sevenDaysAgo = new Date(now.getTime() - 7 * 24 * 60 * 60 * 1000);

        // Filter transactions by time
        const transactions24h = transactions.filter(tx => new Date(tx.timestamp) >= oneDayAgo);
        const transactions7d = transactions.filter(tx => new Date(tx.timestamp) >= sevenDaysAgo);

        // Calculate volumes
        const totalVolume24h = transactions24h.reduce((sum, tx) => sum + (tx.amountSatoshis || 0), 0);
        const totalVolume7d = transactions7d.reduce((sum, tx) => sum + (tx.amountSatoshis || 0), 0);

        // Calculate average prices
        const averagePrice24h = transactions24h.length > 0 ? totalVolume24h / transactions24h.length : 0;
        const averagePrice7d = transactions7d.length > 0 ? totalVolume7d / transactions7d.length : 0;

        // Get top creators
        const creatorVolumes = {};
        transactions.forEach(tx => {
            if (tx.creator) {
                creatorVolumes[tx.creator] = (creatorVolumes[tx.creator] || 0) + (tx.amountSatoshis || 0);
            }
        });

        const topCreators = Object.entries(creatorVolumes)
            .map(([address, volume]) => ({ address, volume }))
            .sort((a, b) => b.volume - a.volume)
            .slice(0, 10);

        // Get top categories
        const categoryCounts = {};
        nfts.forEach(nft => {
            if (nft.category) {
                categoryCounts[nft.category] = (categoryCounts[nft.category] || 0) + 1;
            }
        });

        const topCategories = Object.entries(categoryCounts)
            .map(([name, count]) => ({ name, count }))
            .sort((a, b) => b.count - a.count)
            .slice(0, 10);

        return {
            totalVolume24h,
            totalVolume7d,
            totalTransactions24h: transactions24h.length,
            totalTransactions7d: transactions7d.length,
            averagePrice24h,
            averagePrice7d,
            topCreators,
            topCategories
        };
    },

    // Create NFT purchase transaction
    createPurchaseTransaction: async (nft, buyerAddress, walletProvider) => {
        try {
            // Validate NFT data
            MarketplaceUtils.validateNFTData(nft);

            // Create transaction data
            const transactionData = {
                nftId: nft.id,
                sellerAddress: nft.owner,
                buyerAddress: buyerAddress,
                priceSatoshis: nft.listPriceSatoshis,
                timestamp: new Date().toISOString(),
                type: 'purchase'
            };

            // If wallet provider is available, create and sign transaction
            if (walletProvider && typeof walletProvider.createTransaction === 'function') {
                const signedTx = await walletProvider.createTransaction(transactionData);
                return signedTx;
            }

            // Return transaction data for manual processing
            return transactionData;
        } catch (error) {
            console.error('Failed to create purchase transaction:', error);
            throw error;
        }
    },

    // Create NFT transfer transaction
    createTransferTransaction: async (nft, fromAddress, toAddress, walletProvider) => {
        try {
            // Validate NFT data
            MarketplaceUtils.validateNFTData(nft);

            // Validate ownership
            if (nft.owner !== fromAddress) {
                throw new Error('NFT is not owned by the specified address');
            }

            // Create transaction data
            const transactionData = {
                nftId: nft.id,
                fromAddress: fromAddress,
                toAddress: toAddress,
                timestamp: new Date().toISOString(),
                type: 'transfer'
            };

            // If wallet provider is available, create and sign transaction
            if (walletProvider && typeof walletProvider.createTransaction === 'function') {
                const signedTx = await walletProvider.createTransaction(transactionData);
                return signedTx;
            }

            // Return transaction data for manual processing
            return transactionData;
        } catch (error) {
            console.error('Failed to create transfer transaction:', error);
            throw error;
        }
    },

    // Validate marketplace configuration
    validateConfig: (config) => {
        const required = ['apiBaseUrl', 'walletAddress'];
        const missing = required.filter(field => !config[field]);
        
        if (missing.length > 0) {
            throw new Error(`Missing required configuration: ${missing.join(', ')}`);
        }
        
        return true;
    }
};

// Export marketplace configuration
export const MarketplaceConfig = {
    // Default configuration
    defaults: {
        apiBaseUrl: 'https://api.shah.vip/marketplace',
        refreshInterval: 30000, // 30 seconds
        maxRetries: 3,
        timeout: 10000, // 10 seconds
        itemsPerPage: 20,
        enableAnalytics: true,
        enableFavorites: true,
        enableSearch: true,
        enableFilters: true,
        enableBulkOperations: true,
        enableAdvancedFilters: true
    },

    // Create configuration with defaults
    create: (userConfig = {}) => {
        return {
            ...MarketplaceConfig.defaults,
            ...userConfig
        };
    }
};

// Export marketplace hooks for React
export const useMarketplace = (config) => {
    const [nfts, setNfts] = React.useState([]);
    const [loading, setLoading] = React.useState(true);
    const [error, setError] = React.useState(null);
    const [analytics, setAnalytics] = React.useState(null);

    const loadNFTs = React.useCallback(async () => {
        try {
            setLoading(true);
            setError(null);
            
            const nftData = await marketplaceAPI.getVerifiedNFTs();
            setNfts(nftData);
            
            if (config.enableAnalytics) {
                const analyticsData = await marketplaceAPI.getAnalytics();
                setAnalytics(analyticsData);
            }
        } catch (err) {
            setError(err.message);
            console.error('Failed to load marketplace data:', err);
        } finally {
            setLoading(false);
        }
    }, [config.enableAnalytics]);

    const purchaseNFT = React.useCallback(async (nft) => {
        try {
            const transaction = await MarketplaceUtils.createPurchaseTransaction(
                nft, 
                config.walletAddress, 
                config.walletProvider
            );
            
            // Record purchase
            await marketplaceAPI.recordPurchase(transaction);
            
            // Refresh data
            await loadNFTs();
            
            return transaction;
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [config.walletAddress, config.walletProvider, loadNFTs]);

    const transferNFT = React.useCallback(async (nft, toAddress) => {
        try {
            const transaction = await MarketplaceUtils.createTransferTransaction(
                nft, 
                config.walletAddress, 
                toAddress, 
                config.walletProvider
            );
            
            // Record transfer
            await marketplaceAPI.recordTransfer(transaction);
            
            // Refresh data
            await loadNFTs();
            
            return transaction;
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [config.walletAddress, config.walletProvider, loadNFTs]);

    const toggleFavorite = React.useCallback(async (nft) => {
        try {
            await marketplaceAPI.toggleFavorite(nft.id, config.walletAddress);
            await loadNFTs();
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [config.walletAddress, loadNFTs]);

    // Load data on mount
    React.useEffect(() => {
        loadNFTs();
    }, [loadNFTs]);

    // Set up refresh interval
    React.useEffect(() => {
        if (config.refreshInterval > 0) {
            const interval = setInterval(loadNFTs, config.refreshInterval);
            return () => clearInterval(interval);
        }
    }, [loadNFTs, config.refreshInterval]);

    return {
        nfts,
        loading,
        error,
        analytics,
        loadNFTs,
        purchaseNFT,
        transferNFT,
        toggleFavorite,
        utils: MarketplaceUtils
    };
};

// Export types for TypeScript support
export const MarketplaceTypes = {
    NFT: {
        id: 'string',
        name: 'string',
        description: 'string',
        imageUrl: 'string',
        creator: 'string',
        owner: 'string',
        listPriceSatoshis: 'number',
        mintDate: 'string',
        category: 'string',
        traits: 'array',
        isVerified: 'boolean',
        isFavorited: 'boolean',
        hasUnlockableContent: 'boolean',
        isTradeLocked: 'boolean'
    },
    
    Transaction: {
        id: 'string',
        nftId: 'string',
        type: 'string',
        fromAddress: 'string',
        toAddress: 'string',
        amountSatoshis: 'number',
        timestamp: 'string',
        status: 'string'
    },
    
    CreatorProfile: {
        address: 'string',
        totalNFTs: 'number',
        totalVolume: 'number',
        trustScore: 'number',
        isTrusted: 'boolean',
        badges: 'array'
    },
    
    Analytics: {
        totalVolume24h: 'number',
        totalVolume7d: 'number',
        totalTransactions24h: 'number',
        totalTransactions7d: 'number',
        averagePrice24h: 'number',
        averagePrice7d: 'number',
        topCreators: 'array',
        topCategories: 'array'
    }
};

// Export version info
export const VERSION = '1.0.0';
export const BUILD_DATE = new Date().toISOString();

console.log(`Shahcoin NFT Marketplace v${VERSION} loaded (${BUILD_DATE})`);
