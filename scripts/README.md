# Shahcoin Scripts

This directory contains utility scripts for SHAHCOIN Core setup and management.

## Setup Script

### `setup-shahcoin.sh`

A comprehensive setup script for SHAHCOIN Core that handles building, configuration, and deployment.

#### Features
- **Cross-platform support**: Linux, macOS, Windows (partial)
- **Automatic dependency installation**: Detects and installs required packages
- **Network configuration**: Supports mainnet, testnet, and regtest
- **Secure configuration**: Generates random RPC credentials
- **Easy deployment**: One-command setup for all networks

#### Usage

```bash
# Make script executable (Linux/macOS)
chmod +x scripts/setup-shahcoin.sh

# Show help
./scripts/setup-shahcoin.sh --help

# Build SHAHCOIN Core
./scripts/setup-shahcoin.sh --build

# Create configuration for mainnet
./scripts/setup-shahcoin.sh --config --network mainnet

# Start mainnet daemon
./scripts/setup-shahcoin.sh --start --network mainnet

# Complete setup for testnet (build + config + start)
./scripts/setup-shahcoin.sh --all --network testnet

# Complete setup for regtest
./scripts/setup-shahcoin.sh --all --network regtest
```

#### Examples

**Quick Start - Mainnet:**
```bash
./scripts/setup-shahcoin.sh --all --network mainnet
```

**Development Setup - Testnet:**
```bash
./scripts/setup-shahcoin.sh --all --network testnet
```

**Testing Setup - Regtest:**
```bash
./scripts/setup-shahcoin.sh --all --network regtest
```

#### Configuration Files

The script creates configuration files in `~/.shahcoin/shahcoin.conf` with:

- **Network-specific ports**: 8369 (mainnet), 18369 (testnet), 18469 (regtest)
- **RPC settings**: Secure credentials and localhost binding
- **Wallet settings**: Enabled with keypool
- **Performance tuning**: Optimized for typical usage
- **Security settings**: Restricted RPC access

#### Network Ports

| Network | P2P Port | RPC Port |
|---------|----------|----------|
| Mainnet | 8369     | 8368     |
| Testnet | 18369    | 18368    |
| Regtest | 18469    | 18468    |

#### Troubleshooting

**Build Issues:**
```bash
# Clean build
make clean
./scripts/setup-shahcoin.sh --build
```

**Permission Issues:**
```bash
# Fix script permissions
chmod +x scripts/setup-shahcoin.sh
```

**Dependency Issues:**
```bash
# Manual dependency installation (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential libtool autotools-dev automake pkg-config bsdmainutils python3 libssl-dev libevent-dev libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev libminiupnpc-dev libzmq3-dev libqrencode-dev libsqlite3-dev
```

**Configuration Issues:**
```bash
# Check configuration
cat ~/.shahcoin/shahcoin.conf

# Restart daemon
pkill shahcoind
./scripts/setup-shahcoin.sh --start --network mainnet
```

#### Security Notes

- RPC credentials are automatically generated
- RPC access is restricted to localhost by default
- Update `externalip` in config file with your actual IP
- Use strong passwords for production deployments
- Keep private keys secure

#### Support

For issues with the setup script:
1. Check the troubleshooting section above
2. Review the configuration file at `~/.shahcoin/shahcoin.conf`
3. Check logs at `~/.shahcoin/debug.log`
4. Ensure all dependencies are installed correctly 