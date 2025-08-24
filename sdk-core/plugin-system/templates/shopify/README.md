# Shopify Plugin Template (SHAH Payments)

## Overview
Integrate SHAH payments in Shopify using Shahcoin Plugin SDK.

## Steps
1. Create a Shopify private app and obtain API keys.
2. Use a checkout extension to add a custom SHAH payment option.
3. Call Shahcoin payment API from the extension, listen for confirmation.
4. Update order status when on-chain confirmation is received.

## Example (Pseudo-code)
```js
import { createCheckoutSession } from '@shahcoin/plugins/shopify';

const session = await createCheckoutSession({
  amountUsd: 49.00,
  currency: 'USD',
  returnUrl: 'https://shop.example.com/thanks',
});
// Redirect to SHAH payment page or embedded widget
```
