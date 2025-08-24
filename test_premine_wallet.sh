#!/bin/bash

# SHAHCOIN Premine Wallet Test Script
# Tests the premine wallet functionality after build

echo "🔐 SHAHCOIN Premine Wallet Test"
echo "==============================="

# Premine wallet details
PREMINE_ADDRESS="SVUNaktXHkoRtoZeScGxjMxFVDVMn4xKTG"
PREMINE_PRIVATE_KEY="VPDArztHSodZuqn8rVsf2aakrePbmG2BZqZf9D5ZKpEEynocS4Pm"
PREMINE_AMOUNT="8000000.00000000"

echo "📋 Premine Configuration:"
echo "Address: $PREMINE_ADDRESS"
echo "Amount: $PREMINE_AMOUNT SHAH"
echo ""

# Test 1: Start daemon and verify genesis block
echo "🧪 Test 1: Genesis Block Validation"
echo "Starting shahcoind daemon..."
./src/shahcoind -daemon -testnet=0 -regtest=0

# Wait for daemon to start
sleep 5

# Check if daemon is running
if pgrep -x "shahcoind" > /dev/null; then
    echo "✅ Daemon started successfully"
else
    echo "❌ Daemon failed to start"
    exit 1
fi

# Test 2: Verify genesis block
echo ""
echo "🧪 Test 2: Genesis Block Verification"
GENESIS_HASH=$(./src/shahcoin-cli getblockhash 0)
echo "Genesis Block Hash: $GENESIS_HASH"

if [ ! -z "$GENESIS_HASH" ]; then
    echo "✅ Genesis block found"
else
    echo "❌ Genesis block not found"
    exit 1
fi

# Test 3: Check premine balance
echo ""
echo "🧪 Test 3: Premine Balance Verification"
PREMINE_BALANCE=$(./src/shahcoin-cli getreceivedbyaddress "$PREMINE_ADDRESS")

if [ "$PREMINE_BALANCE" = "$PREMINE_AMOUNT" ]; then
    echo "✅ Premine balance correct: $PREMINE_BALANCE SHAH"
else
    echo "❌ Premine balance incorrect: $PREMINE_BALANCE SHAH (expected: $PREMINE_AMOUNT)"
    exit 1
fi

# Test 4: Import private key
echo ""
echo "🧪 Test 4: Private Key Import"
IMPORT_RESULT=$(./src/shahcoin-cli importprivkey "$PREMINE_PRIVATE_KEY" "premine" true)

if [ $? -eq 0 ]; then
    echo "✅ Private key imported successfully"
else
    echo "❌ Private key import failed"
    exit 1
fi

# Test 5: Verify wallet balance
echo ""
echo "🧪 Test 5: Wallet Balance Verification"
WALLET_BALANCE=$(./src/shahcoin-cli getbalance)

if [ "$WALLET_BALANCE" = "$PREMINE_AMOUNT" ]; then
    echo "✅ Wallet balance correct: $WALLET_BALANCE SHAH"
else
    echo "❌ Wallet balance incorrect: $WALLET_BALANCE SHAH (expected: $PREMINE_AMOUNT)"
    exit 1
fi

# Test 6: Test staking functionality
echo ""
echo "🧪 Test 6: Staking Functionality"
STAKING_INFO=$(./src/shahcoin-cli getstakinginfo)

if [ $? -eq 0 ]; then
    echo "✅ Staking info retrieved successfully"
    echo "Staking Info: $STAKING_INFO"
else
    echo "❌ Staking info retrieval failed"
    exit 1
fi

# Test 7: Test hybrid consensus RPC
echo ""
echo "🧪 Test 7: Hybrid Consensus RPC"
ALGO_INFO=$(./src/shahcoin-cli getalgoinfo)

if [ $? -eq 0 ]; then
    echo "✅ Hybrid consensus RPC working"
    echo "Algorithm Info: $ALGO_INFO"
else
    echo "❌ Hybrid consensus RPC failed"
    exit 1
fi

# Stop daemon
echo ""
echo "🛑 Stopping daemon..."
./src/shahcoin-cli stop

echo ""
echo "🎉 All Premine Wallet Tests Passed!"
echo "==================================="
echo "✅ Genesis block validated"
echo "✅ Premine balance verified"
echo "✅ Private key imported"
echo "✅ Wallet balance correct"
echo "✅ Staking functionality working"
echo "✅ Hybrid consensus operational"
echo ""
echo "🚀 SHAHCOIN is ready for mainnet launch!"
