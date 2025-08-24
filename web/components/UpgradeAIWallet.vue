<template>
  <div class="upgrade-card">
    <header class="header">
      <div class="title-row">
        <span class="logo">🤖</span>
        <h2>Upgrade to AI Wallet</h2>
      </div>
      <p class="subtitle">Unlock full AI features: Assistant, Insights, Troubleshooter, Fee Optimizer, Multilingual.</p>
    </header>

    <section class="price-box">
      <div class="price-row">
        <div class="label">Price</div>
        <div class="value">${{ premiumPriceUsd.toFixed(2) }}</div>
      </div>
      <div class="price-row muted">
        <div class="label">@ SHAH</div>
        <div class="value">≈ {{ shahAmountLabel }}</div>
      </div>
      <div class="price-row muted">
        <div class="label">@ {{ shahTokenSymbol }}</div>
        <div class="value">≈ {{ shahTokenAmountLabel }}</div>
      </div>
    </section>

    <section>
      <div class="section-title">Payment Method</div>
      <div class="methods">
        <label :title="'Use SHAH from your wallet to upgrade.'">
          <input type="radio" name="method" value="SHAH" v-model="method" />
          <span class="icon">🪙</span> SHAH (native)
        </label>
        <label :title="'SHI-20 SHAHtoken supported for easier access.'">
          <input type="radio" name="method" value="SHAHtoken" v-model="method" />
                      <span class="icon">🔗</span> {{ shahTokenSymbol }} (SHI-20)
        </label>
        <label :title="'Prefer card? Pay $9.99 USD via Stripe.'">
          <input type="radio" name="method" value="Stripe" v-model="method" />
          <span class="icon">💳</span> Stripe (Credit Card)
        </label>
      </div>
    </section>

    <section class="actions">
      <button class="btn primary" :disabled="loading || !canConfirm" @click="onConfirm">
        <span v-if="loading">Processing…</span>
        <span v-else>Confirm Upgrade</span>
      </button>
      <button class="btn info" :disabled="loading" @click="$emit('buy-shah')">
        Buy SHAH with Card
      </button>
      <button class="btn" :disabled="loading" @click="$emit('cancel')">
        Cancel
      </button>
    </section>

    <transition name="fade">
      <p v-if="message" class="message">{{ message }}</p>
    </transition>

    <div v-if="success" class="success-box">
      <div class="check">✅</div>
      <div>
        <div class="success-title">AI Assistant Activated!</div>
        <div class="success-desc">You now have Premium access to all AI tools.</div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, ref, watch, onMounted } from 'vue';

type PaymentMethod = 'SHAH' | 'SHAHtoken' | 'Stripe';

const props = defineProps<{
  usdPerShah?: number;
  usdPerShahToken?: number;
  premiumPriceUsd?: number; // default 9.99
  stripeCheckoutUrl?: string;
  shahTokenSymbol?: string; // default 'SHAHtoken'
}>();

const emit = defineEmits<{
  (e: 'upgrade', payload: { method: PaymentMethod; amount: number; currency: 'SHAH' | 'SHAHtoken' }): void;
  (e: 'stripe'): void;
  (e: 'buy-shah'): void;
  (e: 'cancel'): void;
}>();

const method = ref<PaymentMethod>('SHAH');
const loading = ref(false);
const message = ref('');
const success = ref(false);

const premiumPriceUsd = computed(() => props.premiumPriceUsd ?? 9.99);
const shahTokenSymbol = computed(() => props.shahTokenSymbol ?? 'SHAHtoken');

const usdPerShah = ref<number | undefined>(props.usdPerShah);
const usdPerShahToken = ref<number | undefined>(props.usdPerShahToken);

// Optional: Try to fetch price from ShahSwap API if not provided.
async function ensurePrice() {
  try {
    if (!usdPerShah.value) {
      // Placeholder: replace with real ShahSwap price endpoint
      // const res = await fetch('/api/price/shah');
      // const { price } = await res.json();
      // usdPerShah.value = price;
    }
    if (!usdPerShahToken.value) {
      // Placeholder: replace with real ShahSwap price endpoint for SHAHtoken
      // const res = await fetch('/api/price/shahtoken');
      // const { price } = await res.json();
      // usdPerShahToken.value = price;
    }
  } catch (e) {
    // ignore; UI will show N/A
  }
}

onMounted(() => {
  ensurePrice();
});

const shahAmount = computed(() => {
  const price = usdPerShah.value ?? 0;
  if (price <= 0) return 0;
  const raw = premiumPriceUsd.value / price;
  return Math.round(raw * 1e4) / 1e4; // 4 decimals
});

