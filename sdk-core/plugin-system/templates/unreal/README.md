# Unreal Engine Plugin Template (Wallet Login + SHAH Payments)

## Features
- Wallet connect (QR / Deep link)
- Read SHI-20/30 assets
- Trigger SHAH payments

## Usage (C++)
```cpp
#include "ShahcoinClient.h"

UShahcoinClient* Client = NewObject<UShahcoinClient>();
Client->ConnectWallet();
Client->CreatePayment(12.5f, TEXT("Game Item"));
```
