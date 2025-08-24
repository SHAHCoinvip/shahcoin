# WooCommerce Plugin Template (SHAH Payments)

## Overview
Add SHAH payment gateway to WooCommerce via Shahcoin Plugin SDK.

## Steps
1. Create a custom WooCommerce gateway plugin.
2. Expose gateway settings for wallet address and API key.
3. Generate a payment request in SHAH and display QR code.
4. Verify on-chain confirmation and mark order as paid.

## Example (PHP Snippet)
```php
public function process_payment($order_id) {
  $order = wc_get_order($order_id);
  // Create SHAH invoice via Shahcoin API
  // Return array('result' => 'success', 'redirect' => $payment_url);
}
```
