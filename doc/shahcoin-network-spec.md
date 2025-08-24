# Shahcoin Network Specification

## Overview
Shahcoin is a native mainnet-only blockchain based on SHAHCOIN Core, designed for secure and efficient digital transactions.

## Network Parameters

### Mainnet
- **Network Name**: Shahcoin Mainnet
- **Message Start**: `0x53 0x48 0x41 0x48` (SHAH)
- **Default Port**: 8369
- **Bech32 Prefix**: `shah`
- **Genesis Block**: Same as Shahcoin (for compatibility)
- **Block Time**: 10 minutes
- **Difficulty Adjustment**: Every 2016 blocks (2 weeks)
- **Subsidy Halving**: Every 210,000 blocks (~4 years)

### Testnet
- **Network Name**: Shahcoin Testnet
- **Message Start**: `0x53 0x48 0x41 0x54` (SHAT)
- **Default Port**: 18369
- **Bech32 Prefix**: `tshah`
- **Genesis Block**: Same as Shahcoin testnet

### Regtest
- **Network Name**: Shahcoin Regtest
- **Message Start**: `0x53 0x48 0x41 0x52` (SHAR)
- **Default Port**: 18469
- **Bech32 Prefix**: `rshah`
- **Genesis Block**: Same as Shahcoin regtest

## Address Formats

### Mainnet
- **P2PKH**: Starts with `S` (Base58)
- **P2SH**: Starts with `s` (Base58)
- **Private Key**: Starts with `7` (Base58)
- **Bech32**: `shah1...`

### Testnet
- **P2PKH**: Starts with `o` (Base58)
- **P2SH**: Starts with `2` (Base58)
- **Private Key**: Starts with `9` (Base58)
- **Bech32**: `tshah1...`

### Regtest
- **P2PKH**: Starts with `o` (Base58)
- **P2SH**: Starts with `2` (Base58)
- **Private Key**: Starts with `9` (Base58)
- **Bech32**: `rshah1...`

## Consensus Rules
- **Proof of Work**: SHA256D
- **Maximum Block Size**: 1MB (SegWit: 4MB weight units)
- **Maximum Money Supply**: 21,000,000 SHAH
- **Initial Block Reward**: 50 SHAH
- **SegWit**: Enabled at block 481,824
- **Taproot**: Enabled at block 709,632

## DNS Seeds
### Mainnet
- `seed.shah.vip`
- `dnsseed.shahcoin.net`
- `seed.shahcoin.io`

### Testnet
- `testnet-seed.shah.vip`
- `testnet-dnsseed.shahcoin.net`
- `testnet-seed.shahcoin.io`

## Configuration Files

### shahcoin.conf (Mainnet)
```ini
# SHAHCOIN Core configuration file
# Network settings
networkactive=1
listen=1
bind=0.0.0.0:8369
externalip=YOUR_EXTERNAL_IP

# RPC settings
server=1
rpcuser=your_rpc_user
rpcpassword=your_secure_password
rpcbind=127.0.0.1:8368
rpcallowip=127.0.0.1

# Wallet settings
wallet=1
keypool=100

# Mining settings (if mining)
gen=0
genproclimit=1

# Logging
debug=rpc
logips=1
```

### shahcoin.conf (Testnet)
```ini
# SHAHCOIN Core testnet configuration
testnet=1
networkactive=1
listen=1
bind=0.0.0.0:18369
externalip=YOUR_EXTERNAL_IP

server=1
rpcuser=your_rpc_user
rpcpassword=your_secure_password
rpcbind=127.0.0.1:18368
rpcallowip=127.0.0.1

wallet=1
keypool=100

debug=rpc
logips=1
```

### shahcoin.conf (Regtest)
```ini
# SHAHCOIN Core regtest configuration
regtest=1
networkactive=1
listen=1
bind=0.0.0.0:18469

server=1
rpcuser=your_rpc_user
rpcpassword=your_secure_password
rpcbind=127.0.0.1:18468
rpcallowip=127.0.0.1

wallet=1
keypool=100

debug=rpc
logips=1
```

## API Endpoints

### RPC Commands
- `getblockchaininfo` - Get blockchain information
- `getnetworkinfo` - Get network information
- `getwalletinfo` - Get wallet information
- `getnewaddress` - Generate new address
- `sendtoaddress` - Send SHAH to address
- `getbalance` - Get wallet balance

### REST API
- `GET /rest/chaininfo.json` - Chain information
- `GET /rest/block/{hash}.json` - Block information
- `GET /rest/tx/{hash}.json` - Transaction information

## Development Setup

### Building from Source
```bash
# Clone repository
git clone https://github.com/SHAHCoinvip/shahcoin.git
cd shahcoin

# Install dependencies
sudo apt-get install build-essential libtool autotools-dev automake pkg-config bsdmainutils python3

# Configure and build
./autogen.sh
./configure
make -j$(nproc)

# Run tests
make check
```

### Running Nodes
```bash
# Mainnet
./src/shahcoind -daemon

# Testnet
./src/shahcoind -testnet -daemon

# Regtest
./src/shahcoind -regtest -daemon
```

## Security Considerations
- Always use strong RPC passwords
- Restrict RPC access to localhost
- Keep private keys secure
- Use hardware wallets for large amounts
- Regularly update to latest version

## Support
- **Website**: https://shah.vip
- **Documentation**: https://docs.shah.vip
- **GitHub**: https://github.com/SHAHCoinvip/shahcoin
- **Discord**: https://discord.gg/shahcoin
- **Telegram**: https://t.me/shahcoin

## License
MIT License - see LICENSE file for details 
