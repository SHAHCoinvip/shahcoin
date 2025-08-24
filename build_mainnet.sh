#!/bin/bash

# SHAHCOIN Mainnet Build Script
# Finalizes the SHAHCOIN blockchain for mainnet launch
# Project: /mnt/c/Users/hamid/shahcoin-core/shahcoin

echo "🚀 SHAHCOIN Mainnet Build Finalization"
echo "======================================"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
make clean
make distclean

# Configure for mainnet-only build
echo "⚙️  Configuring for mainnet-only build..."
./autogen.sh
./configure \
    --enable-wallet \
    --with-gui=qt5 \
    --disable-tests \
    --disable-bench \
    --disable-gui-tests \
    --with-boost-libdir=/usr/lib/x86_64-linux-gnu \
    --prefix=/usr/local

# Build the binaries
echo "🔨 Building SHAHCOIN binaries..."
make -j$(nproc)

# Verify binaries
echo "✅ Verifying binaries..."
if [ -f "src/shahcoind" ]; then
    echo "✅ shahcoind built successfully"
else
    echo "❌ shahcoind build failed"
    exit 1
fi

if [ -f "src/qt/shahcoin-qt" ]; then
    echo "✅ shahcoin-qt built successfully"
else
    echo "❌ shahcoin-qt build failed"
    exit 1
fi

if [ -f "src/shahcoin-cli" ]; then
    echo "✅ shahcoin-cli built successfully"
else
    echo "❌ shahcoin-cli build failed"
    exit 1
fi

echo "🎉 SHAHCOIN Mainnet Build Complete!"
echo "=================================="
echo "Binaries ready for mainnet launch:"
echo "- shahcoind (daemon)"
echo "- shahcoin-qt (GUI wallet)"
echo "- shahcoin-cli (command line interface)"
echo ""
echo "Next steps:"
echo "1. Test genesis block validation"
echo "2. Import premine private key"
echo "3. Verify 8M SHAH balance"
echo "4. Test staking functionality"
echo "5. Launch mainnet"
