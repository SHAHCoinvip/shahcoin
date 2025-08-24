/* Vault + Deploy Tool (JS)
 * Packages contract + metadata + assets and provides deploy helpers.
 */

export class VaultDeployTool {
	constructor() { this.bundles = new Map(); }

	createBundle({ name, contractSource, metadata, assets = {} }) {
		if (!name) throw new Error('Bundle name required');
		const bundle = {
			name,
			metadata,
			contract: { language: 'solidity', source: contractSource },
			assets,
			createdAt: new Date().toISOString(),
			version: '1.0.0'
		};
		this.bundles.set(name, bundle);
		return bundle;
	}

	exportAsJson(name) {
		const b = this.bundles.get(name);
		if (!b) throw new Error('Bundle not found');
		return JSON.stringify(b, null, 2);
	}

	listBundles() { return Array.from(this.bundles.keys()); }
}

export default { VaultDeployTool };
