/* Metadata + Explorer APIs (client-side helper with in-memory storage)
 * In production, back with real REST endpoints.
 */

import { ShahcoinMetadataSchema } from '../smart-contract-toolkit/metadata-standards.js';

class InMemoryStore {
	constructor() { this.map = new Map(); }
	set(addr, value) { this.map.set(addr.toLowerCase(), value); }
	get(addr) { return this.map.get(addr.toLowerCase()) || null; }
	all() { return Array.from(this.map.entries()).map(([address, metadata]) => ({ address, metadata })); }
}

export class MetadataExplorerAPI {
	constructor() { this.store = new InMemoryStore(); }

	storeMetadata(contractAddress, metadata) {
		ShahcoinMetadataSchema.validateMetadata(metadata);
		this.store.set(contractAddress, {
			...metadata,
			storedAt: new Date().toISOString(),
			lastAccessed: new Date().toISOString()
		});
		return { ok: true };
	}

	getMetadata(contractAddress) {
		const data = this.store.get(contractAddress);
		if (!data) return null;
		data.lastAccessed = new Date().toISOString();
		return data;
	}

	listByType(type) {
		return this.store.all().filter(x => x.metadata?.type === type);
	}

	searchByName(query) {
		const q = String(query || '').toLowerCase();
		return this.store.all().filter(x => String(x.metadata?.name || '').toLowerCase().includes(q));
	}
}

export default { MetadataExplorerAPI };
