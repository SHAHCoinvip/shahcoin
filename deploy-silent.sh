#!/bin/bash
# Silent deployment - doesn't wait for startup logs
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🚀 Quick Silent Deployment"
echo "=========================="

SERVERS=(
    "Seed1:5.78.123.11:kseeLFTpmqFWawJbHnfk"
    "Seed2:91.99.202.196:MPfbgtUfnA9JmfR3xeL3"
    "Explorer:91.99.160.98:wuLqsAa4R3kjupKprJb7"
)

for server_info in "${SERVERS[@]}"; do
    IFS=':' read -r NAME IP PASS <<< "$server_info"
    
    echo ""
    echo "📦 $NAME ($IP)"
    
    # Stop, upload, start - all without waiting
    sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10 root@$IP 'pkill shahcoind; sleep 1' 2>/dev/null
    
    sshpass -p "$PASS" scp -o StrictHostKeyChecking=no -q src/shahcoind src/shahcoin-cli root@$IP:/usr/local/bin/ 2>/dev/null
    
    sshpass -p "$PASS" ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10 root@$IP 'chmod +x /usr/local/bin/shahcoin* && rm -f /root/.shahcoin/main/.lock && nohup /usr/local/bin/shahcoind -daemon > /dev/null 2>&1 &' 2>/dev/null
    
    echo "  ✅ Deployed"
done

echo ""
echo "⏳ Waiting 45 seconds for initialization..."
sleep 45

echo ""
echo "🔍 Testing nodes..."
bash test-nodes.sh

