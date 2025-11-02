#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "⛏️  Mining First Block on ShahCoin Mainnet"
echo "=========================================="
echo ""

# Get mining address (wallet already exists)
echo "📍 Getting mining address..."
ADDRESS=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getnewaddress 2>&1')
echo "Address: $ADDRESS"
echo ""

# Check current difficulty
echo "📊 Current network difficulty..."
DIFFICULTY=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getdifficulty 2>&1')
echo "Difficulty: $DIFFICULTY"
echo ""

# Try to generate with more nonce space
echo "⛏️  Attempting to mine block 1 (this may take a while on mainnet)..."
echo "Note: Mining on mainnet difficulty requires actual PoW computation"
echo ""

# Generate block - try up to 1 billion nonces
RESULT=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 "/usr/local/bin/shahcoin-cli generatetoaddress 1 \"$ADDRESS\" 1000000000 2>&1")

if echo "$RESULT" | grep -q '\['; then
    echo "✅ Block generated!"
    echo "$RESULT"
else
    echo "❌ Mining failed or timed out"
    echo "$RESULT"
    echo ""
    echo "💡 On mainnet, mining requires significant PoW."
    echo "   Options:"
    echo "   1. Use setgenerate true to mine in background"
    echo "   2. Connect external miners"
    echo "   3. Lower difficulty for testing (requires consensus change)"
fi

echo ""
echo "📊 Checking blockchain status..."
bash test-nodes.sh

