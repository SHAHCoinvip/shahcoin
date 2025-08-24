import React, { useMemo, useState } from 'react';

export type PaymentMethod = 'SHAH' | 'SHAHtoken' | 'Stripe';

export interface UpgradeAIWalletProps {
  usdPerShah: number; // live price from PriceOracle
  premiumPriceUsd?: number; // default 9.99
  onUpgrade: (method: PaymentMethod, amountShahRounded4: number) => Promise<{ success: boolean; receipt?: string; error?: string }>;
  onCancel?: () => void;
  onBuyShah?: () => void; // opens BuyShah flow
}

const fmt = (n: number) => (isFinite(n) ? n.toFixed(4) : '0.0000');

export const UpgradeAIWallet: React.FC<UpgradeAIWalletProps> = ({
  usdPerShah,
  premiumPriceUsd = 9.99,
  onUpgrade,
  onCancel,
  onBuyShah,
}) => {
  const [method, setMethod] = useState<PaymentMethod>('SHAH');
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState<string>('');

  const shahAmount = useMemo(() => {
    if (usdPerShah <= 0) return 0;
    const raw = premiumPriceUsd / usdPerShah;
    return Math.round(raw * 1e4) / 1e4; // round to 4 decimals
  }, [usdPerShah, premiumPriceUsd]);

  const onConfirm = async () => {
    setLoading(true);
    setMessage('Processing payment…');
    try {
      const res = await onUpgrade(method, shahAmount);
      if (res.success) setMessage(`Success! Receipt/TX: ${res.receipt ?? 'N/A'}`);
      else setMessage(`Payment failed: ${res.error ?? 'Unknown error'}`);
    } catch (e: any) {
      setMessage(`Payment failed: ${e?.message ?? 'Unknown error'}`);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{ padding: 16, maxWidth: 520 }}>
      <h2>💳 Upgrade to AI Wallet</h2>
      <p>Unlock full AI features (Assistant, Insights, Troubleshooter, Fee Optimizer, Multilingual).</p>

      <div style={{ marginTop: 12, padding: 12, border: '1px solid #e5e7eb', borderRadius: 8 }}>
        <strong>Price</strong>
        <div>USD: ${premiumPriceUsd.toFixed(2)}</div>
        <div>SHAH: {fmt(shahAmount)} (at ${usdPerShah.toFixed(4)} per SHAH)</div>
      </div>

      <div style={{ marginTop: 16 }}>
        <strong>Payment Method</strong>
        <div style={{ display: 'flex', gap: 12, marginTop: 8 }}>
          <label><input type="radio" checked={method === 'SHAH'} onChange={() => setMethod('SHAH')} /> SHAH (auto-convert ${premiumPriceUsd.toFixed(2)})</label>
          <label><input type="radio" checked={method === 'SHAHtoken'} onChange={() => setMethod('SHAHtoken')} /> SHAHtoken (SHI-20)</label>
          <label><input type="radio" checked={method === 'Stripe'} onChange={() => setMethod('Stripe')} /> Stripe (Credit Card)</label>
        </div>
      </div>

      <div style={{ marginTop: 16, display: 'flex', gap: 8 }}>
        <button onClick={onConfirm} disabled={loading} style={{ padding: '8px 12px', background: '#4CAF50', color: '#fff', border: 0, borderRadius: 6 }}>
          {loading ? 'Processing…' : 'Confirm Upgrade'}
        </button>
        {onBuyShah && (
          <button onClick={onBuyShah} disabled={loading} style={{ padding: '8px 12px', background: '#0ea5e9', color: '#fff', border: 0, borderRadius: 6 }}>
            Buy SHAH with Card
          </button>
        )}
        {onCancel && (
          <button onClick={onCancel} disabled={loading} style={{ padding: '8px 12px', border: '1px solid #ccc', borderRadius: 6 }}>
            Cancel
          </button>
        )}
      </div>

      {message && <p style={{ marginTop: 12 }}>{message}</p>}

      <p style={{ marginTop: 12, color: '#6b7280' }}>
        Note: Amount is dynamically calculated using the live ShahSwap price oracle.
      </p>
    </div>
  );
};

export default UpgradeAIWallet;


