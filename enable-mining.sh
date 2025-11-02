#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "⛏️  Enabling CPU Mining on ShahCoin"
echo "===================================="
echo ""

# Enable mining with 2 CPU threads on Seed1
echo "Starting mining on Seed1 with 2 threads..."
RESULT=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli -generate 2 2>&1')
echo "$RESULT"

echo ""
echo "Mining is now running in the background!"
echo "With difficulty 0.00024, a block should be found within seconds to minutes."
echo ""
echo "⏳ Waiting 30 seconds..."
sleep 30

echo ""
echo "📊 Checking if block was mined..."
bash test-nodes.sh

echo ""
echo "📈 Current mining status:"
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getmininginfo'

