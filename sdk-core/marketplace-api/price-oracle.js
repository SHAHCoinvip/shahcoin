// Shahcoin Price Oracle for Marketplace
// Provides real-time SHAH price data for USD conversions

class PriceOracle {
  constructor(config = {}) {
    this.apiBaseUrl = config.apiBaseUrl || 'https://api.shah.vip';
    this.cacheDuration = config.cacheDuration || 30000; // 30 seconds
    this.fallbackPrice = config.fallbackPrice || 0.001; // $0.001 USD per SHAH fallback
    
    this.priceCache = {
      price: null,
      timestamp: 0,
      source: null
    };
    
    this.priceUpdateCallbacks = [];
    this.isPolling = false;
    this.pollInterval = null;
  }

  /**
   * Get current SHAH price in USD
   * @param {boolean} forceRefresh - Force refresh from API
   * @returns {Promise<number>} - SHAH price in USD
   */
  async getShahPrice(forceRefresh = false) {
    const now = Date.now();
    
    // Return cached price if still valid
    if (!forceRefresh && 
        this.priceCache.price && 
        (now - this.priceCache.timestamp) < this.cacheDuration) {
      return this.priceCache.price;
    }

    try {
      // Try multiple price sources
      const price = await this.fetchPriceFromMultipleSources();
      
      if (price && price > 0) {
        this.updatePriceCache(price, 'api');
        this.notifyPriceUpdate(price);
        return price;
      }
    } catch (error) {
      console.warn('Failed to fetch SHAH price from API:', error);
    }

    // Return cached price if available, otherwise fallback
    if (this.priceCache.price) {
      return this.priceCache.price;
    }

    return this.fallbackPrice;
  }

  /**
   * Fetch price from multiple sources for reliability
   * @returns {Promise<number>} - SHAH price in USD
   */
  async fetchPriceFromMultipleSources() {
    const sources = [
      this.fetchFromShahSwap(),
      this.fetchFromCoinGecko(),
      this.fetchFromCoinMarketCap(),
      this.fetchFromCustomAPI()
    ];

    // Try sources in parallel, return first successful result
    const results = await Promise.allSettled(sources);
    
    for (const result of results) {
      if (result.status === 'fulfilled' && result.value && result.value > 0) {
        return result.value;
      }
    }

    throw new Error('All price sources failed');
  }

  /**
   * Fetch price from ShahSwap DEX
   * @returns {Promise<number>} - SHAH price in USD
   */
  async fetchFromShahSwap() {
    try {
      const response = await fetch(`${this.apiBaseUrl}/dex/price/SHAH/USD`, {
        method: 'GET',
        headers: { 'Content-Type': 'application/json' },
        timeout: 5000
      });

      if (!response.ok) throw new Error(`ShahSwap API error: ${response.status}`);
      
      const data = await response.json();
      return parseFloat(data.price || data.lastPrice || 0);
    } catch (error) {
      console.warn('ShahSwap price fetch failed:', error);
      return null;
    }
  }

  /**
   * Fetch price from CoinGecko
   * @returns {Promise<number>} - SHAH price in USD
   */
  async fetchFromCoinGecko() {
    try {
      const response = await fetch('https://api.coingecko.com/api/v3/simple/price?ids=shahcoin&vs_currencies=usd', {
        method: 'GET',
        headers: { 'Content-Type': 'application/json' },
        timeout: 5000
      });

      if (!response.ok) throw new Error(`CoinGecko API error: ${response.status}`);
      
      const data = await response.json();
      return parseFloat(data.shahcoin?.usd || 0);
    } catch (error) {
      console.warn('CoinGecko price fetch failed:', error);
      return null;
    }
  }

  /**
   * Fetch price from CoinMarketCap
   * @returns {Promise<number>} - SHAH price in USD
   */
  async fetchFromCoinMarketCap() {
    try {
      const response = await fetch('https://pro-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?symbol=SHAH&convert=USD', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
          'X-CMC_PRO_API_KEY': process.env.COINMARKETCAP_API_KEY || ''
        },
        timeout: 5000
      });

      if (!response.ok) throw new Error(`CoinMarketCap API error: ${response.status}`);
      
