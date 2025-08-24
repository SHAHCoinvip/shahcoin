export interface AIBalanceCheckerDeps {
  getWalletUsd: () => Promise<number> | number;
  thresholdUsd?: number; // default 10
}

export class AIBalanceChecker {
  private deps: AIBalanceCheckerDeps;
  private timer?: any;
  private lastFree?: boolean;
  private intervalMs: number;
  onChange?: (hasFree: boolean, walletUsd: number) => void;

  constructor(deps: AIBalanceCheckerDeps, intervalMs = 15000) {
    this.deps = deps;
    this.intervalMs = intervalMs;
  }

  start() {
    this.stop();
    this.timer = setInterval(() => this.refresh(), this.intervalMs);
    this.refresh();
  }

  stop() {
    if (this.timer) clearInterval(this.timer);
    this.timer = undefined;
  }

  async refresh() {
    const threshold = this.deps.thresholdUsd ?? 10;
    const usd = await this.deps.getWalletUsd();
    const isFree = usd >= threshold;
    if (this.lastFree !== isFree) {
      this.lastFree = isFree;
      this.onChange?.(isFree, usd);
    }
  }
}


