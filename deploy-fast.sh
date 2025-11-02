#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "⚡ Fast Deploy"

# Seed1
echo "Seed1..."
timeout 5 sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh -o ConnectTimeout=3 root@5.78.123.11 'pkill shahcoind' 2>/dev/null || true
timeout 30 sshpass -p 'kseeLFTpmqFWawJbHnfk' scp -o ConnectTimeout=3 src/shahcoind root@5.78.123.11:/usr/local/bin/ 2>/dev/null
timeout 5 sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh -o ConnectTimeout=3 root@5.78.123.11 '/usr/local/bin/shahcoind -daemon </dev/null >/dev/null 2>&1 &' 2>/dev/null || true
echo "✓ Seed1"

# Seed2
echo "Seed2..."
timeout 5 sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh -o ConnectTimeout=3 root@91.99.202.196 'pkill shahcoind' 2>/dev/null || true
timeout 30 sshpass -p 'MPfbgtUfnA9JmfR3xeL3' scp -o ConnectTimeout=3 src/shahcoind root@91.99.202.196:/usr/local/bin/ 2>/dev/null
timeout 5 sshpass -p 'MPfbgtUfnA9JmfR3xeL3' ssh -o ConnectTimeout=3 root@91.99.202.196 '/usr/local/bin/shahcoind -daemon </dev/null >/dev/null 2>&1 &' 2>/dev/null || true
echo "✓ Seed2"

# Explorer
echo "Explorer..."
timeout 5 sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh -o ConnectTimeout=3 root@91.99.160.98 'pkill shahcoind' 2>/dev/null || true
timeout 30 sshpass -p 'wuLqsAa4R3kjupKprJb7' scp -o ConnectTimeout=3 src/shahcoind root@91.99.160.98:/usr/local/bin/ 2>/dev/null
timeout 5 sshpass -p 'wuLqsAa4R3kjupKprJb7' ssh -o ConnectTimeout=3 root@91.99.160.98 '/usr/local/bin/shahcoind -daemon </dev/null >/dev/null 2>&1 &' 2>/dev/null || true
echo "✓ Explorer"

echo ""
echo "⏳ Waiting 60s..."
sleep 60

echo ""
bash test-nodes.sh

