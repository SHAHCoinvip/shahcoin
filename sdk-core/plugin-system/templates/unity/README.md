# Unity Plugin Template (Wallet Login + SHAH Payments)

## Features
- Wallet connect (QR / Deep link)
- Get SHAH balance and SHI-30 NFTs
- Create SHAH payment requests

## Usage (C#)
```csharp
using Shahcoin.Unity;

var client = new ShahcoinClient();
await client.ConnectWallet();
var balance = await client.GetBalance();
await client.CreatePayment(new PaymentRequest { AmountShah = 12.5M, Memo = "Game Item" });
```
