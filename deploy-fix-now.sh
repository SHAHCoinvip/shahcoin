#!/bin/bash
# ShahCoin Warmup Fix - Quick Deploy
# Run this directly in WSL terminal

set -e

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔧 ShahCoin Warmup Fix Deployment"
echo "=================================="
echo ""

# Check if we're in the right directory
if [ ! -f "src/shahcoind" ]; then
    echo "❌ Error: Run this from /mnt/c/Users/hamid/shahcoin-core/shahcoin"
    exit 1
fi

# Check if binary was updated today
echo "📦 Checking if build is ready..."
BINARY_DATE=$(stat -c '%y' src/shahcoind 2>/dev/null | cut -d' ' -f1)
TODAY=$(date +%Y-%m-%d)

if [ "$BINARY_DATE" != "$TODAY" ]; then
    echo "⏳ Binary not rebuilt yet. Building now..."
    cd src
    rm -f shahcoind libshahcoin_node.a
    make shahcoind -j4
    cd ..
    echo "✅ Build complete!"
else
    echo "✅ Binary already built today"
fi

echo ""
echo "🚀 Deploying to VPS nodes..."
echo ""

# Server details
SEED1_IP="5.78.123.11"
SEED1_PASS="kseeLFTpmqFWawJbHnfk"

SEED2_IP="91.99.202.196"
SEED2_PASS="MPfbgtUfnA9JmfR3xeL3"

EXPLORER_IP="91.99.160.98"
EXPLORER_PASS="wuLqsAa4R3kjupKprJb7"

deploy_node() {
    local NAME=$1
    local IP=$2
    local PASS=$3
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📦 $NAME ($IP)"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    # Stop daemon
    echo "  🛑 Stopping daemon..."
    sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no root@$IP 'pkill shahcoind || true' 2>/dev/null
    sleep 2
    
    # Upload binary
    echo "  📤 Uploading binary..."
    sshpass -p "$PASS" scp -o StrictHostKeyChecking=no src/shahcoind root@$IP:/usr/local/bin/ 2>/dev/null
    sshpass -p "$PASS" scp -o StrictHostKeyChecking=no src/shahcoin-cli root@$IP:/usr/local/bin/ 2>/dev/null
    
    # Set permissions and clean locks
    sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no root@$IP 'chmod +x /usr/local/bin/shahcoin* && rm -f /root/.shahcoin/main/.lock /root/.shahcoin/main/.cookie' 2>/dev/null
    
    # Start daemon
    echo "  ▶️  Starting daemon..."
    sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no root@$IP '/usr/local/bin/shahcoind -daemon' 2>/dev/null
    
    echo "  ✅ $NAME deployed"
    echo ""
}

# Deploy to all nodes
deploy_node "Seed1" "$SEED1_IP" "$SEED1_PASS"
deploy_node "Seed2" "$SEED2_IP" "$SEED2_PASS"
deploy_node "Explorer" "$EXPLORER_IP" "$EXPLORER_PASS"

echo "⏳ Waiting 30 seconds for nodes to initialize..."
sleep 30

echo ""
echo "🔍 Testing Nodes..."
echo "==================="
echo ""

test_node() {
    local NAME=$1
    local IP=$2
    local PASS=$3
    
    echo "📊 $NAME:"
    
    # Check process
    PID=$(sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no root@$IP 'pgrep shahcoind' 2>/dev/null || echo "")
    if [ -n "$PID" ]; then
        echo "  ✅ Process running (PID: $PID)"
    else
        echo "  ❌ Process NOT running"
        return
    fi
    
    # Check RPC
    RPC=$(sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no root@$IP '/usr/local/bin/shahcoin-cli getblockchaininfo 2>&1' || echo "")
    if echo "$RPC" | grep -q '"blocks"'; then
        BLOCKS=$(echo "$RPC" | grep -o '"blocks":[0-9]*' | cut -d: -f2)
        echo "  ✅ RPC READY - blocks: $BLOCKS"
        echo "  🎉 SUCCESS! Node is working!"
    elif echo "$RPC" | grep -q "warming up"; then
        echo "  ⚠️  Still warming up (waiting longer...)"
    else
        echo "  ❌ RPC error: $(echo "$RPC" | head -1)"
    fi
    
    echo ""
}

test_node "Seed1" "$SEED1_IP" "$SEED1_PASS"
test_node "Seed2" "$SEED2_IP" "$SEED2_PASS"
test_node "Explorer" "$EXPLORER_IP" "$EXPLORER_PASS"

echo "=================================="
echo "✅ Deployment Complete!"
echo "=================================="
echo ""
echo "📝 Check logs:"
echo "  sshpass -p '$SEED1_PASS' ssh root@$SEED1_IP 'tail -f /root/.shahcoin/main/debug.log'"
echo ""

