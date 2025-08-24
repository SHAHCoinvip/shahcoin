/* Transaction Testing Sandbox (JS)
 * Simulates wallet functions with dummy keys/funds for dev use only.
 */

import crypto from 'crypto';

export class SandboxWallet {
	constructor() {
		this.privateKey = crypto.randomBytes(32).toString('hex');
		this.address = 'S' + crypto.randomBytes(20).toString('hex');
		this.balanceShah = 1000;
	}
	getAddress() { return this.address; }
	getBalance() { return this.balanceShah; }
	async signMessage(message) { return crypto.createHmac('sha256', this.privateKey).update(message).digest('hex'); }
}

export class TransactionSandbox {
	constructor() { this.txHistory = []; }

	createMockTx({ from, to, amountShah, memo }) {
		if (amountShah <= 0) throw new Error('Invalid amount');
		const tx = {
			id: 'tx_' + crypto.randomBytes(8).toString('hex'),
			from, to, amountShah, memo: memo || '',
			feeShah: 0.001, status: 'created', createdAt: new Date().toISOString()
		};
		this.txHistory.push(tx);
		return tx;
	}

	async simulateBroadcast(tx) {
		tx.status = 'broadcast';
		tx.broadcastAt = new Date().toISOString();
		await new Promise(r => setTimeout(r, 200));
		tx.status = 'confirmed';
		tx.confirmedAt = new Date().toISOString();
		return tx;
	}

	listTx() { return this.txHistory.slice().reverse(); }
}

export default { SandboxWallet, TransactionSandbox };
