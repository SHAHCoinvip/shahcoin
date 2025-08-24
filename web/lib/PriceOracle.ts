export interface PriceOracleDeps {
  getPrice: () => Promise<number> | number; // returns USD per SHAH
}

export class PriceOracle {
  private deps: PriceOracleDeps;
  private timer?: any;
  private last?: number;
  private intervalMs: number;
  onPrice?: (usdPerShah: number) => void;

  constructor(deps: PriceOracleDeps, intervalMs = 30000) {
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
    const v = await this.deps.getPrice();
    if (v > 0 && this.last !== v) {
      this.last = v;
      this.onPrice?.(v);
    }
  }
}


