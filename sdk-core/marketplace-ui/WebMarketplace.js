import React, { useState, useEffect, useCallback } from 'react';
import { 
    FaStore, 
    FaChartBar, 
    FaUser, 
    FaSearch, 
    FaFilter, 
    FaHeart, 
    FaHeartBroken,
    FaCheckCircle,
    FaShieldAlt,
    FaShoppingCart,
    FaPaperPlane,
    FaPlus,
    FaTimes,
    FaCog,
    FaDownload,
    FaShare,
    FaEye,
    FaEyeSlash
} from 'react-icons/fa';
import { marketplaceAPI } from './marketplace-api';
import './WebMarketplace.css';

const WebMarketplace = ({ 
    walletAddress, 
    onNFTPurchase, 
    onNFTTransfer,
    onShowCreatorProfile,
    onShowAnalytics,
    onShowBulkOperations,
    onShowAdvancedFilters 
}) => {
    const [activeTab, setActiveTab] = useState('marketplace');
    const [viewMode, setViewMode] = useState('verified'); // verified, all, my, favorites
    const [nfts, setNfts] = useState([]);
    const [filteredNfts, setFilteredNfts] = useState([]);
    const [loading, setLoading] = useState(true);
    const [selectedNFT, setSelectedNFT] = useState(null);
    const [showFilters, setShowFilters] = useState(false);
    const [showSearch, setShowSearch] = useState(false);
    const [searchQuery, setSearchQuery] = useState('');
    const [filters, setFilters] = useState({
        category: '',
        creator: '',
        minPrice: '',
        maxPrice: '',
        verifiedOnly: true,
        myNFTsOnly: false,
        favoritedOnly: false,
        hasUnlockableContent: false,
        isTradeLocked: false,
        minMintDate: null,
        maxMintDate: null,
        traits: [],
        sortBy: 'price',
        sortOrder: 'asc'
    });
    const [creatorProfiles, setCreatorProfiles] = useState({});
    const [analytics, setAnalytics] = useState({
        totalVolume24h: 0,
        totalVolume7d: 0,
        totalTransactions24h: 0,
        averagePrice24h: 0,
        topCreators: [],
        topCategories: []
    });
    const [isMobile, setIsMobile] = useState(false);
    const [showMobileMenu, setShowMobileMenu] = useState(false);

    // Check screen size
    useEffect(() => {
        const checkScreenSize = () => {
            setIsMobile(window.innerWidth < 768);
        };
        
        checkScreenSize();
        window.addEventListener('resize', checkScreenSize);
        
        return () => window.removeEventListener('resize', checkScreenSize);
    }, []);

    // Load marketplace data
    const loadMarketplaceData = useCallback(async () => {
        try {
            setLoading(true);
            
            // Fetch NFTs based on view mode
            let nftData = [];
            switch (viewMode) {
                case 'verified':
                    nftData = await marketplaceAPI.getVerifiedNFTs();
                    break;
                case 'all':
                    nftData = await marketplaceAPI.getAllNFTs();
                    break;
                case 'my':
                    nftData = await marketplaceAPI.getMyNFTs(walletAddress);
                    break;
                case 'favorites':
                    nftData = await marketplaceAPI.getFavoritedNFTs(walletAddress);
                    break;
                default:
                    nftData = await marketplaceAPI.getVerifiedNFTs();
            }

            // Load creator profiles for displayed NFTs
            const creatorAddresses = [...new Set(nftData.map(nft => nft.creator))];
            const profiles = {};
            for (const address of creatorAddresses) {
                try {
                    profiles[address] = await marketplaceAPI.getCreatorProfile(address);
                } catch (error) {
                    console.warn(`Failed to load profile for ${address}:`, error);
                }
            }

            setNfts(nftData);
            setCreatorProfiles(profiles);
            
            // Load analytics data
            const analyticsData = await marketplaceAPI.getAnalytics();
            setAnalytics(analyticsData);
            
        } catch (error) {
            console.error('Failed to load marketplace data:', error);
            alert('Failed to load marketplace data. Please try again.');
        } finally {
            setLoading(false);
        }
    }, [viewMode, walletAddress]);

    // Apply filters and search
    const applyFilters = useCallback(() => {
        let filtered = [...nfts];

        // Search filter
        if (searchQuery.trim()) {
            const query = searchQuery.toLowerCase();
            filtered = filtered.filter(nft => 
                nft.name.toLowerCase().includes(query) ||
                nft.description.toLowerCase().includes(query) ||
                nft.creator.toLowerCase().includes(query) ||
                nft.category.toLowerCase().includes(query)
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
            filtered = filtered.filter(nft => nft.listPriceSatoshis >= parseInt(filters.minPrice));
        }
        if (filters.maxPrice) {
            filtered = filtered.filter(nft => nft.listPriceSatoshis <= parseInt(filters.maxPrice));
        }

        // Verification filter
        if (filters.verifiedOnly) {
            filtered = filtered.filter(nft => nft.isVerified);
        }

        // My NFTs filter
        if (filters.myNFTsOnly) {
            filtered = filtered.filter(nft => nft.owner === walletAddress);
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
        if (filters.traits.length > 0) {
            filtered = filtered.filter(nft => 
                filters.traits.every(trait => 
                    nft.traits && nft.traits.some(nftTrait => 
                        nftTrait.type === trait.type && nftTrait.value === trait.value
                    )
                )
            );
        }

        // Sorting
        filtered.sort((a, b) => {
            let aValue, bValue;
            
            switch (filters.sortBy) {
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
                default:
                    aValue = a.listPriceSatoshis || 0;
                    bValue = b.listPriceSatoshis || 0;
            }

            if (filters.sortOrder === 'asc') {
                return aValue > bValue ? 1 : -1;
            } else {
                return aValue < bValue ? 1 : -1;
            }
        });

        setFilteredNfts(filtered);
    }, [nfts, searchQuery, filters, walletAddress]);

    // Apply filters when data changes
    useEffect(() => {
        applyFilters();
    }, [applyFilters]);

    // Load data on mount and when view mode changes
    useEffect(() => {
        loadMarketplaceData();
    }, [loadMarketplaceData]);

    // Handle NFT selection
    const handleNFTPress = (nft) => {
        setSelectedNFT(nft);
    };

    // Handle purchase
    const handlePurchase = async (nft) => {
        try {
            await onNFTPurchase(nft);
            setSelectedNFT(null);
            await loadMarketplaceData(); // Refresh data
            alert('NFT purchased successfully!');
        } catch (error) {
            console.error('Purchase failed:', error);
            alert('Failed to purchase NFT. Please try again.');
        }
    };

    // Handle transfer
    const handleTransfer = async (nft) => {
        try {
            await onNFTTransfer(nft);
            setSelectedNFT(null);
            await loadMarketplaceData(); // Refresh data
            alert('NFT transferred successfully!');
        } catch (error) {
            console.error('Transfer failed:', error);
            alert('Failed to transfer NFT. Please try again.');
        }
    };

    // Toggle favorite
    const toggleFavorite = async (nft) => {
        try {
            await marketplaceAPI.toggleFavorite(nft.id, walletAddress);
            await loadMarketplaceData(); // Refresh data
        } catch (error) {
            console.error('Failed to toggle favorite:', error);
            alert('Failed to update favorite status.');
        }
    };

    // Format price
    const formatPrice = (satoshis) => {
        const shah = satoshis / 100000000;
        return `${shah.toFixed(8)} SHAH`;
    };

    // Format address
    const formatAddress = (address) => {
        return `${address.slice(0, 6)}...${address.slice(-4)}`;
    };

    // Render NFT card
    const renderNFTCard = (nft) => {
        const creatorProfile = creatorProfiles[nft.creator] || {};
        
        return (
            <div 
                key={nft.id}
                className="nft-card"
                onClick={() => handleNFTPress(nft)}
            >
                <div className="nft-image-container">
                    <img 
                        src={nft.imageUrl} 
                        alt={nft.name}
                        className="nft-image"
                    />
                    {nft.isVerified && (
                        <div className="verified-badge">
                            <FaCheckCircle />
                        </div>
                    )}
                    <button 
                        className="favorite-button"
                        onClick={(e) => {
                            e.stopPropagation();
                            toggleFavorite(nft);
                        }}
                    >
                        {nft.isFavorited ? <FaHeart /> : <FaHeartBroken />}
                    </button>
                </div>
                
                <div className="nft-info">
                    <div className="nft-header">
                        <h3 className="nft-name">{nft.name}</h3>
                        {creatorProfile.isTrusted && (
                            <div className="trusted-badge" title="Trusted Creator">
                                <FaShieldAlt />
                            </div>
                        )}
                    </div>
                    
                    <p className="nft-description">{nft.description}</p>
                    
                    <div className="nft-meta">
                        <div className="creator-info">
                            <span className="meta-label">Creator:</span>
                            <button 
                                className="creator-link"
                                onClick={(e) => {
                                    e.stopPropagation();
                                    onShowCreatorProfile(nft.creator);
                                }}
                            >
                                {formatAddress(nft.creator)}
                            </button>
                        </div>
                        
                        <div className="price-info">
                            <span className="meta-label">Price:</span>
                            <span className="price-value">
                                {formatPrice(nft.listPriceSatoshis)}
                            </span>
                        </div>
                    </div>
                    
                    <div className="nft-actions">
                        {nft.owner === walletAddress ? (
                            <button 
                                className="action-button transfer-button"
                                onClick={(e) => {
                                    e.stopPropagation();
                                    handleTransfer(nft);
                                }}
                            >
                                <FaPaperPlane />
                                Transfer
                            </button>
                        ) : (
                            <button 
                                className="action-button buy-button"
                                onClick={(e) => {
                                    e.stopPropagation();
                                    handlePurchase(nft);
                                }}
                            >
                                <FaShoppingCart />
                                Buy
                            </button>
                        )}
                    </div>
                </div>
            </div>
        );
    };

    // Render tab bar
    const renderTabBar = () => (
        <div className="tab-bar">
            <button 
                className={`tab ${activeTab === 'marketplace' ? 'active' : ''}`}
                onClick={() => setActiveTab('marketplace')}
            >
                <FaStore />
                <span>Marketplace</span>
            </button>
            
            <button 
                className={`tab ${activeTab === 'analytics' ? 'active' : ''}`}
                onClick={() => setActiveTab('analytics')}
            >
                <FaChartBar />
                <span>Analytics</span>
            </button>
            
            <button 
                className={`tab ${activeTab === 'profile' ? 'active' : ''}`}
                onClick={() => setActiveTab('profile')}
            >
                <FaUser />
                <span>Profile</span>
            </button>
        </div>
    );

    // Render marketplace content
    const renderMarketplaceContent = () => (
        <div className="content">
            {/* Header with view mode toggle and search */}
            <div className="header">
                <div className="view-mode-toggle">
                    <button 
                        className={`view-mode-button ${viewMode === 'verified' ? 'active' : ''}`}
                        onClick={() => setViewMode('verified')}
                    >
                        Verified
                    </button>
                    
                    <button 
                        className={`view-mode-button ${viewMode === 'all' ? 'active' : ''}`}
                        onClick={() => setViewMode('all')}
                    >
                        All
                    </button>
                    
                    <button 
                        className={`view-mode-button ${viewMode === 'my' ? 'active' : ''}`}
                        onClick={() => setViewMode('my')}
                    >
                        My NFTs
                    </button>
                    
                    <button 
                        className={`view-mode-button ${viewMode === 'favorites' ? 'active' : ''}`}
                        onClick={() => setViewMode('favorites')}
                    >
                        Favorites
                    </button>
                </div>
                
                <div className="header-actions">
                    <button 
                        className="action-icon-button"
                        onClick={() => setShowSearch(!showSearch)}
                        title="Search"
                    >
                        <FaSearch />
                    </button>
                    
                    <button 
                        className="action-icon-button"
                        onClick={() => setShowFilters(!showFilters)}
                        title="Filters"
                    >
                        <FaFilter />
                    </button>
                    
                    {!isMobile && (
                        <>
                            <button 
                                className="action-icon-button"
                                onClick={onShowAdvancedFilters}
                                title="Advanced Filters"
                            >
                                <FaCog />
                            </button>
                            
                            <button 
                                className="action-icon-button"
                                onClick={onShowBulkOperations}
                                title="Bulk Operations"
                            >
                                <FaDownload />
                            </button>
                        </>
                    )}
                </div>
            </div>

            {/* Search bar */}
            {showSearch && (
                <div className="search-container">
                    <div className="search-input-wrapper">
                        <FaSearch className="search-icon" />
                        <input
                            type="text"
                            className="search-input"
                            placeholder="Search NFTs, creators, categories..."
                            value={searchQuery}
                            onChange={(e) => setSearchQuery(e.target.value)}
                        />
                        {searchQuery.length > 0 && (
                            <button 
                                className="clear-search-button"
                                onClick={() => setSearchQuery('')}
                            >
                                <FaTimes />
                            </button>
                        )}
                    </div>
                </div>
            )}

            {/* Filters panel */}
            {showFilters && (
                <div className="filters-panel">
                    <div className="filter-chips">
                        <button 
                            className={`filter-chip ${filters.verifiedOnly ? 'active' : ''}`}
                            onClick={() => setFilters(prev => ({ ...prev, verifiedOnly: !prev.verifiedOnly }))}
                        >
                            Verified Only
                        </button>
                        
                        <button 
                            className={`filter-chip ${filters.myNFTsOnly ? 'active' : ''}`}
                            onClick={() => setFilters(prev => ({ ...prev, myNFTsOnly: !prev.myNFTsOnly }))}
                        >
                            My NFTs
                        </button>
                        
                        <button 
                            className={`filter-chip ${filters.favoritedOnly ? 'active' : ''}`}
                            onClick={() => setFilters(prev => ({ ...prev, favoritedOnly: !prev.favoritedOnly }))}
                        >
                            Favorited
                        </button>
                        
                        <button 
                            className={`filter-chip ${filters.hasUnlockableContent ? 'active' : ''}`}
                            onClick={() => setFilters(prev => ({ ...prev, hasUnlockableContent: !prev.hasUnlockableContent }))}
                        >
                            Unlockable Content
                        </button>
                    </div>
                </div>
            )}

            {/* NFT Grid */}
            {loading ? (
                <div className="loading-container">
                    <div className="loading-spinner"></div>
                    <p className="loading-text">Loading marketplace...</p>
                </div>
            ) : (
                <div className="nft-grid">
                    {filteredNfts.length > 0 ? (
                        filteredNfts.map(renderNFTCard)
                    ) : (
                        <div className="empty-container">
                            <div className="empty-icon">
                                <FaEyeSlash />
                            </div>
                            <h3 className="empty-text">No NFTs found</h3>
                            <p className="empty-subtext">
                                Try adjusting your filters or view mode
                            </p>
                        </div>
                    )}
                </div>
            )}
        </div>
    );

    // Render analytics content
    const renderAnalyticsContent = () => (
        <div className="content">
            <div className="analytics-container">
                <h1 className="analytics-title">Marketplace Analytics</h1>
                
                {/* Volume Stats */}
                <div className="stats-grid">
                    <div className="stat-card">
                        <h3 className="stat-label">24h Volume</h3>
                        <p className="stat-value">
                            {formatPrice(analytics.totalVolume24h)}
                        </p>
                    </div>
                    
                    <div className="stat-card">
                        <h3 className="stat-label">7d Volume</h3>
                        <p className="stat-value">
                            {formatPrice(analytics.totalVolume7d)}
                        </p>
                    </div>
                    
                    <div className="stat-card">
                        <h3 className="stat-label">24h Transactions</h3>
                        <p className="stat-value">
                            {analytics.totalTransactions24h}
                        </p>
                    </div>
                    
                    <div className="stat-card">
                        <h3 className="stat-label">Avg Price</h3>
                        <p className="stat-value">
                            {formatPrice(analytics.averagePrice24h)}
                        </p>
                    </div>
                </div>
                
                {/* Top Creators */}
                <div className="section">
                    <h2 className="section-title">Top Creators</h2>
                    <div className="creator-list">
                        {analytics.topCreators.map((creator, index) => (
                            <div key={creator.address} className="creator-row">
                                <span className="creator-rank">#{index + 1}</span>
                                <span className="creator-name">
                                    {formatAddress(creator.address)}
                                </span>
                                <span className="creator-volume">
                                    {formatPrice(creator.volume)}
                                </span>
                            </div>
                        ))}
                    </div>
                </div>
                
                {/* Top Categories */}
                <div className="section">
                    <h2 className="section-title">Top Categories</h2>
                    <div className="category-list">
                        {analytics.topCategories.map((category, index) => (
                            <div key={category.name} className="category-row">
                                <span className="category-rank">#{index + 1}</span>
                                <span className="category-name">{category.name}</span>
                                <span className="category-count">{category.count} NFTs</span>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
        </div>
    );

    // Render profile content
    const renderProfileContent = () => (
        <div className="content">
            <div className="profile-container">
                <h1 className="profile-title">My Profile</h1>
                
                <div className="profile-stats">
                    <div className="profile-stat">
                        <h3 className="profile-stat-value">
                            {nfts.filter(nft => nft.owner === walletAddress).length}
                        </h3>
                        <p className="profile-stat-label">Owned NFTs</p>
                    </div>
                    
                    <div className="profile-stat">
                        <h3 className="profile-stat-value">
                            {nfts.filter(nft => nft.creator === walletAddress).length}
                        </h3>
                        <p className="profile-stat-label">Created NFTs</p>
                    </div>
                    
                    <div className="profile-stat">
                        <h3 className="profile-stat-value">
                            {nfts.filter(nft => nft.isFavorited).length}
                        </h3>
                        <p className="profile-stat-label">Favorites</p>
                    </div>
                </div>
                
                <button 
                    className="create-nft-button"
                    onClick={() => onShowCreatorProfile(walletAddress)}
                >
                    <FaPlus />
                    Create New NFT
                </button>
            </div>
        </div>
    );

    // Render NFT detail modal
    const renderNFTDetailModal = () => (
        selectedNFT && (
            <div className="modal-overlay" onClick={() => setSelectedNFT(null)}>
                <div className="modal-content" onClick={(e) => e.stopPropagation()}>
                    <div className="modal-header">
                        <h2 className="modal-title">NFT Details</h2>
                        <button 
                            className="close-button"
                            onClick={() => setSelectedNFT(null)}
                        >
                            <FaTimes />
                        </button>
                    </div>
                    
                    <div className="modal-body">
                        <div className="modal-image-container">
                            <img 
                                src={selectedNFT.imageUrl} 
                                alt={selectedNFT.name}
                                className="modal-image"
                            />
                        </div>
                        
                        <div className="modal-info">
                            <div className="modal-header">
                                <h3 className="modal-nft-name">{selectedNFT.name}</h3>
                                {selectedNFT.isVerified && (
                                    <div className="verified-badge">
                                        <FaCheckCircle />
                                    </div>
                                )}
                            </div>
                            
                            <p className="modal-description">{selectedNFT.description}</p>
                            
                            <div className="modal-meta">
                                <div className="modal-meta-row">
                                    <span className="modal-meta-label">Creator:</span>
                                    <button 
                                        className="modal-meta-value creator-link"
                                        onClick={() => {
                                            setSelectedNFT(null);
                                            onShowCreatorProfile(selectedNFT.creator);
                                        }}
                                    >
                                        {formatAddress(selectedNFT.creator)}
                                    </button>
                                </div>
                                
                                <div className="modal-meta-row">
                                    <span className="modal-meta-label">Owner:</span>
                                    <span className="modal-meta-value">
                                        {formatAddress(selectedNFT.owner)}
                                    </span>
                                </div>
                                
                                <div className="modal-meta-row">
                                    <span className="modal-meta-label">Price:</span>
                                    <span className="modal-meta-value">
                                        {formatPrice(selectedNFT.listPriceSatoshis)}
                                    </span>
                                </div>
                                
                                <div className="modal-meta-row">
                                    <span className="modal-meta-label">Mint Date:</span>
                                    <span className="modal-meta-value">
                                        {new Date(selectedNFT.mintDate).toLocaleDateString()}
                                    </span>
                                </div>
                                
                                <div className="modal-meta-row">
                                    <span className="modal-meta-label">Category:</span>
                                    <span className="modal-meta-value">{selectedNFT.category}</span>
                                </div>
                                
                                {selectedNFT.traits && selectedNFT.traits.length > 0 && (
                                    <div className="traits-container">
                                        <h4 className="traits-title">Traits:</h4>
                                        <div className="traits-list">
                                            {selectedNFT.traits.map((trait, index) => (
                                                <div key={index} className="trait-chip">
                                                    <span className="trait-type">{trait.type}</span>
                                                    <span className="trait-value">{trait.value}</span>
                                                </div>
                                            ))}
                                        </div>
                                    </div>
                                )}
                            </div>
                            
                            <div className="modal-actions">
                                <button 
                                    className="modal-favorite-button"
                                    onClick={() => toggleFavorite(selectedNFT)}
                                >
                                    {selectedNFT.isFavorited ? <FaHeart /> : <FaHeartBroken />}
                                    {selectedNFT.isFavorited ? 'Favorited' : 'Favorite'}
                                </button>
                                
                                {selectedNFT.owner === walletAddress ? (
                                    <button 
                                        className="modal-transfer-button"
                                        onClick={() => handleTransfer(selectedNFT)}
                                    >
                                        <FaPaperPlane />
                                        Transfer
                                    </button>
                                ) : (
                                    <button 
                                        className="modal-buy-button"
                                        onClick={() => handlePurchase(selectedNFT)}
                                    >
                                        <FaShoppingCart />
                                        Buy Now
                                    </button>
                                )}
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        )
    );

    return (
        <div className="web-marketplace">
            {renderTabBar()}
            
            {activeTab === 'marketplace' && renderMarketplaceContent()}
            {activeTab === 'analytics' && renderAnalyticsContent()}
            {activeTab === 'profile' && renderProfileContent()}
            
            {renderNFTDetailModal()}
        </div>
    );
};

export default WebMarketplace;
