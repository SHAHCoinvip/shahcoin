import React, { useState, useEffect, useCallback } from 'react';
import { 
    View, 
    Text, 
    TouchableOpacity, 
    ScrollView, 
    FlatList, 
    Image, 
    TextInput, 
    Modal, 
    Alert,
    Dimensions,
    RefreshControl,
    ActivityIndicator
} from 'react-native';
import { 
    Ionicons, 
    MaterialIcons, 
    FontAwesome5 
} from '@expo/vector-icons';
import { marketplaceAPI } from './marketplace-api';

const { width, height } = Dimensions.get('window');
const isTablet = width >= 768;

const MobileMarketplace = ({ 
    walletAddress, 
    onNFTPurchase, 
    onNFTTransfer,
    onShowCreatorProfile,
    onShowAnalytics 
}) => {
    const [activeTab, setActiveTab] = useState('marketplace');
    const [viewMode, setViewMode] = useState('verified'); // verified, all, my, favorites
    const [nfts, setNfts] = useState([]);
    const [filteredNfts, setFilteredNfts] = useState([]);
    const [loading, setLoading] = useState(true);
    const [refreshing, setRefreshing] = useState(false);
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
            Alert.alert('Error', 'Failed to load marketplace data. Please try again.');
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

    // Pull to refresh
    const onRefresh = useCallback(async () => {
        setRefreshing(true);
        await loadMarketplaceData();
        setRefreshing(false);
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
            Alert.alert('Success', 'NFT purchased successfully!');
        } catch (error) {
            console.error('Purchase failed:', error);
            Alert.alert('Error', 'Failed to purchase NFT. Please try again.');
        }
    };

    // Handle transfer
    const handleTransfer = async (nft) => {
        try {
            await onNFTTransfer(nft);
            setSelectedNFT(null);
            await loadMarketplaceData(); // Refresh data
            Alert.alert('Success', 'NFT transferred successfully!');
        } catch (error) {
            console.error('Transfer failed:', error);
            Alert.alert('Error', 'Failed to transfer NFT. Please try again.');
        }
    };

    // Toggle favorite
    const toggleFavorite = async (nft) => {
        try {
            await marketplaceAPI.toggleFavorite(nft.id, walletAddress);
            await loadMarketplaceData(); // Refresh data
        } catch (error) {
            console.error('Failed to toggle favorite:', error);
            Alert.alert('Error', 'Failed to update favorite status.');
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
    const renderNFTCard = ({ item: nft }) => {
        const creatorProfile = creatorProfiles[nft.creator] || {};
        
        return (
            <TouchableOpacity 
                style={styles.nftCard}
                onPress={() => handleNFTPress(nft)}
            >
                <Image 
                    source={{ uri: nft.imageUrl }} 
                    style={styles.nftImage}
                    resizeMode="cover"
                />
                
                <View style={styles.nftInfo}>
                    <View style={styles.nftHeader}>
                        <Text style={styles.nftName} numberOfLines={1}>
                            {nft.name}
                        </Text>
                        {nft.isVerified && (
                            <Ionicons name="checkmark-circle" size={16} color="#4CAF50" />
                        )}
                    </View>
                    
                    <Text style={styles.nftDescription} numberOfLines={2}>
                        {nft.description}
                    </Text>
                    
                    <View style={styles.nftMeta}>
                        <View style={styles.creatorInfo}>
                            <Text style={styles.creatorLabel}>Creator:</Text>
                            <TouchableOpacity 
                                onPress={() => onShowCreatorProfile(nft.creator)}
                                style={styles.creatorButton}
                            >
                                <Text style={styles.creatorAddress}>
                                    {formatAddress(nft.creator)}
                                </Text>
                                {creatorProfile.isTrusted && (
                                    <Ionicons name="shield-checkmark" size={12} color="#2196F3" />
                                )}
                            </TouchableOpacity>
                        </View>
                        
                        <View style={styles.priceInfo}>
                            <Text style={styles.priceLabel}>Price:</Text>
                            <Text style={styles.priceValue}>
                                {formatPrice(nft.listPriceSatoshis)}
                            </Text>
                        </View>
                    </View>
                    
                    <View style={styles.nftActions}>
                        <TouchableOpacity 
                            style={styles.favoriteButton}
                            onPress={() => toggleFavorite(nft)}
                        >
                            <Ionicons 
                                name={nft.isFavorited ? "heart" : "heart-outline"} 
                                size={20} 
                                color={nft.isFavorited ? "#E91E63" : "#666"} 
                            />
                        </TouchableOpacity>
                        
                        {nft.owner === walletAddress ? (
                            <TouchableOpacity 
                                style={styles.transferButton}
                                onPress={() => handleTransfer(nft)}
                            >
                                <Text style={styles.transferButtonText}>Transfer</Text>
                            </TouchableOpacity>
                        ) : (
                            <TouchableOpacity 
                                style={styles.buyButton}
                                onPress={() => handlePurchase(nft)}
                            >
                                <Text style={styles.buyButtonText}>Buy</Text>
                            </TouchableOpacity>
                        )}
                    </View>
                </View>
            </TouchableOpacity>
        );
    };

    // Render tab bar
    const renderTabBar = () => (
        <View style={styles.tabBar}>
            <TouchableOpacity 
                style={[styles.tab, activeTab === 'marketplace' && styles.activeTab]}
                onPress={() => setActiveTab('marketplace')}
            >
                <Ionicons 
                    name="storefront" 
                    size={20} 
                    color={activeTab === 'marketplace' ? '#2196F3' : '#666'} 
                />
                <Text style={[styles.tabText, activeTab === 'marketplace' && styles.activeTabText]}>
                    Marketplace
                </Text>
            </TouchableOpacity>
            
            <TouchableOpacity 
                style={[styles.tab, activeTab === 'analytics' && styles.activeTab]}
                onPress={() => setActiveTab('analytics')}
            >
                <Ionicons 
                    name="analytics" 
                    size={20} 
                    color={activeTab === 'analytics' ? '#2196F3' : '#666'} 
                />
                <Text style={[styles.tabText, activeTab === 'analytics' && styles.activeTabText]}>
                    Analytics
                </Text>
            </TouchableOpacity>
            
            <TouchableOpacity 
                style={[styles.tab, activeTab === 'profile' && styles.activeTab]}
                onPress={() => setActiveTab('profile')}
            >
                <Ionicons 
                    name="person" 
                    size={20} 
                    color={activeTab === 'profile' ? '#2196F3' : '#666'} 
                />
                <Text style={[styles.tabText, activeTab === 'profile' && styles.activeTabText]}>
                    Profile
                </Text>
            </TouchableOpacity>
        </View>
    );

    // Render marketplace content
    const renderMarketplaceContent = () => (
        <View style={styles.content}>
            {/* Header with view mode toggle and search */}
            <View style={styles.header}>
                <View style={styles.viewModeToggle}>
                    <TouchableOpacity 
                        style={[styles.viewModeButton, viewMode === 'verified' && styles.activeViewMode]}
                        onPress={() => setViewMode('verified')}
                    >
                        <Text style={[styles.viewModeText, viewMode === 'verified' && styles.activeViewModeText]}>
                            Verified
                        </Text>
                    </TouchableOpacity>
                    
                    <TouchableOpacity 
                        style={[styles.viewModeButton, viewMode === 'all' && styles.activeViewMode]}
                        onPress={() => setViewMode('all')}
                    >
                        <Text style={[styles.viewModeText, viewMode === 'all' && styles.activeViewModeText]}>
                            All
                        </Text>
                    </TouchableOpacity>
                    
                    <TouchableOpacity 
                        style={[styles.viewModeButton, viewMode === 'my' && styles.activeViewMode]}
                        onPress={() => setViewMode('my')}
                    >
                        <Text style={[styles.viewModeText, viewMode === 'my' && styles.activeViewModeText]}>
                            My NFTs
                        </Text>
                    </TouchableOpacity>
                    
                    <TouchableOpacity 
                        style={[styles.viewModeButton, viewMode === 'favorites' && styles.activeViewMode]}
                        onPress={() => setViewMode('favorites')}
                    >
                        <Text style={[styles.viewModeText, viewMode === 'favorites' && styles.activeViewModeText]}>
                            Favorites
                        </Text>
                    </TouchableOpacity>
                </View>
                
                <View style={styles.headerActions}>
                    <TouchableOpacity 
                        style={styles.searchButton}
                        onPress={() => setShowSearch(!showSearch)}
                    >
                        <Ionicons name="search" size={20} color="#666" />
                    </TouchableOpacity>
                    
                    <TouchableOpacity 
                        style={styles.filterButton}
                        onPress={() => setShowFilters(!showFilters)}
                    >
                        <Ionicons name="filter" size={20} color="#666" />
                    </TouchableOpacity>
                </View>
            </View>

            {/* Search bar */}
            {showSearch && (
                <View style={styles.searchContainer}>
                    <TextInput
                        style={styles.searchInput}
                        placeholder="Search NFTs, creators, categories..."
                        value={searchQuery}
                        onChangeText={setSearchQuery}
                        placeholderTextColor="#999"
                    />
                    {searchQuery.length > 0 && (
                        <TouchableOpacity 
                            style={styles.clearSearchButton}
                            onPress={() => setSearchQuery('')}
                        >
                            <Ionicons name="close-circle" size={20} color="#999" />
                        </TouchableOpacity>
                    )}
                </View>
            )}

            {/* Filters panel */}
            {showFilters && (
                <View style={styles.filtersPanel}>
                    <ScrollView horizontal showsHorizontalScrollIndicator={false}>
                        <View style={styles.filterChips}>
                            <TouchableOpacity 
                                style={[styles.filterChip, filters.verifiedOnly && styles.activeFilterChip]}
                                onPress={() => setFilters(prev => ({ ...prev, verifiedOnly: !prev.verifiedOnly }))}
                            >
                                <Text style={[styles.filterChipText, filters.verifiedOnly && styles.activeFilterChipText]}>
                                    Verified Only
                                </Text>
                            </TouchableOpacity>
                            
                            <TouchableOpacity 
                                style={[styles.filterChip, filters.myNFTsOnly && styles.activeFilterChip]}
                                onPress={() => setFilters(prev => ({ ...prev, myNFTsOnly: !prev.myNFTsOnly }))}
                            >
                                <Text style={[styles.filterChipText, filters.myNFTsOnly && styles.activeFilterChipText]}>
                                    My NFTs
                                </Text>
                            </TouchableOpacity>
                            
                            <TouchableOpacity 
                                style={[styles.filterChip, filters.favoritedOnly && styles.activeFilterChip]}
                                onPress={() => setFilters(prev => ({ ...prev, favoritedOnly: !prev.favoritedOnly }))}
                            >
                                <Text style={[styles.filterChipText, filters.favoritedOnly && styles.activeFilterChipText]}>
                                    Favorited
                                </Text>
                            </TouchableOpacity>
                            
                            <TouchableOpacity 
                                style={[styles.filterChip, filters.hasUnlockableContent && styles.activeFilterChip]}
                                onPress={() => setFilters(prev => ({ ...prev, hasUnlockableContent: !prev.hasUnlockableContent }))}
                            >
                                <Text style={[styles.filterChipText, filters.hasUnlockableContent && styles.activeFilterChipText]}>
                                    Unlockable Content
                                </Text>
                            </TouchableOpacity>
                        </View>
                    </ScrollView>
                </View>
            )}

            {/* NFT List */}
            {loading ? (
                <View style={styles.loadingContainer}>
                    <ActivityIndicator size="large" color="#2196F3" />
                    <Text style={styles.loadingText}>Loading marketplace...</Text>
                </View>
            ) : (
                <FlatList
                    data={filteredNfts}
                    renderItem={renderNFTCard}
                    keyExtractor={(item) => item.id}
                    numColumns={isTablet ? 3 : 2}
                    columnWrapperStyle={isTablet ? null : styles.nftRow}
                    showsVerticalScrollIndicator={false}
                    refreshControl={
                        <RefreshControl
                            refreshing={refreshing}
                            onRefresh={onRefresh}
                            colors={['#2196F3']}
                        />
                    }
                    ListEmptyComponent={
                        <View style={styles.emptyContainer}>
                            <Ionicons name="images-outline" size={64} color="#ccc" />
                            <Text style={styles.emptyText}>No NFTs found</Text>
                            <Text style={styles.emptySubtext}>
                                Try adjusting your filters or view mode
                            </Text>
                        </View>
                    }
                    contentContainerStyle={styles.nftList}
                />
            )}
        </View>
    );

    // Render analytics content
    const renderAnalyticsContent = () => (
        <View style={styles.content}>
            <ScrollView showsVerticalScrollIndicator={false}>
                <View style={styles.analyticsContainer}>
                    <Text style={styles.analyticsTitle}>Marketplace Analytics</Text>
                    
                    {/* Volume Stats */}
                    <View style={styles.statsGrid}>
                        <View style={styles.statCard}>
                            <Text style={styles.statLabel}>24h Volume</Text>
                            <Text style={styles.statValue}>
                                {formatPrice(analytics.totalVolume24h)}
                            </Text>
                        </View>
                        
                        <View style={styles.statCard}>
                            <Text style={styles.statLabel}>7d Volume</Text>
                            <Text style={styles.statValue}>
                                {formatPrice(analytics.totalVolume7d)}
                            </Text>
                        </View>
                        
                        <View style={styles.statCard}>
                            <Text style={styles.statLabel}>24h Transactions</Text>
                            <Text style={styles.statValue}>
                                {analytics.totalTransactions24h}
                            </Text>
                        </View>
                        
                        <View style={styles.statCard}>
                            <Text style={styles.statLabel}>Avg Price</Text>
                            <Text style={styles.statValue}>
                                {formatPrice(analytics.averagePrice24h)}
                            </Text>
                        </View>
                    </View>
                    
                    {/* Top Creators */}
                    <View style={styles.section}>
                        <Text style={styles.sectionTitle}>Top Creators</Text>
                        {analytics.topCreators.map((creator, index) => (
                            <View key={creator.address} style={styles.creatorRow}>
                                <Text style={styles.creatorRank}>#{index + 1}</Text>
                                <Text style={styles.creatorName}>
                                    {formatAddress(creator.address)}
                                </Text>
                                <Text style={styles.creatorVolume}>
                                    {formatPrice(creator.volume)}
                                </Text>
                            </View>
                        ))}
                    </View>
                    
                    {/* Top Categories */}
                    <View style={styles.section}>
                        <Text style={styles.sectionTitle}>Top Categories</Text>
                        {analytics.topCategories.map((category, index) => (
                            <View key={category.name} style={styles.categoryRow}>
                                <Text style={styles.categoryRank}>#{index + 1}</Text>
                                <Text style={styles.categoryName}>{category.name}</Text>
                                <Text style={styles.categoryCount}>{category.count} NFTs</Text>
                            </View>
                        ))}
                    </View>
                </View>
            </ScrollView>
        </View>
    );

    // Render profile content
    const renderProfileContent = () => (
        <View style={styles.content}>
            <ScrollView showsVerticalScrollIndicator={false}>
                <View style={styles.profileContainer}>
                    <Text style={styles.profileTitle}>My Profile</Text>
                    
                    <View style={styles.profileStats}>
                        <View style={styles.profileStat}>
                            <Text style={styles.profileStatValue}>
                                {nfts.filter(nft => nft.owner === walletAddress).length}
                            </Text>
                            <Text style={styles.profileStatLabel}>Owned NFTs</Text>
                        </View>
                        
                        <View style={styles.profileStat}>
                            <Text style={styles.profileStatValue}>
                                {nfts.filter(nft => nft.creator === walletAddress).length}
                            </Text>
                            <Text style={styles.profileStatLabel}>Created NFTs</Text>
                        </View>
                        
                        <View style={styles.profileStat}>
                            <Text style={styles.profileStatValue}>
                                {nfts.filter(nft => nft.isFavorited).length}
                            </Text>
                            <Text style={styles.profileStatLabel}>Favorites</Text>
                        </View>
                    </View>
                    
                    <TouchableOpacity 
                        style={styles.createNFTButton}
                        onPress={() => onShowCreatorProfile(walletAddress)}
                    >
                        <Ionicons name="add-circle" size={20} color="#fff" />
                        <Text style={styles.createNFTButtonText}>Create New NFT</Text>
                    </TouchableOpacity>
                </View>
            </ScrollView>
        </View>
    );

    // Render NFT detail modal
    const renderNFTDetailModal = () => (
        <Modal
            visible={selectedNFT !== null}
            animationType="slide"
            presentationStyle="pageSheet"
        >
            {selectedNFT && (
                <View style={styles.modalContainer}>
                    <View style={styles.modalHeader}>
                        <TouchableOpacity 
                            style={styles.closeButton}
                            onPress={() => setSelectedNFT(null)}
                        >
                            <Ionicons name="close" size={24} color="#666" />
                        </TouchableOpacity>
                        <Text style={styles.modalTitle}>NFT Details</Text>
                        <View style={styles.placeholder} />
                    </View>
                    
                    <ScrollView style={styles.modalContent}>
                        <Image 
                            source={{ uri: selectedNFT.imageUrl }} 
                            style={styles.modalImage}
                            resizeMode="contain"
                        />
                        
                        <View style={styles.modalInfo}>
                            <View style={styles.modalHeader}>
                                <Text style={styles.modalNFTName}>{selectedNFT.name}</Text>
                                {selectedNFT.isVerified && (
                                    <Ionicons name="checkmark-circle" size={20} color="#4CAF50" />
                                )}
                            </View>
                            
                            <Text style={styles.modalDescription}>{selectedNFT.description}</Text>
                            
                            <View style={styles.modalMeta}>
                                <View style={styles.modalMetaRow}>
                                    <Text style={styles.modalMetaLabel}>Creator:</Text>
                                    <TouchableOpacity 
                                        onPress={() => {
                                            setSelectedNFT(null);
                                            onShowCreatorProfile(selectedNFT.creator);
                                        }}
                                    >
                                        <Text style={styles.modalMetaValue}>
                                            {formatAddress(selectedNFT.creator)}
                                        </Text>
                                    </TouchableOpacity>
                                </View>
                                
                                <View style={styles.modalMetaRow}>
                                    <Text style={styles.modalMetaLabel}>Owner:</Text>
                                    <Text style={styles.modalMetaValue}>
                                        {formatAddress(selectedNFT.owner)}
                                    </Text>
                                </View>
                                
                                <View style={styles.modalMetaRow}>
                                    <Text style={styles.modalMetaLabel}>Price:</Text>
                                    <Text style={styles.modalMetaValue}>
                                        {formatPrice(selectedNFT.listPriceSatoshis)}
                                    </Text>
                                </View>
                                
                                <View style={styles.modalMetaRow}>
                                    <Text style={styles.modalMetaLabel}>Mint Date:</Text>
                                    <Text style={styles.modalMetaValue}>
                                        {new Date(selectedNFT.mintDate).toLocaleDateString()}
                                    </Text>
                                </View>
                                
                                <View style={styles.modalMetaRow}>
                                    <Text style={styles.modalMetaLabel}>Category:</Text>
                                    <Text style={styles.modalMetaValue}>{selectedNFT.category}</Text>
                                </View>
                                
                                {selectedNFT.traits && selectedNFT.traits.length > 0 && (
                                    <View style={styles.traitsContainer}>
                                        <Text style={styles.traitsTitle}>Traits:</Text>
                                        <View style={styles.traitsList}>
                                            {selectedNFT.traits.map((trait, index) => (
                                                <View key={index} style={styles.traitChip}>
                                                    <Text style={styles.traitType}>{trait.type}</Text>
                                                    <Text style={styles.traitValue}>{trait.value}</Text>
                                                </View>
                                            ))}
                                        </View>
                                    </View>
                                )}
                            </View>
                            
                            <View style={styles.modalActions}>
                                <TouchableOpacity 
                                    style={styles.modalFavoriteButton}
                                    onPress={() => toggleFavorite(selectedNFT)}
                                >
                                    <Ionicons 
                                        name={selectedNFT.isFavorited ? "heart" : "heart-outline"} 
                                        size={24} 
                                        color={selectedNFT.isFavorited ? "#E91E63" : "#666"} 
                                    />
                                    <Text style={styles.modalFavoriteText}>
                                        {selectedNFT.isFavorited ? 'Favorited' : 'Favorite'}
                                    </Text>
                                </TouchableOpacity>
                                
                                {selectedNFT.owner === walletAddress ? (
                                    <TouchableOpacity 
                                        style={styles.modalTransferButton}
                                        onPress={() => handleTransfer(selectedNFT)}
                                    >
                                        <Ionicons name="send" size={20} color="#fff" />
                                        <Text style={styles.modalTransferButtonText}>Transfer</Text>
                                    </TouchableOpacity>
                                ) : (
                                    <TouchableOpacity 
                                        style={styles.modalBuyButton}
                                        onPress={() => handlePurchase(selectedNFT)}
                                    >
                                        <Ionicons name="cart" size={20} color="#fff" />
                                        <Text style={styles.modalBuyButtonText}>Buy Now</Text>
                                    </TouchableOpacity>
                                )}
                            </View>
                        </View>
                    </ScrollView>
                </View>
            )}
        </Modal>
    );

    return (
        <View style={styles.container}>
            {renderTabBar()}
            
            {activeTab === 'marketplace' && renderMarketplaceContent()}
            {activeTab === 'analytics' && renderAnalyticsContent()}
            {activeTab === 'profile' && renderProfileContent()}
            
            {renderNFTDetailModal()}
        </View>
    );
};

const styles = {
    container: {
        flex: 1,
        backgroundColor: '#f5f5f5',
    },
    tabBar: {
        flexDirection: 'row',
        backgroundColor: '#fff',
        borderBottomWidth: 1,
        borderBottomColor: '#e0e0e0',
        paddingHorizontal: 16,
        paddingVertical: 8,
    },
    tab: {
        flex: 1,
        alignItems: 'center',
        paddingVertical: 8,
        borderRadius: 8,
    },
    activeTab: {
        backgroundColor: '#e3f2fd',
    },
    tabText: {
        fontSize: 12,
        color: '#666',
        marginTop: 4,
    },
    activeTabText: {
        color: '#2196F3',
        fontWeight: '600',
    },
    content: {
        flex: 1,
    },
    header: {
        flexDirection: 'row',
        justifyContent: 'space-between',
        alignItems: 'center',
        paddingHorizontal: 16,
        paddingVertical: 12,
        backgroundColor: '#fff',
        borderBottomWidth: 1,
        borderBottomColor: '#e0e0e0',
    },
    viewModeToggle: {
        flexDirection: 'row',
        backgroundColor: '#f0f0f0',
        borderRadius: 8,
        padding: 2,
    },
    viewModeButton: {
        paddingHorizontal: 12,
        paddingVertical: 6,
        borderRadius: 6,
    },
    activeViewMode: {
        backgroundColor: '#fff',
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 1 },
        shadowOpacity: 0.1,
        shadowRadius: 2,
        elevation: 2,
    },
    viewModeText: {
        fontSize: 12,
        color: '#666',
        fontWeight: '500',
    },
    activeViewModeText: {
        color: '#2196F3',
        fontWeight: '600',
    },
    headerActions: {
        flexDirection: 'row',
        gap: 12,
    },
    searchButton: {
        padding: 8,
    },
    filterButton: {
        padding: 8,
    },
    searchContainer: {
        flexDirection: 'row',
        alignItems: 'center',
        paddingHorizontal: 16,
        paddingVertical: 8,
        backgroundColor: '#fff',
        borderBottomWidth: 1,
        borderBottomColor: '#e0e0e0',
    },
    searchInput: {
        flex: 1,
        height: 40,
        backgroundColor: '#f5f5f5',
        borderRadius: 20,
        paddingHorizontal: 16,
        fontSize: 16,
        color: '#333',
    },
    clearSearchButton: {
        marginLeft: 8,
        padding: 4,
    },
    filtersPanel: {
        backgroundColor: '#fff',
        borderBottomWidth: 1,
        borderBottomColor: '#e0e0e0',
        paddingVertical: 8,
    },
    filterChips: {
        flexDirection: 'row',
        paddingHorizontal: 16,
        gap: 8,
    },
    filterChip: {
        paddingHorizontal: 12,
        paddingVertical: 6,
        backgroundColor: '#f0f0f0',
        borderRadius: 16,
        borderWidth: 1,
        borderColor: '#e0e0e0',
    },
    activeFilterChip: {
        backgroundColor: '#2196F3',
        borderColor: '#2196F3',
    },
    filterChipText: {
        fontSize: 12,
        color: '#666',
    },
    activeFilterChipText: {
        color: '#fff',
        fontWeight: '600',
    },
    nftList: {
        padding: 8,
    },
    nftRow: {
        justifyContent: 'space-between',
    },
    nftCard: {
        flex: 1,
        backgroundColor: '#fff',
        borderRadius: 12,
        margin: 4,
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
        overflow: 'hidden',
    },
    nftImage: {
        width: '100%',
        height: isTablet ? 200 : 150,
    },
    nftInfo: {
        padding: 12,
    },
    nftHeader: {
        flexDirection: 'row',
        alignItems: 'center',
        marginBottom: 4,
    },
    nftName: {
        flex: 1,
        fontSize: 14,
        fontWeight: '600',
        color: '#333',
        marginRight: 4,
    },
    nftDescription: {
        fontSize: 12,
        color: '#666',
        marginBottom: 8,
        lineHeight: 16,
    },
    nftMeta: {
        marginBottom: 8,
    },
    creatorInfo: {
        flexDirection: 'row',
        alignItems: 'center',
        marginBottom: 4,
    },
    creatorLabel: {
        fontSize: 11,
        color: '#999',
        marginRight: 4,
    },
    creatorButton: {
        flexDirection: 'row',
        alignItems: 'center',
    },
    creatorAddress: {
        fontSize: 11,
        color: '#2196F3',
        fontWeight: '500',
    },
    priceInfo: {
        flexDirection: 'row',
        alignItems: 'center',
    },
    priceLabel: {
        fontSize: 11,
        color: '#999',
        marginRight: 4,
    },
    priceValue: {
        fontSize: 12,
        fontWeight: '600',
        color: '#333',
    },
    nftActions: {
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'space-between',
    },
    favoriteButton: {
        padding: 4,
    },
    buyButton: {
        backgroundColor: '#4CAF50',
        paddingHorizontal: 12,
        paddingVertical: 6,
        borderRadius: 6,
    },
    buyButtonText: {
        color: '#fff',
        fontSize: 12,
        fontWeight: '600',
    },
    transferButton: {
        backgroundColor: '#FF9800',
        paddingHorizontal: 12,
        paddingVertical: 6,
        borderRadius: 6,
    },
    transferButtonText: {
        color: '#fff',
        fontSize: 12,
        fontWeight: '600',
    },
    loadingContainer: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
    },
    loadingText: {
        marginTop: 16,
        fontSize: 16,
        color: '#666',
    },
    emptyContainer: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        paddingVertical: 64,
    },
    emptyText: {
        fontSize: 18,
        color: '#666',
        marginTop: 16,
    },
    emptySubtext: {
        fontSize: 14,
        color: '#999',
        marginTop: 8,
        textAlign: 'center',
    },
    analyticsContainer: {
        padding: 16,
    },
    analyticsTitle: {
        fontSize: 24,
        fontWeight: 'bold',
        color: '#333',
        marginBottom: 24,
    },
    statsGrid: {
        flexDirection: 'row',
        flexWrap: 'wrap',
        gap: 12,
        marginBottom: 24,
    },
    statCard: {
        flex: 1,
        minWidth: 150,
        backgroundColor: '#fff',
        padding: 16,
        borderRadius: 12,
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
    },
    statLabel: {
        fontSize: 12,
        color: '#666',
        marginBottom: 4,
    },
    statValue: {
        fontSize: 18,
        fontWeight: 'bold',
        color: '#333',
    },
    section: {
        backgroundColor: '#fff',
        borderRadius: 12,
        padding: 16,
        marginBottom: 16,
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
    },
    sectionTitle: {
        fontSize: 18,
        fontWeight: '600',
        color: '#333',
        marginBottom: 12,
    },
    creatorRow: {
        flexDirection: 'row',
        alignItems: 'center',
        paddingVertical: 8,
        borderBottomWidth: 1,
        borderBottomColor: '#f0f0f0',
    },
    creatorRank: {
        fontSize: 14,
        fontWeight: '600',
        color: '#2196F3',
        marginRight: 12,
        minWidth: 30,
    },
    creatorName: {
        flex: 1,
        fontSize: 14,
        color: '#333',
    },
    creatorVolume: {
        fontSize: 14,
        fontWeight: '600',
        color: '#4CAF50',
    },
    categoryRow: {
        flexDirection: 'row',
        alignItems: 'center',
        paddingVertical: 8,
        borderBottomWidth: 1,
        borderBottomColor: '#f0f0f0',
    },
    categoryRank: {
        fontSize: 14,
        fontWeight: '600',
        color: '#2196F3',
        marginRight: 12,
        minWidth: 30,
    },
    categoryName: {
        flex: 1,
        fontSize: 14,
        color: '#333',
    },
    categoryCount: {
        fontSize: 14,
        color: '#666',
    },
    profileContainer: {
        padding: 16,
    },
    profileTitle: {
        fontSize: 24,
        fontWeight: 'bold',
        color: '#333',
        marginBottom: 24,
    },
    profileStats: {
        flexDirection: 'row',
        gap: 16,
        marginBottom: 24,
    },
    profileStat: {
        flex: 1,
        backgroundColor: '#fff',
        padding: 16,
        borderRadius: 12,
        alignItems: 'center',
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
    },
    profileStatValue: {
        fontSize: 24,
        fontWeight: 'bold',
        color: '#2196F3',
        marginBottom: 4,
    },
    profileStatLabel: {
        fontSize: 12,
        color: '#666',
    },
    createNFTButton: {
        backgroundColor: '#2196F3',
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'center',
        paddingVertical: 16,
        borderRadius: 12,
        shadowColor: '#000',
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
    },
    createNFTButtonText: {
        color: '#fff',
        fontSize: 16,
        fontWeight: '600',
        marginLeft: 8,
    },
    modalContainer: {
        flex: 1,
        backgroundColor: '#fff',
    },
    modalHeader: {
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'space-between',
        paddingHorizontal: 16,
        paddingVertical: 12,
        borderBottomWidth: 1,
        borderBottomColor: '#e0e0e0',
    },
    closeButton: {
        padding: 4,
    },
    modalTitle: {
        fontSize: 18,
        fontWeight: '600',
        color: '#333',
    },
    placeholder: {
        width: 32,
    },
    modalContent: {
        flex: 1,
    },
    modalImage: {
        width: '100%',
        height: 300,
    },
    modalInfo: {
        padding: 16,
    },
    modalNFTName: {
        fontSize: 20,
        fontWeight: 'bold',
        color: '#333',
        marginRight: 8,
    },
    modalDescription: {
        fontSize: 14,
        color: '#666',
        lineHeight: 20,
        marginBottom: 16,
    },
    modalMeta: {
        marginBottom: 16,
    },
    modalMetaRow: {
        flexDirection: 'row',
        alignItems: 'center',
        marginBottom: 8,
    },
    modalMetaLabel: {
        fontSize: 14,
        color: '#666',
        marginRight: 8,
        minWidth: 80,
    },
    modalMetaValue: {
        fontSize: 14,
        color: '#333',
        fontWeight: '500',
    },
    traitsContainer: {
        marginTop: 12,
    },
    traitsTitle: {
        fontSize: 14,
        fontWeight: '600',
        color: '#333',
        marginBottom: 8,
    },
    traitsList: {
        flexDirection: 'row',
        flexWrap: 'wrap',
        gap: 8,
    },
    traitChip: {
        backgroundColor: '#f0f0f0',
        paddingHorizontal: 8,
        paddingVertical: 4,
        borderRadius: 8,
    },
    traitType: {
        fontSize: 10,
        color: '#666',
        textTransform: 'uppercase',
    },
    traitValue: {
        fontSize: 12,
        color: '#333',
        fontWeight: '500',
    },
    modalActions: {
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'space-between',
        paddingTop: 16,
        borderTopWidth: 1,
        borderTopColor: '#e0e0e0',
    },
    modalFavoriteButton: {
        flexDirection: 'row',
        alignItems: 'center',
        padding: 8,
    },
    modalFavoriteText: {
        fontSize: 14,
        color: '#666',
        marginLeft: 4,
    },
    modalBuyButton: {
        backgroundColor: '#4CAF50',
        flexDirection: 'row',
        alignItems: 'center',
        paddingHorizontal: 16,
        paddingVertical: 12,
        borderRadius: 8,
    },
    modalBuyButtonText: {
        color: '#fff',
        fontSize: 14,
        fontWeight: '600',
        marginLeft: 4,
    },
    modalTransferButton: {
        backgroundColor: '#FF9800',
        flexDirection: 'row',
        alignItems: 'center',
        paddingHorizontal: 16,
        paddingVertical: 12,
        borderRadius: 8,
    },
    modalTransferButtonText: {
        color: '#fff',
        fontSize: 14,
        fontWeight: '600',
        marginLeft: 4,
    },
};

export default MobileMarketplace;
