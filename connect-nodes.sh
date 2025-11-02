#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔗 Connecting ShahCoin Nodes"
echo "=============================="
echo ""

# Connect Seed1 to Seed2
echo "Connecting Seed1 → Seed2..."
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli addnode "91.99.202.196:8369" "add"'
echo "✓"

# Connect Seed1 to Explorer
echo "Connecting Seed1 → Explorer..."
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli addnode "91.99.160.98:8369" "add"'
echo "✓"

# Connect Seed2 to Seed1
echo "Connecting Seed2 → Seed1..."
sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh root@91.99.202.196 '/usr/local/bin/shahcoin-cli addnode "5.78.123.11:8369" "add"'
echo "✓"

# Connect Seed2 to Explorer
echo "Connecting Seed2 → Explorer..."
sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh root@91.99.202.196 '/usr/local/bin/shahcoin-cli addnode "91.99.160.98:8369" "add"'
echo "✓"

# Connect Explorer to Seed1
echo "Connecting Explorer → Seed1..."
sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh root@91.99.160.98 '/usr/local/bin/shahcoin-cli addnode "5.78.123.11:8369" "add"'
echo "✓"

# Connect Explorer to Seed2
echo "Connecting Explorer → Seed2..."
sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh root@91.99.160.98 '/usr/local/bin/shahcoin-cli addnode "91.99.202.196:8369" "add"'
echo "✓"

echo ""
echo "⏳ Waiting 10 seconds for connections..."
sleep 10

echo ""
echo "📊 Checking peer connections:"
echo ""
echo "Seed1:"
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getconnectioncount'

echo ""
echo "Seed2:"
sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh root@91.99.202.196 '/usr/local/bin/shahcoin-cli getconnectioncount'

echo ""
echo "Explorer:"
sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh root@91.99.160.98 '/usr/local/bin/shahcoin-cli getconnectioncount'

echo ""
echo "✅ Network connected!"

