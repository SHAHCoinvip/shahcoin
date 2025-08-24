# Shahcoin Plugin System

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Plugin System](https://img.shields.io/badge/Plugin%20System-Modular-green.svg)](https://shah.vip/plugins)

A modular plugin system for Shahcoin that allows developers to create mini-plugins for NFT galleries, staking extensions, UI widgets, and more.

## 🎯 Features

### 🧩 Modular Plugin Architecture
- **Plugin Discovery**: Auto-detect plugins in Qt Wallet and Web3 frontend
- **Dynamic Loading**: Load plugins via Shahcoin Plugin API
- **Sandboxed Execution**: Secure plugin execution environment
- **Version Management**: Plugin versioning and compatibility checking
- **Dependency Resolution**: Handle plugin dependencies automatically

### 🔌 Plugin Types
- **NFT Galleries**: Custom NFT display and management plugins
- **Staking Extensions**: Advanced staking interfaces and tools
- **UI Widgets**: Custom UI components and dashboards
- **Trading Tools**: Enhanced trading interfaces and analytics
- **Wallet Extensions**: Additional wallet functionality
- **DEX Plugins**: Custom DEX interfaces and tools

### 🛠 Development Tools
- **Plugin SDK**: Complete SDK for plugin development
- **Testing Sandbox**: Test plugins in isolated environment
- **Plugin Marketplace**: Discover and install community plugins
- **Documentation**: Comprehensive plugin development guides
- **Examples**: Sample plugins and templates

## 🚀 Quick Start

### 1. Create a Plugin
```javascript
// Basic plugin structure
const MyPlugin = {
  name: 'My Custom Plugin',
  version: '1.0.0',
  description: 'A custom plugin for Shahcoin',
  author: 'Your Name',
  dependencies: ['shahcoin-core'],
  
  // Plugin initialization
  initialize: async (context) => {
    console.log('Plugin initialized:', context);
    return true;
  },
  
  // Plugin cleanup
  destroy: async () => {
    console.log('Plugin destroyed');
  },
  
  // Plugin methods
  methods: {
    customFunction: async (params) => {
      return { success: true, data: params };
    }
  }
};

// Register plugin
ShahcoinPluginSystem.register(MyPlugin);
```

### 2. Load Plugin
```javascript
// Load plugin in application
const pluginManager = new ShahcoinPluginManager({
  apiKey: 'your-api-key',
  network: 'mainnet'
});

// Load plugin from file
await pluginManager.loadPlugin('./plugins/my-plugin.js');

// Load plugin from URL
await pluginManager.loadPlugin('https://example.com/plugin.js');

// Load plugin from marketplace
await pluginManager.installPlugin('nft-gallery', '1.0.0');
```

### 3. Use Plugin
```javascript
// Access plugin methods
const plugin = pluginManager.getPlugin('my-plugin');
const result = await plugin.methods.customFunction({ param: 'value' });

// Listen to plugin events
plugin.on('event', (data) => {
  console.log('Plugin event:', data);
});

// Call plugin UI components
const component = plugin.getComponent('MyComponent');
document.body.appendChild(component);
```

## 🧩 Plugin Architecture

### Plugin Structure
```javascript
// Complete plugin structure
const CompletePlugin = {
  // Basic information
  name: 'Complete Plugin',
  version: '1.0.0',
  description: 'A complete plugin example',
  author: 'Plugin Author',
  license: 'MIT',
  
  // Dependencies
  dependencies: {
    'shahcoin-core': '^1.0.0',
    'react': '^18.0.0'
  },
  
  // Plugin configuration
  config: {
    settings: {
      apiKey: { type: 'string', required: true },
      network: { type: 'string', default: 'mainnet' }
    },
    permissions: ['wallet.read', 'nft.read', 'swap.execute']
  },
  
  // Plugin lifecycle
  initialize: async (context) => {
    // Initialize plugin
    const { wallet, nft, swap } = context;
    
    // Set up event listeners
    wallet.on('connected', handleWalletConnected);
    nft.on('minted', handleNFTMinted);
    
    return { success: true };
  },
  
  destroy: async () => {
    // Cleanup resources
    console.log('Plugin destroyed');
  },
  
  // Plugin methods
  methods: {
    getData: async (params) => {
      // Plugin logic
      return { data: 'plugin data' };
    },
    
    executeAction: async (params) => {
      // Execute actions
      return { success: true };
    }
  },
  
  // UI components
  components: {
    MyComponent: {
      render: (props) => {
        return React.createElement('div', props, 'Plugin Component');
      },
      props: {
        title: { type: 'string', default: 'Default Title' },
        data: { type: 'object', required: true }
      }
    }
  },
  
  // Event handlers
  events: {
    onWalletConnected: (address) => {
      console.log('Wallet connected:', address);
    },
    
    onNFTMinted: (nft) => {
      console.log('NFT minted:', nft);
    }
  }
};
```

### Plugin Context
```javascript
// Plugin context provides access to Shahcoin APIs
const pluginContext = {
  // Wallet API
  wallet: {
    connect: async () => { /* ... */ },
    disconnect: async () => { /* ... */ },
    getBalance: async () => { /* ... */ },
    sendTransaction: async (params) => { /* ... */ },
    on: (event, callback) => { /* ... */ }
  },
  
  // NFT API
  nft: {
    getNFTs: async (address) => { /* ... */ },
    mintNFT: async (params) => { /* ... */ },
    transferNFT: async (params) => { /* ... */ },
    on: (event, callback) => { /* ... */ }
  },
  
  // Swap API
  swap: {
    getQuote: async (params) => { /* ... */ },
    executeSwap: async (params) => { /* ... */ },
    getLiquidity: async (pair) => { /* ... */ },
    on: (event, callback) => { /* ... */ }
  },
  
  // UI API
  ui: {
    showModal: (component, props) => { /* ... */ },
    hideModal: () => { /* ... */ },
    showNotification: (message, type) => { /* ... */ },
    updateTheme: (theme) => { /* ... */ }
  },
  
  // Storage API
  storage: {
    get: (key) => { /* ... */ },
    set: (key, value) => { /* ... */ },
    remove: (key) => { /* ... */ },
    clear: () => { /* ... */ }
  }
};
```

## 🔌 Plugin Examples

### NFT Gallery Plugin
```javascript
// NFT Gallery Plugin
const NFTGalleryPlugin = {
  name: 'NFT Gallery',
  version: '1.0.0',
  description: 'Custom NFT gallery with advanced features',
  author: 'NFT Developer',
  
  dependencies: {
    'shahcoin-core': '^1.0.0',
    'react': '^18.0.0'
  },
  
  config: {
    settings: {
      galleryStyle: { type: 'string', default: 'grid' },
      itemsPerPage: { type: 'number', default: 12 },
      showMetadata: { type: 'boolean', default: true }
    },
    permissions: ['nft.read', 'wallet.read']
  },
  
  initialize: async (context) => {
    const { nft, wallet, ui } = context;
    
    // Set up NFT gallery
    const gallery = new NFTGallery({
      style: context.config.galleryStyle,
      itemsPerPage: context.config.itemsPerPage
    });
    
    // Store in context
    context.gallery = gallery;
    
    return { success: true };
  },
  
  methods: {
    loadNFTs: async (address) => {
      const nfts = await context.nft.getNFTs(address);
      return context.gallery.displayNFTs(nfts);
    },
    
    filterByCollection: async (collectionAddress) => {
      const nfts = await context.nft.getNFTsByCollection(collectionAddress);
      return context.gallery.displayNFTs(nfts);
    },
    
    sortByPrice: async (order = 'asc') => {
      return context.gallery.sortByPrice(order);
    }
  },
  
  components: {
    NFTGallery: {
      render: (props) => {
        return React.createElement('div', {
          className: 'nft-gallery',
          ...props
        }, [
          React.createElement('h2', {}, 'NFT Gallery'),
          React.createElement('div', {
            className: 'gallery-grid'
          }, props.nfts.map(nft => 
            React.createElement('div', {
              key: nft.id,
              className: 'nft-item'
            }, [
              React.createElement('img', {
                src: nft.image,
                alt: nft.name
              }),
              React.createElement('h3', {}, nft.name),
              React.createElement('p', {}, nft.description)
            ])
          ))
        ]);
      },
      props: {
        nfts: { type: 'array', required: true },
        style: { type: 'string', default: 'grid' }
      }
    }
  }
};
```

### Staking Extension Plugin
```javascript
// Staking Extension Plugin
const StakingExtensionPlugin = {
  name: 'Staking Extension',
  version: '1.0.0',
  description: 'Advanced staking interface with analytics',
  author: 'Staking Developer',
  
  dependencies: {
    'shahcoin-core': '^1.0.0',
    'chart.js': '^3.0.0'
  },
  
  config: {
    settings: {
      autoCompound: { type: 'boolean', default: false },
      minStakeAmount: { type: 'number', default: 100 },
      showAnalytics: { type: 'boolean', default: true }
    },
    permissions: ['staking.read', 'staking.write', 'wallet.read']
  },
  
  initialize: async (context) => {
    const { staking, wallet, ui } = context;
    
    // Initialize staking dashboard
    const dashboard = new StakingDashboard({
      autoCompound: context.config.autoCompound,
      minStakeAmount: context.config.minStakeAmount
    });
    
    context.dashboard = dashboard;
    
    return { success: true };
  },
  
  methods: {
    getStakingInfo: async (address) => {
      const info = await context.staking.getInfo(address);
      return context.dashboard.displayInfo(info);
    },
    
    stake: async (amount) => {
      const result = await context.staking.stake(amount);
      context.ui.showNotification('Staking successful!', 'success');
      return result;
    },
    
    unstake: async (amount) => {
      const result = await context.staking.unstake(amount);
      context.ui.showNotification('Unstaking successful!', 'success');
      return result;
    },
    
    claimRewards: async () => {
      const result = await context.staking.claimRewards();
      context.ui.showNotification('Rewards claimed!', 'success');
      return result;
    }
  },
  
  components: {
    StakingDashboard: {
      render: (props) => {
        return React.createElement('div', {
          className: 'staking-dashboard'
        }, [
          React.createElement('h2', {}, 'Staking Dashboard'),
          React.createElement('div', {
            className: 'staking-stats'
          }, [
            React.createElement('div', {
              className: 'stat-item'
            }, [
              React.createElement('h3', {}, 'Total Staked'),
              React.createElement('p', {}, `${props.totalStaked} SHAH`)
            ]),
            React.createElement('div', {
              className: 'stat-item'
            }, [
              React.createElement('h3', {}, 'Rewards Earned'),
              React.createElement('p', {}, `${props.rewardsEarned} SHAH`)
            ]),
            React.createElement('div', {
              className: 'stat-item'
            }, [
              React.createElement('h3', {}, 'APY'),
              React.createElement('p', {}, `${props.apy}%`)
            ])
          ]),
          React.createElement('div', {
            className: 'staking-actions'
          }, [
            React.createElement('button', {
              onClick: props.onStake
            }, 'Stake'),
            React.createElement('button', {
              onClick: props.onUnstake
            }, 'Unstake'),
            React.createElement('button', {
              onClick: props.onClaim
            }, 'Claim Rewards')
          ])
        ]);
      },
      props: {
        totalStaked: { type: 'number', required: true },
        rewardsEarned: { type: 'number', required: true },
        apy: { type: 'number', required: true },
        onStake: { type: 'function', required: true },
        onUnstake: { type: 'function', required: true },
        onClaim: { type: 'function', required: true }
      }
    }
  }
};
```

## 🛠 Plugin Development

### Plugin SDK
```javascript
// Plugin SDK for development
class ShahcoinPluginSDK {
  constructor(config) {
    this.config = config;
    this.plugins = new Map();
    this.context = this.createContext();
  }
  
  // Create plugin context
  createContext() {
    return {
      wallet: new WalletAPI(),
      nft: new NFTAPI(),
      swap: new SwapAPI(),
      ui: new UIAPI(),
      storage: new StorageAPI(),
      config: this.config
    };
  }
  
  // Register plugin
  register(plugin) {
    // Validate plugin
    this.validatePlugin(plugin);
    
    // Store plugin
    this.plugins.set(plugin.name, plugin);
    
    // Initialize plugin
    this.initializePlugin(plugin);
    
    return plugin;
  }
  
  // Validate plugin structure
  validatePlugin(plugin) {
    const required = ['name', 'version', 'description', 'author'];
    
    for (const field of required) {
      if (!plugin[field]) {
        throw new Error(`Plugin missing required field: ${field}`);
      }
    }
    
    // Check dependencies
    if (plugin.dependencies) {
      this.checkDependencies(plugin.dependencies);
    }
    
    return true;
  }
  
  // Initialize plugin
  async initializePlugin(plugin) {
    try {
      if (plugin.initialize) {
        await plugin.initialize(this.context);
      }
      
      console.log(`Plugin ${plugin.name} initialized successfully`);
    } catch (error) {
      console.error(`Failed to initialize plugin ${plugin.name}:`, error);
      throw error;
    }
  }
  
  // Get plugin
  getPlugin(name) {
    return this.plugins.get(name);
  }
  
  // Call plugin method
  async callPluginMethod(pluginName, methodName, params) {
    const plugin = this.getPlugin(pluginName);
    
    if (!plugin) {
      throw new Error(`Plugin not found: ${pluginName}`);
    }
    
    if (!plugin.methods || !plugin.methods[methodName]) {
      throw new Error(`Method not found: ${methodName}`);
    }
    
    return await plugin.methods[methodName](params);
  }
  
  // Get plugin component
  getPluginComponent(pluginName, componentName) {
    const plugin = this.getPlugin(pluginName);
    
    if (!plugin) {
      throw new Error(`Plugin not found: ${pluginName}`);
    }
    
    if (!plugin.components || !plugin.components[componentName]) {
      throw new Error(`Component not found: ${componentName}`);
    }
    
    return plugin.components[componentName];
  }
}

// Usage
const sdk = new ShahcoinPluginSDK({
  apiKey: 'your-api-key',
  network: 'mainnet'
});

sdk.register(MyPlugin);
```

### Testing Sandbox
```javascript
// Plugin testing sandbox
class PluginSandbox {
  constructor() {
    this.sandbox = this.createSandbox();
    this.plugins = new Map();
  }
  
  // Create isolated sandbox environment
  createSandbox() {
    return {
      // Mock APIs for testing
      wallet: {
        connect: jest.fn(),
        disconnect: jest.fn(),
        getBalance: jest.fn(),
        sendTransaction: jest.fn(),
        on: jest.fn()
      },
      
      nft: {
        getNFTs: jest.fn(),
        mintNFT: jest.fn(),
        transferNFT: jest.fn(),
        on: jest.fn()
      },
      
      swap: {
        getQuote: jest.fn(),
        executeSwap: jest.fn(),
        getLiquidity: jest.fn(),
        on: jest.fn()
      },
      
      ui: {
        showModal: jest.fn(),
        hideModal: jest.fn(),
        showNotification: jest.fn(),
        updateTheme: jest.fn()
      },
      
      storage: {
        get: jest.fn(),
        set: jest.fn(),
        remove: jest.fn(),
        clear: jest.fn()
      }
    };
  }
  
  // Load plugin in sandbox
  async loadPlugin(plugin) {
    try {
      // Initialize plugin in sandbox
      if (plugin.initialize) {
        await plugin.initialize(this.sandbox);
      }
      
      this.plugins.set(plugin.name, plugin);
      
      return { success: true, plugin };
    } catch (error) {
      return { success: false, error: error.message };
    }
  }
  
  // Test plugin method
  async testMethod(pluginName, methodName, params) {
    const plugin = this.plugins.get(pluginName);
    
    if (!plugin || !plugin.methods || !plugin.methods[methodName]) {
      throw new Error(`Method not found: ${methodName}`);
    }
    
    return await plugin.methods[methodName](params);
  }
  
  // Test plugin component
  testComponent(pluginName, componentName, props) {
    const plugin = this.plugins.get(pluginName);
    
    if (!plugin || !plugin.components || !plugin.components[componentName]) {
      throw new Error(`Component not found: ${componentName}`);
    }
    
    const component = plugin.components[componentName];
    return component.render(props);
  }
  
  // Get sandbox state
  getState() {
    return {
      plugins: Array.from(this.plugins.keys()),
      mockCalls: this.getMockCalls()
    };
  }
  
  // Get mock function calls
  getMockCalls() {
    const calls = {};
    
    Object.keys(this.sandbox).forEach(api => {
      calls[api] = {};
      Object.keys(this.sandbox[api]).forEach(method => {
        calls[api][method] = this.sandbox[api][method].mock.calls;
      });
    });
    
    return calls;
  }
}

// Usage
const sandbox = new PluginSandbox();
await sandbox.loadPlugin(MyPlugin);
const result = await sandbox.testMethod('My Plugin', 'customFunction', { test: true });
```

## 📦 Plugin Marketplace

### Plugin Discovery
```javascript
// Plugin marketplace API
class PluginMarketplace {
  constructor() {
    this.baseUrl = 'https://plugins.shah.vip/api';
  }
  
  // Search plugins
  async searchPlugins(query, filters = {}) {
    const response = await fetch(`${this.baseUrl}/plugins/search`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ query, filters })
    });
    
    return response.json();
  }
  
  // Get plugin details
  async getPluginDetails(pluginId) {
    const response = await fetch(`${this.baseUrl}/plugins/${pluginId}`);
    return response.json();
  }
  
  // Install plugin
  async installPlugin(pluginId, version = 'latest') {
    const response = await fetch(`${this.baseUrl}/plugins/${pluginId}/install`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ version })
    });
    
    return response.json();
  }
  
  // Rate plugin
  async ratePlugin(pluginId, rating, review) {
    const response = await fetch(`${this.baseUrl}/plugins/${pluginId}/rate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ rating, review })
    });
    
    return response.json();
  }
  
  // Get popular plugins
  async getPopularPlugins(limit = 10) {
    const response = await fetch(`${this.baseUrl}/plugins/popular?limit=${limit}`);
    return response.json();
  }
  
  // Get recent plugins
  async getRecentPlugins(limit = 10) {
    const response = await fetch(`${this.baseUrl}/plugins/recent?limit=${limit}`);
    return response.json();
  }
}

// Usage
const marketplace = new PluginMarketplace();

// Search for NFT plugins
const nftPlugins = await marketplace.searchPlugins('nft', {
  category: 'gallery',
  rating: 4.5
});

// Install plugin
const result = await marketplace.installPlugin('nft-gallery', '1.0.0');
```

## 🛠 Development

### Local Development
```bash
# Install dependencies
npm install

# Start development server
npm run dev

# Start plugin sandbox
npm run sandbox

# Build plugin
npm run build

# Test plugin
npm test

# Publish plugin
npm run publish
```

### Environment Variables
```bash
# Required environment variables
SHAHCOIN_API_KEY=your_api_key
PLUGIN_MARKETPLACE_URL=https://plugins.shah.vip
PLUGIN_STORAGE_PATH=./plugins
SANDBOX_ENABLED=true
```

## 📞 Support

- 📖 **Documentation**: [https://shah.vip/dev/plugins](https://shah.vip/dev/plugins)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin-core/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## 📄 License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
