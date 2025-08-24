/* Developer Analytics (JS)
 * Tracks SDK usage, token creation, and developer interactions.
 */

export class DevAnalytics {
	constructor(options = {}) {
		this.buffer = [];
		this.flushIntervalMs = options.flushInterval || 5000;
		this.endpoint = options.endpoint || null; // POST endpoint
		this.timer = setInterval(() => this.flush().catch(() => {}), this.flushIntervalMs);
	}

	track(eventName, payload = {}) {
		this.buffer.push({ event: eventName, payload, ts: Date.now() });
	}

	trackTokenCreation(details) { this.track('token_creation', details); }
	trackNftCreation(details) { this.track('nft_creation', details); }
	trackFeePaid(details) { this.track('fee_paid', details); }
	trackDeploy(details) { this.track('deploy', details); }

	async flush() {
		if (!this.buffer.length) return { ok: true, sent: 0 };
		const batch = this.buffer.splice(0, this.buffer.length);
		if (!this.endpoint) return { ok: true, sent: batch.length };
		await fetch(this.endpoint, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify(batch) });
		return { ok: true, sent: batch.length };
	}

	dispose() { clearInterval(this.timer); }
}

export default { DevAnalytics };
