#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔍 Testing ShahCoin Nodes"
echo "========================="
echo ""

# Seed1
echo "📊 Seed1 (5.78.123.11):"
PID=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh -o StrictHostKeyChecking=no root@5.78.123.11 'pgrep shahcoind' 2>/dev/null || echo "")
if [ -n "$PID" ]; then
    echo "  ✅ Process running (PID: $PID)"
    RPC=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh -o StrictHostKeyChecking=no root@5.78.123.11 '/usr/local/bin/shahcoin-cli getblockchaininfo 2>&1')
    if echo "$RPC" | grep -q '"blocks"'; then
        echo "  🎉 RPC READY!"
        echo "$RPC" | grep -E '"blocks"|"bestblockhash"|"initialblockdownload"'
    else
        echo "  Status: $RPC"
    fi
else
    echo "  ❌ Not running"
fi

echo ""

# Seed2
echo "📊 Seed2 (91.99.202.196):"
PID=$(sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh -o StrictHostKeyChecking=no root@91.99.202.196 'pgrep shahcoind' 2>/dev/null || echo "")
if [ -n "$PID" ]; then
    echo "  ✅ Process running (PID: $PID)"
    RPC=$(sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh -o StrictHostKeyChecking=no root@91.99.202.196 '/usr/local/bin/shahcoin-cli getblockchaininfo 2>&1')
    if echo "$RPC" | grep -q '"blocks"'; then
        echo "  🎉 RPC READY!"
        echo "$RPC" | grep -E '"blocks"|"bestblockhash"|"initialblockdownload"'
    else
        echo "  Status: $RPC"
    fi
else
    echo "  ❌ Not running"
fi

echo ""

# Explorer
echo "📊 Explorer (91.99.160.98):"
PID=$(sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh -o StrictHostKeyChecking=no root@91.99.160.98 'pgrep shahcoind' 2>/dev/null || echo "")
if [ -n "$PID" ]; then
    echo "  ✅ Process running (PID: $PID)"
    RPC=$(sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh -o StrictHostKeyChecking=no root@91.99.160.98 '/usr/local/bin/shahcoin-cli getblockchaininfo 2>&1')
    if echo "$RPC" | grep -q '"blocks"'; then
        echo "  🎉 RPC READY!"
        echo "$RPC" | grep -E '"blocks"|"bestblockhash"|"initialblockdownload"'
    else
        echo "  Status: $RPC"
    fi
else
    echo "  ❌ Not running"
fi

echo ""
echo "✅ Test complete"

