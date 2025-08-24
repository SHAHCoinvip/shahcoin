# Shahcoin NFT Marketplace - Mobile Responsive Implementation

A comprehensive, mobile-responsive NFT marketplace for the Shahcoin blockchain ecosystem. This implementation provides both React Native (mobile) and React (web) components with automatic platform detection.

## Features

### 🎨 **Core Marketplace Features**
- **Verified NFT Display**: Show only quality, verified NFTs by default
- **Multiple View Modes**: Verified, All NFTs, My NFTs, Favorites
- **Advanced Search**: Search by name, description, creator, category
- **Smart Filtering**: Category, creator, price range, traits, dates
- **Real-time Analytics**: Volume, transactions, top creators, categories
- **Creator Profiles**: Trust scores, badges, transaction history

### 📱 **Mobile Responsive Design**
- **Automatic Platform Detection**: React Native for mobile, React for web
- **Touch-Friendly Interface**: Optimized for mobile interactions
- **Responsive Grid Layout**: Adapts to screen size (1-3 columns)
- **Pull-to-Refresh**: Native mobile refresh experience
- **Modal Details**: Full-screen NFT detail views
- **Gesture Support**: Swipe, tap, and scroll optimizations

### 🔧 **Advanced Features**
- **Bulk Operations**: Batch listing, price updates, delisting
- **Advanced Filters**: Multi-tab filtering with presets
- **Search Autocomplete**: Smart search suggestions
- **Virtual Scrolling**: Performance optimization for large lists
- **Dark Mode Support**: Automatic theme detection
- **Offline Support**: Cached data and graceful error handling

### 💰 **Transaction Features**
- **Purchase Flow**: Secure NFT buying with confirmation
- **Transfer System**: Send NFTs to other addresses
- **Escrow Protection**: High-value transaction protection
- **Payment Methods**: SHAH native, SHAHtoken, Stripe integration
- **Transaction History**: Complete audit trail

## Installation

```bash
npm install @shahcoin/marketplace-ui
```

## Quick Start

### Basic Usage

```jsx
import ShahcoinMarketplace from '@shahcoin/marketplace-ui';

function App() {
  const config = {
    walletAddress: 'your-wallet-address',
    apiBaseUrl: 'https://api.shah.vip/marketplace',
    onNFTPurchase: async (nft) => {
      // Handle NFT purchase
      console.log('Purchasing NFT:', nft);
    },
    onNFTTransfer: async (nft) => {
      // Handle NFT transfer
      console.log('Transferring NFT:', nft);
    }
  };

  return <ShahcoinMarketplace {...config} />;
}
```

### Advanced Usage with Hooks

```jsx
import { useMarketplace, MarketplaceConfig } from '@shahcoin/marketplace-ui';

function AdvancedMarketplace() {
  const config = MarketplaceConfig.create({
    walletAddress: 'your-wallet-address',
    walletProvider: yourWalletProvider,
    enableAnalytics: true,
    refreshInterval: 30000
  });

  const {
    nfts,
    loading,
    error,
    analytics,
    purchaseNFT,
    transferNFT,
    toggleFavorite
  } = useMarketplace(config);

  if (loading) return <div>Loading...</div>;
  if (error) return <div>Error: {error}</div>;

  return (
    <div>
      <h1>Marketplace Analytics</h1>
      <p>24h Volume: {analytics?.totalVolume24h}</p>
      
      <div className="nft-grid">
        {nfts.map(nft => (
          <NFTCard 
            key={nft.id} 
            nft={nft}
            onPurchase={() => purchaseNFT(nft)}
            onTransfer={() => transferNFT(nft, 'recipient-address')}
            onFavorite={() => toggleFavorite(nft)}
          />
        ))}
      </div>
    </div>
  );
}
```

## Component Architecture

### Platform Detection

The marketplace automatically detects the platform and renders the appropriate component:

```jsx
// Automatically uses MobileMarketplace in React Native
// Automatically uses WebMarketplace in React/Web
const ShahcoinMarketplace = (props) => {
  const isReactNative = typeof navigator === 'undefined' || 
                       (navigator.product && navigator.product === 'ReactNative');

  return isReactNative ? 
    <MobileMarketplace {...props} /> : 
    <WebMarketplace {...props} />;
};
```