      const data = await response.json();
      return parseFloat(data.data?.SHAH?.quote?.USD?.price || 0);
    } catch (error) {
      console.warn('CoinMarketCap price fetch failed:', error);
      return null;
    }
  }

  /**
   * Fetch price from custom Shahcoin API
   * @returns {Promise<number>} - SHAH price in USD
   */
  async fetchFromCustomAPI() {
    try {
      const response = await fetch(`${this.apiBaseUrl}/price/shash/usd`, {
        method: 'GET',
        headers: { 'Content-Type': 'application/json' },
        timeout: 5000
      });

      if (!response.ok) throw new Error(`Custom API error: ${response.status}`);
      
      const data = await response.json();
      return parseFloat(data.price || data.usd || 0);
    } catch (error) {
      console.warn('Custom API price fetch failed:', error);
      return null;
    }
  }

  /**
   * Update price cache
   * @param {number} price - SHAH price in USD
   * @param {string} source - Price source
   */
  updatePriceCache(price, source) {
    this.priceCache = {
      price: price,
      timestamp: Date.now(),
      source: source
    };
  }

  /**
   * Subscribe to price updates
   * @param {Function} callback - Callback function
   * @returns {Function} - Unsubscribe function
   */
  subscribeToPriceUpdates(callback) {
    this.priceUpdateCallbacks.push(callback);
    
    // Return unsubscribe function
    return () => {
      const index = this.priceUpdateCallbacks.indexOf(callback);
      if (index > -1) {
        this.priceUpdateCallbacks.splice(index, 1);
      }
    };
  }

  /**
   * Notify all subscribers of price update
   * @param {number} price - New SHAH price
   */
  notifyPriceUpdate(price) {
    this.priceUpdateCallbacks.forEach(callback => {
      try {
        callback(price);
      } catch (error) {
        console.error('Price update callback error:', error);
      }
    });
  }

  /**
   * Start automatic price polling
   * @param {number} interval - Polling interval in milliseconds
   */
  startPolling(interval = 30000) {
    if (this.isPolling) return;
    
    this.isPolling = true;
    this.pollInterval = setInterval(async () => {
      try {
        await this.getShahPrice(true);
      } catch (error) {
        console.warn('Price polling failed:', error);
      }
    }, interval);
  }

  /**
   * Stop automatic price polling
   */
  stopPolling() {
    if (this.pollInterval) {
      clearInterval(this.pollInterval);
      this.pollInterval = null;
    }
    this.isPolling = false;
  }

  /**
   * Get price cache info
   * @returns {Object} - Price cache information
   */
  getPriceCacheInfo() {
    return {
      ...this.priceCache,
      isStale: (Date.now() - this.priceCache.timestamp) > this.cacheDuration,
      timeUntilStale: Math.max(0, this.cacheDuration - (Date.now() - this.priceCache.timestamp))
    };
  }

  /**
   * Convert SHAH amount to USD
   * @param {number} shahAmount - Amount in SHAH
   * @returns {Promise<number>} - USD equivalent
   */
  async shahToUSD(shahAmount) {
    const price = await this.getShahPrice();
    return shahAmount * price;
  }

  /**
   * Convert USD amount to SHAH
   * @param {number} usdAmount - Amount in USD
   * @returns {Promise<number>} - SHAH equivalent
   */
  async usdToShah(usdAmount) {
    const price = await this.getShahPrice();
    return usdAmount / price;
  }

  /**
   * Convert satoshis to USD
   * @param {number} satoshis - Amount in satoshis
   * @returns {Promise<number>} - USD equivalent
   */
  async satoshisToUSD(satoshis) {
    const shahAmount = satoshis / 100000000; // Convert satoshis to SHAH
    return this.shahToUSD(shahAmount);
  }

  /**
   * Convert USD to satoshis
   * @param {number} usdAmount - Amount in USD
   * @returns {Promise<number>} - Satoshis equivalent
   */
  async usdToSatoshis(usdAmount) {
    const shahAmount = await this.usdToShah(usdAmount);
    return Math.round(shahAmount * 100000000); // Convert SHAH to satoshis
  }
}

// Export singleton instance
export const priceOracle = new PriceOracle();

// Export class for custom instances
export { PriceOracle };

// Convenience function for getting SHAH price
export async function getShahPrice(forceRefresh = false) {
  return priceOracle.getShahPrice(forceRefresh);
}

// React hook for price oracle
export function usePriceOracle(config = {}) {
  const [price, setPrice] = useState(null);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState(null);
  const [lastUpdate, setLastUpdate] = useState(null);

  useEffect(() => {
    let unsubscribe = null;
    
    const initPrice = async () => {
      try {
        setIsLoading(true);
        setError(null);
        
        const currentPrice = await priceOracle.getShahPrice();
        setPrice(currentPrice);
        setLastUpdate(new Date());
        
        // Subscribe to updates
        unsubscribe = priceOracle.subscribeToPriceUpdates((newPrice) => {
          setPrice(newPrice);
          setLastUpdate(new Date());
        });
        
        // Start polling if enabled
        if (config.enablePolling !== false) {
          priceOracle.startPolling(config.pollInterval);
        }
      } catch (err) {
        setError(err.message);
      } finally {
        setIsLoading(false);
      }
    };

    initPrice();

    return () => {
      if (unsubscribe) unsubscribe();
      if (config.enablePolling !== false) {
        priceOracle.stopPolling();
      }
    };
  }, [config.enablePolling, config.pollInterval]);

  const refreshPrice = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      
      const newPrice = await priceOracle.getShahPrice(true);
      setPrice(newPrice);
      setLastUpdate(new Date());
    } catch (err) {
      setError(err.message);
    } finally {
      setIsLoading(false);
    }
  }, []);

  return {
    price,
    isLoading,
    error,
    lastUpdate,
    refreshPrice,
    shahToUSD: (shahAmount) => priceOracle.shahToUSD(shahAmount),
    usdToShah: (usdAmount) => priceOracle.usdToShah(usdAmount),
    satoshisToUSD: (satoshis) => priceOracle.satoshisToUSD(satoshis),
    usdToSatoshis: (usdAmount) => priceOracle.usdToSatoshis(usdAmount)
  };
}
