# Shahcoin Developer Analytics Dashboard

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Analytics](https://img.shields.io/badge/Analytics-Real%20Time-green.svg)](https://shah.vip/analytics)

A comprehensive analytics dashboard for Shahcoin developers to track token creation, NFT mints, ShahSwap usage, and blockchain activity.

## 🎯 Features

### 📊 Real-time Analytics
- **Token Creation Events**: Track new token deployments and configurations
- **NFT Mints & Metadata**: Monitor NFT collection growth and metadata updates
- **ShahSwap Usage Volume**: Real-time DEX trading volume and liquidity metrics
- **Wallet & Contract Activity**: User engagement and contract interactions
- **Network Performance**: Block time, transaction throughput, and gas metrics

### 📈 Visual Charts & Graphs
- **Interactive Charts**: Line charts, bar graphs, pie charts, and heatmaps
- **Time-based Filtering**: Filter data by hour, day, week, month, or custom ranges
- **Project-specific Views**: Analytics filtered by specific projects or contracts
- **User Segmentation**: Analyze different user groups and behaviors
- **Comparative Analysis**: Compare metrics across different time periods

### 📋 Data Export & Integration
- **CSV Export**: Export analytics data for external analysis
- **API Integration**: RESTful API for programmatic access
- **Webhook Support**: Real-time notifications for important events
- **Dashboard Embedding**: Embed analytics in external applications
- **Custom Reports**: Generate custom reports and scheduled exports

## 🚀 Quick Start

### 1. Access Dashboard
```javascript
// Access the analytics dashboard
const dashboard = new ShahcoinAnalytics({
  apiKey: 'your-api-key',
  projectId: 'your-project-id'
});

// Initialize dashboard
await dashboard.initialize();
```

### 2. View Analytics
```javascript
// Get token creation analytics
const tokenAnalytics = await dashboard.getTokenAnalytics({
  timeRange: '7d',
  projectId: 'my-project'
});

// Get NFT mint analytics
const nftAnalytics = await dashboard.getNFTAnalytics({
  timeRange: '30d',
  collectionId: 'my-nft-collection'
});

// Get ShahSwap volume analytics
const swapAnalytics = await dashboard.getSwapAnalytics({
  timeRange: '24h',
  tokenPair: 'SHAH/USDT'
});
```

### 3. Export Data
```javascript
// Export analytics data
const csvData = await dashboard.exportAnalytics({
  type: 'token_creation',
  timeRange: '30d',
  format: 'csv'
});

// Download CSV file
downloadCSV(csvData, 'token_analytics.csv');
```

## 📊 Analytics Categories

### Token Analytics
```javascript
// Token creation metrics
const tokenMetrics = {
  totalTokens: 1250,
  newTokens24h: 15,
  averageSupply: 1000000,
  mostPopularDecimals: 18,
  topCreators: [
    { address: '0x123...', tokens: 25 },
    { address: '0x456...', tokens: 18 }
  ],
  supplyDistribution: {
    '0-100k': 45,
    '100k-1M': 30,
    '1M-10M': 20,
    '10M+': 5
  }
};

// Token activity metrics
const tokenActivity = {
  totalTransfers: 50000,
  uniqueHolders: 2500,
  averageHoldTime: '7.5 days',
  topTokens: [
    { symbol: 'SHAH', volume: 1000000 },
    { symbol: 'GAME', volume: 500000 },
    { symbol: 'ART', volume: 250000 }
  ]
};
```

### NFT Analytics
```javascript
// NFT collection metrics
const nftMetrics = {
  totalCollections: 150,
  totalNFTs: 25000,
  newNFTs24h: 500,
  averagePrice: 0.5, // SHAH
  floorPrices: {
    'Collection A': 2.5,
    'Collection B': 1.8,
    'Collection C': 3.2
  },
  topCollections: [
    { name: 'ShahPunks', volume: 10000 },
    { name: 'ShahArt', volume: 8000 },
    { name: 'ShahGames', volume: 6000 }
  ]
};

// NFT trading metrics
const nftTrading = {
  totalSales: 15000,
  totalVolume: 75000, // SHAH
  averageSalePrice: 5.0,
  uniqueTraders: 1200,
  tradingVolume24h: 5000
};
```

### ShahSwap Analytics
```javascript
// DEX trading metrics
const swapMetrics = {
  totalVolume24h: 500000, // SHAH
  totalTrades24h: 2500,
  uniqueTraders24h: 800,
  topPairs: [
    { pair: 'SHAH/USDT', volume: 200000 },
    { pair: 'SHAH/ETH', volume: 150000 },
    { pair: 'GAME/SHAH', volume: 100000 }
  ],
  liquidityMetrics: {
    totalLiquidity: 2000000,
    averageAPY: 15.5,
    topLiquidityProviders: [
      { address: '0x123...', liquidity: 100000 },
      { address: '0x456...', liquidity: 80000 }
    ]
  }
};
```

### Network Analytics
```javascript
// Blockchain performance metrics
const networkMetrics = {
  blockTime: 2.5, // seconds
  transactionsPerSecond: 100,
  totalTransactions: 5000000,
  activeAddresses24h: 5000,
  gasMetrics: {
    averageGasPrice: 20, // gwei
    gasUsed24h: 50000000,
    gasLimitUtilization: 75 // percentage
  },
  stakingMetrics: {
    totalStaked: 10000000, // SHAH
    stakingAPY: 12.5,
    activeStakers: 2500,
    averageStakeTime: '30 days'
  }
};
```

## 🎨 Dashboard Components

### Analytics Dashboard
```javascript
// React component for analytics dashboard
import React, { useState, useEffect } from 'react';
import { LineChart, BarChart, PieChart } from 'recharts';

const AnalyticsDashboard = () => {
  const [timeRange, setTimeRange] = useState('7d');
  const [projectId, setProjectId] = useState('all');
  const [analytics, setAnalytics] = useState({});

  useEffect(() => {
    loadAnalytics();
  }, [timeRange, projectId]);

  const loadAnalytics = async () => {
    const data = await dashboard.getAnalytics({
      timeRange,
      projectId
    });
    setAnalytics(data);
  };

  return (
    <div className="analytics-dashboard">
      <div className="dashboard-header">
        <h1>Shahcoin Analytics Dashboard</h1>
        <div className="filters">
          <select value={timeRange} onChange={(e) => setTimeRange(e.target.value)}>
            <option value="1h">Last Hour</option>
            <option value="24h">Last 24 Hours</option>
            <option value="7d">Last 7 Days</option>
            <option value="30d">Last 30 Days</option>
            <option value="90d">Last 90 Days</option>
          </select>
          <select value={projectId} onChange={(e) => setProjectId(e.target.value)}>
            <option value="all">All Projects</option>
            <option value="my-project">My Project</option>
          </select>
        </div>
      </div>

      <div className="metrics-grid">
        <MetricCard
          title="Token Creation"
          value={analytics.tokenCreation?.total || 0}
          change={analytics.tokenCreation?.change24h || 0}
          icon="token"
        />
        <MetricCard
          title="NFT Mints"
          value={analytics.nftMints?.total || 0}
          change={analytics.nftMints?.change24h || 0}
          icon="nft"
        />
        <MetricCard
          title="Swap Volume"
          value={`${analytics.swapVolume?.total || 0} SHAH`}
          change={analytics.swapVolume?.change24h || 0}
          icon="swap"
        />
        <MetricCard
          title="Active Users"
          value={analytics.activeUsers?.total || 0}
          change={analytics.activeUsers?.change24h || 0}
          icon="users"
        />
      </div>

      <div className="charts-section">
        <div className="chart-container">
          <h3>Token Creation Trend</h3>
          <LineChart data={analytics.tokenCreation?.trend || []}>
            {/* Chart configuration */}
          </LineChart>
        </div>
        
        <div className="chart-container">
          <h3>NFT Sales Volume</h3>
          <BarChart data={analytics.nftSales?.volume || []}>
            {/* Chart configuration */}
          </BarChart>
        </div>
        
        <div className="chart-container">
          <h3>Swap Volume by Pair</h3>
          <PieChart data={analytics.swapVolume?.byPair || []}>
            {/* Chart configuration */}
          </PieChart>
        </div>
      </div>

      <div className="export-section">
        <button onClick={() => exportAnalytics('csv')}>
          Export to CSV
        </button>
        <button onClick={() => exportAnalytics('json')}>
          Export to JSON
        </button>
      </div>
    </div>
  );
};
```

### Real-time Updates
```javascript
// WebSocket connection for real-time updates
const setupRealTimeUpdates = () => {
  const ws = new WebSocket('wss://analytics.shah.vip/ws');
  
  ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    
    switch (data.type) {
      case 'token_created':
        updateTokenMetrics(data);
        break;
      case 'nft_minted':
        updateNFTMetrics(data);
        break;
      case 'swap_executed':
        updateSwapMetrics(data);
        break;
      case 'transaction_confirmed':
        updateNetworkMetrics(data);
        break;
    }
  };
  
  return ws;
};

// Update metrics in real-time
const updateTokenMetrics = (data) => {
  setAnalytics(prev => ({
    ...prev,
    tokenCreation: {
      ...prev.tokenCreation,
      total: prev.tokenCreation.total + 1,
      recent: [data, ...prev.tokenCreation.recent.slice(0, 9)]
    }
  }));
};
```

## 📊 API Reference

### Analytics API
```javascript
// Get comprehensive analytics
const getAnalytics = async (params) => {
  const response = await fetch('/api/analytics', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      timeRange: params.timeRange,
      projectId: params.projectId,
      metrics: params.metrics || 'all'
    })
  });
  
  return response.json();
};

// Get specific metric analytics
const getTokenAnalytics = async (params) => {
  return await getAnalytics({
    ...params,
    metrics: 'token_creation'
  });
};

const getNFTAnalytics = async (params) => {
  return await getAnalytics({
    ...params,
    metrics: 'nft_mints'
  });
};

const getSwapAnalytics = async (params) => {
  return await getAnalytics({
    ...params,
    metrics: 'swap_volume'
  });
};
```

### Export API
```javascript
// Export analytics data
const exportAnalytics = async (params) => {
  const response = await fetch('/api/analytics/export', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      type: params.type,
      timeRange: params.timeRange,
      format: params.format,
      filters: params.filters
    })
  });
  
  if (params.format === 'csv') {
    const blob = await response.blob();
    downloadFile(blob, `analytics_${params.type}_${params.timeRange}.csv`);
  } else {
    return response.json();
  }
};
```

## 🔗 Integration Examples

### Embed in External Application
```javascript
// Embed analytics dashboard
const embedAnalytics = (containerId, config) => {
  const iframe = document.createElement('iframe');
  iframe.src = `https://analytics.shah.vip/embed?apiKey=${config.apiKey}&projectId=${config.projectId}`;
  iframe.width = '100%';
  iframe.height = '600px';
  iframe.frameBorder = '0';
  
  document.getElementById(containerId).appendChild(iframe);
};