### MobileMarketplace (React Native)

Optimized for mobile devices with:
- Touch-friendly UI elements
- Native scrolling and gestures
- Mobile-optimized modals
- Responsive grid layouts
- Pull-to-refresh functionality

### WebMarketplace (React)

Optimized for web browsers with:
- Desktop and tablet layouts
- Hover effects and animations
- Advanced filtering UI
- Bulk operations interface
- Keyboard navigation support

## API Integration

### Marketplace API

```jsx
import { marketplaceAPI } from '@shahcoin/marketplace-ui';

// Fetch verified NFTs
const verifiedNFTs = await marketplaceAPI.getVerifiedNFTs();

// Fetch all NFTs
const allNFTs = await marketplaceAPI.getAllNFTs();

// Fetch user's NFTs
const myNFTs = await marketplaceAPI.getMyNFTs(walletAddress);

// Fetch favorited NFTs
const favorites = await marketplaceAPI.getFavoritedNFTs(walletAddress);

// Get creator profile
const profile = await marketplaceAPI.getCreatorProfile(creatorAddress);

// Get analytics
const analytics = await marketplaceAPI.getAnalytics();

// Submit listing
await marketplaceAPI.submitListing(listingData);

// Record purchase
await marketplaceAPI.recordPurchase(purchaseData);

// Toggle favorite
await marketplaceAPI.toggleFavorite(nftId, walletAddress);
```

### Utilities

```jsx
import { MarketplaceUtils } from '@shahcoin/marketplace-ui';

// Format price
const formattedPrice = MarketplaceUtils.formatPrice(100000000); // "1.00000000 SHAH"

// Format address
const shortAddress = MarketplaceUtils.formatAddress('long-wallet-address'); // "long...ress"

// Validate NFT data
MarketplaceUtils.validateNFTData(nft);

// Calculate rarity score
const rarity = MarketplaceUtils.calculateRarityScore(nft);

// Sort NFTs
const sortedNFTs = MarketplaceUtils.sortNFTs(nfts, 'price', 'desc');

// Filter NFTs
const filteredNFTs = MarketplaceUtils.filterNFTs(nfts, {
  search: 'search term',
  category: 'Art',
  minPrice: 1000000,
  verifiedOnly: true
});

// Generate analytics
const analytics = MarketplaceUtils.generateAnalytics(nfts, transactions);
```

## Configuration

### Default Configuration

```jsx
const defaultConfig = {
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
};
```

### Custom Configuration

```jsx
import { MarketplaceConfig } from '@shahcoin/marketplace-ui';

const config = MarketplaceConfig.create({
  walletAddress: 'your-wallet-address',
  walletProvider: yourWalletProvider,
  apiBaseUrl: 'https://your-api.com/marketplace',
  refreshInterval: 60000, // 1 minute
  enableAnalytics: false,
  enableBulkOperations: false
});
```

## Styling

### CSS Customization

The web marketplace uses CSS custom properties for easy theming:

```css
:root {
  --marketplace-primary: #2196F3;
  --marketplace-secondary: #4CAF50;
  --marketplace-accent: #FF9800;
  --marketplace-background: #f8f9fa;
  --marketplace-surface: #ffffff;
  --marketplace-text: #333333;
  --marketplace-text-secondary: #6c757d;
}
```

### Dark Mode

Automatic dark mode support with `prefers-color-scheme`:

```css
@media (prefers-color-scheme: dark) {
  :root {
    --marketplace-background: #1a1a1a;
    --marketplace-surface: #2d2d2d;
    --marketplace-text: #e0e0e0;
    --marketplace-text-secondary: #b0b0b0;
  }
}
```

## Performance Optimizations

### Virtual Scrolling

For large NFT collections, virtual scrolling is implemented:

