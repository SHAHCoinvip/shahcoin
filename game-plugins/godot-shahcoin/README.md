# Godot Shahcoin Plugin

[![Built on SHAHCOIN Core](https://img.shields.io/badge/Built%20on-Shahcoin%20Core-blue.svg)](https://shah.vip/dev)
[![Godot Version](https://img.shields.io/badge/Godot-4.0%2B-green.svg)](https://godotengine.org/)

Godot plugin for integrating Shahcoin blockchain functionality into your games.

## Features

- 🔗 **Wallet Integration** - Connect to Shahcoin wallets
- 💰 **Balance Checking** - Check SHAH and token balances
- 🎮 **NFT Ownership** - Verify NFT ownership for in-game items
- 🎯 **In-Game Actions** - Trigger game events based on wallet state
- 🎁 **ShahSwap Rewards** - Integrate with ShahSwap for rewards/boosts
- 🔑 **API Key Authentication** - Secure integration with API keys

## Installation

### Method 1: Asset Library

1. Open Godot and go to AssetLib
2. Search for "Shahcoin Plugin"
3. Click "Download" and install

### Method 2: Manual Installation

1. Download the plugin files
2. Copy the `shahcoin_plugin` folder to your Godot project's `addons` folder
3. Enable the plugin in Project Settings > Plugins

## Quick Start

### 1. Setup API Key

```gdscript
# Initialize the plugin with your API key
ShahcoinPlugin.initialize("your-api-key-here")
```

### 2. Connect Wallet

```gdscript
# Connect to a wallet
ShahcoinPlugin.connect_wallet(func(success, address):
    if success:
        print("Connected to wallet: ", address)
    else:
        print("Failed to connect wallet")
)
```

### 3. Check Balance

```gdscript
# Check SHAH balance
ShahcoinPlugin.get_balance(func(balance):
    print("SHAH Balance: ", balance)
)

# Check token balance
ShahcoinPlugin.get_token_balance("TOKEN_CONTRACT_ADDRESS", func(balance):
    print("Token Balance: ", balance)
)
```

### 4. Check NFT Ownership

```gdscript
# Check if player owns a specific NFT
ShahcoinPlugin.has_nft("NFT_CONTRACT_ADDRESS", "TOKEN_ID", func(has_nft):
    if has_nft:
        print("Player owns this NFT!")
        # Unlock special content
        unlock_special_content()
)
```

### 5. In-Game Actions

```gdscript
# Trigger game action based on wallet state
func check_player_status():
    ShahcoinPlugin.get_balance(func(balance):
        if balance >= 100:
            # Player has VIP status
            enable_vip_features()
    )
```

## API Reference

### ShahcoinPlugin

Main singleton for managing Shahcoin integration.

#### Static Methods

- `initialize(api_key: String)` - Initialize the plugin with API key
- `set_network(network: ShahcoinNetwork)` - Set the network to use

#### Instance Methods

##### Wallet Operations

```gdscript
# Connect to wallet
connect_wallet(callback: Callable)

# Disconnect wallet
disconnect_wallet()

# Check if wallet is connected
is_wallet_connected() -> bool

# Get wallet address
get_wallet_address() -> String
```

##### Balance Operations

```gdscript
# Get SHAH balance
get_balance(callback: Callable)

# Get token balance
get_token_balance(contract_address: String, callback: Callable)

# Get NFT count
get_nft_count(contract_address: String, callback: Callable)
```

##### NFT Operations

```gdscript
# Check NFT ownership
has_nft(contract_address: String, token_id: String, callback: Callable)

# Get owned NFTs
get_owned_nfts(contract_address: String, callback: Callable)

# Get NFT metadata
get_nft_metadata(contract_address: String, token_id: String, callback: Callable)
```

##### ShahSwap Integration

```gdscript
# Get token price
get_token_price(token_symbol: String, callback: Callable)

# Check if player can afford item
can_afford_item(token_symbol: String, amount: float, callback: Callable)

# Get swap quote
get_swap_quote(input_token: String, output_token: String, amount: float, callback: Callable)
```

### Data Structures

#### NFTInfo

```gdscript
class NFTInfo:
    var token_id: String
    var name: String
    var description: String
    var image_url: String
    var attributes: Dictionary
```

#### NFTMetadata

```gdscript
class NFTMetadata:
    var name: String
    var description: String
    var image: String
    var attributes: Dictionary
```

#### SwapQuote

```gdscript
class SwapQuote:
    var input_token: String
    var output_token: String
    var input_amount: float
    var output_amount: float
    var price_impact: float
    var fee: float
```

## Examples

### VIP System

```gdscript
extends Node

func _ready():
    check_vip_status()

func check_vip_status():
    ShahcoinPlugin.get_balance(func(balance):
        if balance >= 1000:
            enable_vip_features()
    )

func enable_vip_features():
    # Enable VIP-only features
    print("VIP features enabled!")
```

### NFT-Based Items

```gdscript
extends Node

@export var nft_contract_address: String = "NFT_CONTRACT_ADDRESS"
@export var required_token_id: String = "1"

func check_item_access():
    ShahcoinPlugin.has_nft(nft_contract_address, required_token_id, func(has_nft):
        if has_nft:
            unlock_item()
        else:
            show_purchase_prompt()
    )

func unlock_item():
    # Unlock the item for the player
    print("Item unlocked!")

func show_purchase_prompt():
    # Show purchase prompt
    print("Purchase this NFT to unlock the item!")
```

### Token-Based Economy

```gdscript
extends Node

@export var game_token_address: String = "GAME_TOKEN_ADDRESS"
@export var item_cost: float = 10.0

func purchase_item():
    ShahcoinPlugin.get_token_balance(game_token_address, func(balance):
        if balance >= item_cost:
            process_purchase()
        else:
            show_insufficient_funds()
    )

func process_purchase():
    # Process the purchase
    print("Purchase successful!")

func show_insufficient_funds():
    print("Insufficient funds!")
```

## Configuration

### Network Settings

The plugin supports multiple networks:

```gdscript
# Set network (default: mainnet)
ShahcoinPlugin.set_network(ShahcoinPlugin.NETWORK_MAINNET)
ShahcoinPlugin.set_network(ShahcoinPlugin.NETWORK_TESTNET)
ShahcoinPlugin.set_network(ShahcoinPlugin.NETWORK_REGTEST)
```

### Error Handling

```gdscript
# Subscribe to error events
ShahcoinPlugin.error_occurred.connect(func(error):
    print("Shahcoin Error: ", error)
)

# Subscribe to connection events
ShahcoinPlugin.wallet_connected.connect(func(address):
    print("Wallet connected: ", address)
)

ShahcoinPlugin.wallet_disconnected.connect(func():
    print("Wallet disconnected")
)
```

## Signals

The plugin provides several signals for event handling:

```gdscript
# Connection events
wallet_connected(address: String)
wallet_disconnected()

# Balance events
balance_changed(balance: float)

# Transaction events
transaction_received(tx_data: Dictionary)

# Error events
error_occurred(error: String)
```

## Best Practices

1. **Always check wallet connection** before making API calls
2. **Handle errors gracefully** and provide user feedback
3. **Cache results** when possible to reduce API calls
4. **Use appropriate networks** (testnet for development)
5. **Secure your API keys** and never expose them in client builds

## Support

- 📖 **Documentation**: [https://shah.vip/dev](https://shah.vip/dev)
- 🐛 **Issues**: [GitHub Issues](https://github.com/SHAHCoinvip/shahcoin/issues)
- 💬 **Discord**: [Shahcoin Community](https://discord.gg/shahcoin)

## License

MIT License - see [LICENSE](./LICENSE) for details.

---

**Built with love by SHAHCOIN Core Contributors** ❤️
