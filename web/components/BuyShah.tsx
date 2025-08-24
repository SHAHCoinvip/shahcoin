import React, { useMemo, useState } from 'react';

export interface BuyShahProps {
  usdPerShah: number;
  defaultUsd?: number; // default onramp amount
  onStripePay: (usdAmount: number) => Promise<{ success: boolean; txid?: string; error?: string }>;
}

export const BuyShah: React.FC<BuyShahProps> = ({ usdPerShah, defaultUsd = 25, onStripePay }) => {
  const [usd, setUsd] = useState(defaultUsd);
  const [loading, setLoading] = useState(false);
  const [msg, setMsg] = useState('');

  const shahEst = useMemo(() => (usdPerShah > 0 ? usd / usdPerShah : 0), [usd, usdPerShah]);
  const est4 = Math.round(shahEst * 1e4) / 1e4;

  const onBuy = async () => {
    setLoading(true);
    setMsg('Processing Stripe payment…');
    try {
      const r = await onStripePay(usd);
      if (r.success) setMsg(`Success! Deposit TX: ${r.txid ?? 'N/A'}`);
      else setMsg(`Failed: ${r.error ?? 'Unknown error'}`);
    } catch (e: any) {
      setMsg(`Failed: ${e?.message ?? 'Unknown error'}`);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{ padding: 16, maxWidth: 520 }}>
      <h2>🪙 Buy SHAH</h2>
      <p>Onramp with Stripe, auto-convert via ShahSwap, deposit to your wallet.</p>
      <div style={{ display: 'grid', gap: 8 }}>
        <label>
          USD Amount:
          <input type="number" value={usd} onChange={e => setUsd(Number(e.target.value))} min={5} max={1000} step={5} style={{ marginLeft: 8 }} />
        </label>
        <div>Estimated SHAH: <strong>{est4.toFixed(4)}</strong> @ ${usdPerShah.toFixed(4)} / SHAH</div>
        <div style={{ color: '#b45309' }}>Slippage warning: Actual received SHAH may vary based on market conditions.</div>
      </div>
      <div style={{ marginTop: 12 }}>
        <button onClick={onBuy} disabled={loading} style={{ padding: '8px 12px', background: '#0ea5e9', color: '#fff', border: 0, borderRadius: 6 }}>
          {loading ? 'Processing…' : 'Buy with Card'}
        </button>
      </div>
      {msg && <p style={{ marginTop: 12 }}>{msg}</p>}
    </div>
  );
};

export default BuyShah;