```jsx
// Automatically enabled for lists > 100 items
const VirtualScrollList = ({ items, itemHeight, renderItem }) => {
  const [visibleRange, setVisibleRange] = useState({ start: 0, end: 20 });
  
  // Only render visible items
  const visibleItems = items.slice(visibleRange.start, visibleRange.end);
  
  return (
    <div style={{ height: items.length * itemHeight }}>
      {visibleItems.map(renderItem)}
    </div>
  );
};
```

### Lazy Loading

Images and data are loaded on-demand:

```jsx
// Lazy image loading
const LazyImage = ({ src, alt }) => {
  const [loaded, setLoaded] = useState(false);
  
  return (
    <img 
      src={loaded ? src : placeholder}
      alt={alt}
      onLoad={() => setLoaded(true)}
      loading="lazy"
    />
  );
};
```

### Debounced Search

Search input is debounced to prevent excessive API calls:

```jsx
const useDebouncedSearch = (callback, delay = 300) => {
  const [searchTerm, setSearchTerm] = useState('');
  
  useEffect(() => {
    const timer = setTimeout(() => {
      callback(searchTerm);
    }, delay);
    
    return () => clearTimeout(timer);
  }, [searchTerm, callback, delay]);
  
  return [searchTerm, setSearchTerm];
};
```

## Error Handling

### Graceful Error States

```jsx
const ErrorBoundary = ({ children }) => {
  const [hasError, setHasError] = useState(false);
  
  if (hasError) {
    return (
      <div className="error-container">
        <h2>Something went wrong</h2>
        <p>Please try refreshing the page</p>
        <button onClick={() => window.location.reload()}>
          Refresh
        </button>
      </div>
    );
  }
  
  return children;
};
```

### Network Error Handling

```jsx
const useNetworkStatus = () => {
  const [isOnline, setIsOnline] = useState(navigator.onLine);
  
  useEffect(() => {
    const handleOnline = () => setIsOnline(true);
    const handleOffline = () => setIsOnline(false);
    
    window.addEventListener('online', handleOnline);
    window.addEventListener('offline', handleOffline);
    
    return () => {
      window.removeEventListener('online', handleOnline);
      window.removeEventListener('offline', handleOffline);
    };
  }, []);
  
  return isOnline;
};
```

## Testing

### Component Testing

```jsx
import { render, screen, fireEvent } from '@testing-library/react';
import ShahcoinMarketplace from '@shahcoin/marketplace-ui';

test('renders marketplace with NFTs', async () => {
  const mockNFTs = [
    {
      id: '1',
      name: 'Test NFT',
      description: 'Test Description',
      imageUrl: 'test.jpg',
      creator: 'creator-address',
      owner: 'owner-address',
      listPriceSatoshis: 100000000
    }
  ];
  
  render(<ShahcoinMarketplace nfts={mockNFTs} />);
  
  expect(screen.getByText('Test NFT')).toBeInTheDocument();
  expect(screen.getByText('1.00000000 SHAH')).toBeInTheDocument();
});
```

### API Testing

```jsx
import { marketplaceAPI } from '@shahcoin/marketplace-ui';

test('fetches verified NFTs', async () => {
  const nfts = await marketplaceAPI.getVerifiedNFTs();
  
  expect(Array.isArray(nfts)).toBe(true);
  expect(nfts.every(nft => nft.isVerified)).toBe(true);
});
```

## Deployment

### Web Deployment

```bash
# Build for production
npm run build

# Deploy to Vercel
vercel --prod

# Deploy to Netlify
netlify deploy --prod
```

### Mobile Deployment

```bash
# Build for iOS
npx react-native run-ios --configuration Release

# Build for Android
npx react-native run-android --variant release
```

## Browser Support

- **Chrome**: 80+
- **Firefox**: 75+
- **Safari**: 13+
- **Edge**: 80+
- **Mobile Safari**: 13+
- **Chrome Mobile**: 80+

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

MIT License - see LICENSE file for details

## Support

- **Documentation**: [docs.shah.vip/marketplace](https://docs.shah.vip/marketplace)
- **Issues**: [GitHub Issues](https://github.com/shahcoin/marketplace-ui/issues)
- **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

---

**Built with ❤️ for the Shahcoin community**