// Usage
embedAnalytics('analytics-container', {
  apiKey: 'your-api-key',
  projectId: 'my-project'
});
```

### Webhook Integration
```javascript
// Set up webhook for real-time notifications
const setupWebhook = async (webhookUrl, events) => {
  const response = await fetch('/api/webhooks', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      url: webhookUrl,
      events: events,
      secret: generateSecret()
    })
  });
  
  return response.json();
};

// Webhook events
const webhookEvents = [
  'token.created',
  'nft.minted',
  'swap.executed',
  'user.registered',
  'project.created'
];
```

## 📈 Custom Reports

### Scheduled Reports
```javascript
// Create scheduled report
const createScheduledReport = async (config) => {
  const response = await fetch('/api/reports/schedule', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${apiKey}`
    },
    body: JSON.stringify({
      name: config.name,
      schedule: config.schedule, // daily, weekly, monthly
      metrics: config.metrics,
      recipients: config.recipients,
      format: config.format // email, csv, pdf
    })
  });
  
  return response.json();
};

// Example scheduled report
const weeklyReport = {
  name: 'Weekly Project Analytics',
  schedule: 'weekly',
  metrics: ['token_creation', 'nft_mints', 'swap_volume'],
  recipients: ['analytics@myproject.com'],
  format: 'email'
};
```

### Custom Metrics
```javascript
// Define custom metrics
const customMetrics = {
  userRetention: {
    calculation: 'returning_users / total_users',
    timeRange: '30d',
    description: 'Percentage of users who return within 30 days'
  },
  averageTransactionValue: {
    calculation: 'total_volume / total_transactions',
    timeRange: '7d',
    description: 'Average value per transaction'
  },
  projectGrowth: {
    calculation: 'current_users - previous_users',
    timeRange: '7d',
    description: 'User growth over the last 7 days'
  }
};
```

## 🛠 Development

### Local Development
```bash
# Install dependencies
npm install

# Start development server
npm run dev

# Build for production
npm run build

# Run tests
npm test
```

### Environment Variables
```bash
# Required environment variables
SHAHCOIN_API_KEY=your_api_key
SHAHCOIN_RPC_URL=https://rpc.shah.vip
DATABASE_URL=your_database_url
REDIS_URL=your_redis_url
WEBHOOK_SECRET=your_webhook_secret
```

## 📞 Support

- 📖 **Documentation**: [https://shah.vip/dev/analytics](https://shah.vip/dev/analytics)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin-core/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## 📄 License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
