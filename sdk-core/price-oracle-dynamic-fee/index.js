/* Shahcoin Price Oracle + Dynamic Fee Module (JS SDK)
 * Lightweight implementation with in-memory caching and polling.
 */

export class ShahcoinPriceOracle {
	constructor(options = {}) {
		this.sources = options.sources || ['shahswap'];
		this.updateIntervalMs = options.updateInterval || 30000;
		this.cacheTtlMs = options.cacheTimeout || 60000;
		this.lastPrice = null; // { usd, shah, timestamp, sources }
		this._subscribers = new Set();
		this._timer = null;
	}

	async getCurrentPrice() {
		const now = Date.now();
		if (this.lastPrice && now - this.lastPrice.timestamp < this.cacheTtlMs) {
			return this.lastPrice;
		}
		const fetched = await this._fetchPriceFromSources();
		this.lastPrice = { ...fetched, timestamp: Date.now() };
		this._emit(this.lastPrice);
		return this.lastPrice;
	}

	subscribeToPriceUpdates(callback) {
		this._subscribers.add(callback);
		if (!this._timer) {
			this._timer = setInterval(async () => {
				try {
					const price = await this.getCurrentPrice();
					this._emit(price);
				} catch (_) {}
			}, this.updateIntervalMs);
		}
		return () => this._subscribers.delete(callback);
	}

	_emit(price) {
		for (const cb of this._subscribers) {
			try { cb(price); } catch (_) {}
		}
	}

	async _fetchPriceFromSources() {
		// NOTE: Replace with real HTTP calls to ShahSwap and other sources.
		// Fallback deterministic mock for offline/dev usage.
		const baseUsd = 0.85; // mock SHAH price in USD
		return { usd: baseUsd, shah: 1, sources: this.sources.slice(0, 1) };
	}
}

export class DynamicFeeCalculator {
	constructor(options = {}) {
		this.priceOracle = options.priceOracle || new ShahcoinPriceOracle();
		this.baseFees = options.baseFees || { token_creation: 49, nft_creation: 25, bundle_creation: 59 };
		this.addOnFees = options.addOnFees || {
			custom_logo: 10,
			verified_badge: 20,
			upgradeable_contract: 15,
			airdrop_tool: 8,
			custom_image: 5,
			unlockable_content: 10,
			trade_lock: 6,
			booster_tag: 9
		};
		this.priceBuffer = options.priceBuffer ?? 0.02; // 2% buffer
	}

	async calculateFee(feeType, opts = {}) {
		const price = await this.priceOracle.getCurrentPrice();
		const baseUsd = this.baseFees[feeType] ?? 0;
		const addOns = Array.isArray(opts.addOns) ? opts.addOns : [];
		const addOnsUsd = addOns.reduce((sum, key) => sum + (this.addOnFees[key] || 0), 0);
		let totalUsd = baseUsd + addOnsUsd;
		if (opts.volumeUsd && opts.volumeUsd >= 1000) totalUsd *= 0.9; // 10% volume discount
		if (feeType === 'bundle_creation') totalUsd = Math.min(totalUsd, this.baseFees.bundle_creation); // bundle cap
		const usdWithBuffer = totalUsd * (1 + this.priceBuffer);
		const shah = +(usdWithBuffer / price.usd).toFixed(8);
		return { usd: +usdWithBuffer.toFixed(2), shah, breakdown: { baseUsd, addOnsUsd, buffer: this.priceBuffer } };
	}

	subscribeToFeeUpdates(feeTypes, callback) {
		const types = Array.isArray(feeTypes) ? feeTypes : [feeTypes];
		return this.priceOracle.subscribeToPriceUpdates(async () => {
			const results = {};
			for (const t of types) results[t] = await this.calculateFee(t);
			try { callback(results); } catch (_) {}
		});
	}
}

export default { ShahcoinPriceOracle, DynamicFeeCalculator };
