#!/bin/bash

# SHAHCOIN Core Setup Script
# This script helps set up SHAHCOIN Core for different networks

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Function to install dependencies
install_dependencies() {
    local os=$(detect_os)
    
    print_header "Installing Dependencies"
    
    case $os in
        "linux")
            if command_exists apt-get; then
                print_status "Installing dependencies via apt-get..."
                sudo apt-get update
                sudo apt-get install -y build-essential libtool autotools-dev automake pkg-config bsdmainutils python3 libssl-dev libevent-dev libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev libminiupnpc-dev libzmq3-dev libqrencode-dev libsqlite3-dev
            elif command_exists yum; then
                print_status "Installing dependencies via yum..."
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y libtool autoconf automake pkg-config python3 openssl-devel libevent-devel boost-devel miniupnpc-devel zeromq-devel qrencode-devel sqlite-devel
            elif command_exists dnf; then
                print_status "Installing dependencies via dnf..."
                sudo dnf groupinstall -y "Development Tools"
                sudo dnf install -y libtool autoconf automake pkg-config python3 openssl-devel libevent-devel boost-devel miniupnpc-devel zeromq-devel qrencode-devel sqlite-devel
            else
                print_error "Unsupported package manager. Please install dependencies manually."
                exit 1
            fi
            ;;
        "macos")
            if command_exists brew; then
                print_status "Installing dependencies via Homebrew..."
                brew install automake berkeley-db4 libtool boost miniupnpc pkg-config python3 qrencode qt5 libevent sqlite3 zeromq
            else
                print_error "Homebrew not found. Please install Homebrew first: https://brew.sh/"
                exit 1
            fi
            ;;
        "windows")
            print_warning "Windows setup requires manual dependency installation."
            print_status "Please install Visual Studio Build Tools and required libraries."
            ;;
        *)
            print_error "Unsupported operating system: $os"
            exit 1
            ;;
    esac
}

# Function to build SHAHCOIN Core
build_shahcoin() {
    print_header "Building SHAHCOIN Core"
    
    if [ ! -f "./autogen.sh" ]; then
        print_error "autogen.sh not found. Please run this script from the Shahcoin source directory."
        exit 1
    fi
    
    print_status "Running autogen.sh..."
    ./autogen.sh
    
    print_status "Configuring build..."
    ./configure --enable-wallet --with-gui=qt5
    
    print_status "Building SHAHCOIN Core (this may take a while)..."
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
    
    print_status "Build completed successfully!"
}

# Function to create configuration file
create_config() {
    local network=$1
    local config_dir="$HOME/.shahcoin"
    local config_file="$config_dir/shahcoin.conf"
    
    print_header "Creating Configuration for $network"
    
    # Create config directory
    mkdir -p "$config_dir"
    
    # Determine network-specific settings
    case $network in
        "mainnet")
            port=8369
            rpcport=8368
            network_flag=""
            ;;
        "testnet")
            port=18369
            rpcport=18368
            network_flag="testnet=1"
            ;;
        "regtest")
            port=18469
            rpcport=18468
            network_flag="regtest=1"
            ;;
        *)
            print_error "Invalid network: $network"
            exit 1
            ;;
    esac
    
    # Generate random RPC credentials
    local rpc_user="shahcoin_user_$(openssl rand -hex 8)"
    local rpc_password="$(openssl rand -hex 32)"
    
    # Create configuration file
    cat > "$config_file" << EOF
# SHAHCOIN Core Configuration File
# Generated by setup script on $(date)

# Network settings
$network_flag
networkactive=1
listen=1
bind=0.0.0.0:$port
externalip=YOUR_EXTERNAL_IP

# RPC settings
server=1
rpcuser=$rpc_user
rpcpassword=$rpc_password
rpcbind=127.0.0.1:$rpcport
rpcallowip=127.0.0.1

# Wallet settings
wallet=1
keypool=100

# Mining settings (disabled by default)
gen=0
genproclimit=1

# Logging
debug=rpc
logips=1

# Performance
dbcache=450
maxorphantx=10
maxmempool=50

# Security
rpcworkqueue=16
rpcthreads=16
EOF
    
    print_status "Configuration file created: $config_file"
    print_status "RPC User: $rpc_user"
    print_status "RPC Password: $rpc_password"
    print_warning "Please update externalip in the config file with your actual IP address."
}

# Function to start Shahcoin daemon
start_daemon() {
    local network=$1
    
    print_header "Starting Shahcoin Daemon ($network)"
    
    local daemon_cmd="./src/shahcoind"
    case $network in
        "mainnet")
            daemon_cmd="$daemon_cmd"
            ;;
        "testnet")
            daemon_cmd="$daemon_cmd -testnet"
            ;;
        "regtest")
            daemon_cmd="$daemon_cmd -regtest"
            ;;
    esac
    
    daemon_cmd="$daemon_cmd -daemon"
    
    print_status "Starting daemon with command: $daemon_cmd"
    $daemon_cmd
    
    print_status "Shahcoin daemon started in background."
    print_status "Check logs with: tail -f ~/.shahcoin/debug.log"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -n, --network NETWORK   Set network (mainnet, testnet, regtest)"
    echo "  -b, --build             Build SHAHCOIN Core"
    echo "  -c, --config            Create configuration file"
    echo "  -s, --start             Start daemon"
    echo "  -a, --all               Do everything (build, config, start)"
    echo ""
    echo "Examples:"
    echo "  $0 -a -n mainnet        Build, configure, and start mainnet"
    echo "  $0 -b                   Build only"
    echo "  $0 -c -n testnet        Create testnet configuration"
    echo "  $0 -s -n regtest        Start regtest daemon"
}

# Main script
main() {
    local network="mainnet"
    local do_build=false
    local do_config=false
    local do_start=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -n|--network)
                network="$2"
                shift 2
                ;;
            -b|--build)
                do_build=true
                shift
                ;;
            -c|--config)
                do_config=true
                shift
                ;;
            -s|--start)
                do_start=true
                shift
                ;;
            -a|--all)
                do_build=true
                do_config=true
                do_start=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    # Validate network
    case $network in
        mainnet|testnet|regtest)
            ;;
        *)
            print_error "Invalid network: $network"
            print_error "Valid networks: mainnet, testnet, regtest"
            exit 1
            ;;
    esac
    
    print_header "SHAHCOIN Core Setup Script"
    print_status "Network: $network"
    
    # Execute requested actions
    if [ "$do_build" = true ]; then
        install_dependencies
        build_shahcoin
    fi
    
    if [ "$do_config" = true ]; then
        create_config "$network"
    fi
    
    if [ "$do_start" = true ]; then
        start_daemon "$network"
    fi
    
    print_header "Setup Complete!"
    print_status "SHAHCOIN Core is ready for $network"
    print_status "Configuration: ~/.shahcoin/shahcoin.conf"
    print_status "Data directory: ~/.shahcoin/"
    print_status "Log file: ~/.shahcoin/debug.log"
}

# Run main function with all arguments
main "$@" 