#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔍 Final Mining Diagnostic"
echo "=========================="
echo ""

echo "1️⃣ Available generate commands:"
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli help' | grep -E 'generat|mine'
echo ""

echo "2️⃣ Current block template:"
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getblocktemplate "{\"rules\":[\"segwit\"]}" 2>&1' | head -30
echo ""

echo "3️⃣ Try legacy generate command:"
LEGACY=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli generate 1 2>&1')
echo "$LEGACY"
echo ""

if echo "$LEGACY" | grep -q "Method not found"; then
    echo "4️⃣ Legacy generate not available, trying generatetoaddress with unlimited tries:"
    ADDRESS=$(sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getnewaddress')
    echo "Mining to: $ADDRESS"
    
    # Try with explicit unlimited (or very high) max tries
    sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 "/usr/local/bin/shahcoin-cli generatetoaddress 1 \"$ADDRESS\" 4294967295"
else
    echo "✅ Block might have been generated!"
fi

echo ""
echo "5️⃣ Final status check:"
bash test-nodes.sh

