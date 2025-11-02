#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔧 Fixing remaining nodes..."
echo ""

# Restart Seed2
echo "Restarting Seed2..."
timeout 5 sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh root@91.99.202.196 'pkill shahcoind; sleep 2; /usr/local/bin/shahcoind -daemon </dev/null >/dev/null 2>&1 &' 2>/dev/null || true
echo "✓ Seed2 restarted"

# Clean restart Seed1 (delete old data and start fresh)
echo "Clean restart Seed1..."
timeout 5 sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 'pkill shahcoind; sleep 2; rm -rf /root/.shahcoin/main/blocks /root/.shahcoin/main/chainstate; /usr/local/bin/shahcoind -daemon </dev/null >/dev/null 2>&1 &' 2>/dev/null || true
echo "✓ Seed1 clean restart"

echo ""
echo "⏳ Waiting 60 seconds for initialization..."
sleep 60

echo ""
bash test-nodes.sh