const shahTokenAmount = computed(() => {
  const price = usdPerShahToken.value ?? 0;
  if (price <= 0) return 0;
  const raw = premiumPriceUsd.value / price;
  return Math.round(raw * 1e4) / 1e4;
});

const shahAmountLabel = computed(() =>
  usdPerShah.value && usdPerShah.value > 0 ? `${shahAmount.value.toFixed(4)} SHAH` : 'N/A'
);
const shahTokenAmountLabel = computed(() =>
  usdPerShahToken.value && usdPerShahToken.value > 0 ? `${shahTokenAmount.value.toFixed(4)} ${shahTokenSymbol.value}` : 'N/A'
);

const canConfirm = computed(() => {
  if (method.value === 'SHAH') return (usdPerShah.value ?? 0) > 0;
  if (method.value === 'SHAHtoken') return (usdPerShahToken.value ?? 0) > 0;
  return true; // Stripe always available
});

async function onConfirm() {
  loading.value = true;
  message.value = 'Processing…';
  success.value = false;
  try {
    if (method.value === 'Stripe') {
      if (props.stripeCheckoutUrl) {
        window.location.href = props.stripeCheckoutUrl;
        return;
      }
      // Let parent open/create checkout session
      emit('stripe');
      message.value = 'Opening Stripe Checkout…';
      return;
    }

    if (method.value === 'SHAH') {
      emit('upgrade', { method: 'SHAH', amount: shahAmount.value, currency: 'SHAH' });
      message.value = `Requesting wallet payment: ${shahAmount.value.toFixed(4)} SHAH…`;
    } else if (method.value === 'SHAHtoken') {
      emit('upgrade', { method: 'SHAHtoken', amount: shahTokenAmount.value, currency: 'SHAHtoken' });
      message.value = `Requesting wallet payment: ${shahTokenAmount.value.toFixed(4)} ${shahTokenSymbol.value}…`;
    }

    // Parent is responsible for verifying on-chain and then marking success
    // For demo, we simulate instant success after short delay
    setTimeout(() => {
      success.value = true;
      message.value = '✅ Upgrade complete!';
    }, 800);
  } catch (e: any) {
    message.value = `Error: ${e?.message ?? 'Unknown error'}`;
  } finally {
    loading.value = false;
  }
}
</script>

<style scoped>
.upgrade-card {
  background: #0b1220;
  color: #e5e7eb;
  border: 1px solid #1f2937;
  border-radius: 12px;
  padding: 16px;
  max-width: 720px;
}
.header .title-row {
  display: flex;
  align-items: center;
  gap: 8px;
}
.header .logo {
  font-size: 20px;
}
.subtitle {
  color: #9ca3af;
  margin-top: 4px;
}
.price-box {
  margin-top: 12px;
  padding: 12px;
  border: 1px solid #1f2937;
  border-radius: 8px;
  background: #0f172a;
}
.price-row {
  display: flex;
  justify-content: space-between;
  margin: 4px 0;
}
.price-row .label { color: #9ca3af; }
.price-row.muted .value { color: #9ca3af; }

.section-title {
  margin-top: 16px;
  margin-bottom: 8px;
  font-weight: 600;
}
.methods {
  display: grid;
  gap: 10px;
}
.methods label {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px;
  border: 1px solid #1f2937;
  border-radius: 8px;
  cursor: pointer;
  background: #0f172a;
}
.methods input { accent-color: #22c55e; }
.icon { width: 18px; text-align: center; }

.actions {
  display: flex;
  gap: 8px;
  margin-top: 16px;
}
.btn {
  padding: 8px 12px;
  border-radius: 8px;
  background: #111827;
  color: #e5e7eb;
  border: 1px solid #374151;
}
.btn.primary { background: #22c55e; border-color: #22c55e; color: #0b1220; }
.btn.info { background: #0ea5e9; border-color: #0ea5e9; color: #0b1220; }
.btn:disabled { opacity: 0.6; cursor: not-allowed; }

.message { margin-top: 10px; color: #9ca3af; }

.success-box {
  margin-top: 16px;
  display: flex;
  gap: 12px;
  align-items: center;
  padding: 12px;
  border: 1px solid #1f2937;
  border-radius: 8px;
  background: #0f172a;
}
.check { font-size: 20px; }
.success-title { font-weight: 700; }
.success-desc { color: #9ca3af; }

.fade-enter-active, .fade-leave-active { transition: opacity .2s; }
.fade-enter-from, .fade-leave-to { opacity: 0; }
</style>


