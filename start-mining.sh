#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "⛏️  Starting ShahCoin with Built-in Mining"
echo "=========================================="
echo ""

echo "Stopping Seed1..."
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 'pkill shahcoind'
sleep 3

echo "Starting Seed1 with mining enabled (2 threads)..."
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoind -daemon -generate=2 </dev/null >/dev/null 2>&1 &'

echo "✓ Mining started with 2 CPU threads"
echo ""
echo "At difficulty 0.00024, a block should be found within seconds to minutes."
echo ""
echo "⏳ Waiting 45 seconds for initialization and mining..."
sleep 45

echo ""
echo "📊 Checking if blocks were mined..."
bash test-nodes.sh

echo ""
echo "📈 Mining info:"
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getmininginfo'

echo ""
echo "💡 Mining is running continuously. Check again in a few minutes:"
echo "   bash test-nodes.sh"

