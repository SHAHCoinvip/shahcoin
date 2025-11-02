# ⛏️ Quick Start: Mining ShahCoin

Get started mining SHAH in under 5 minutes!

---

## 🚀 Fastest Way to Start Mining

### Step 1: Install ShahCoin

Download from: https://shah.vip/download

Or build from source:
```bash
git clone https://github.com/SHAHCoinvip/shahcoin.git
cd shahcoin
./autogen.sh
./configure
make
sudo make install
```

---

### Step 2: Create Configuration

Create `~/.shahcoin/shahcoin.conf`:

```conf
server=1
daemon=1
listen=1

# Connect to network
addnode=5.78.123.11:8369
addnode=91.99.202.196:8369
addnode=91.99.160.98:8369

# RPC access
rpcuser=miner
rpcpassword=CHANGE_THIS_PASSWORD
rpcport=8368
rpcallowip=127.0.0.1

# Performance
dbcache=450
maxconnections=125
```

---

### Step 3: Start Mining!

```bash
# Start with 2 CPU threads
shahcoind -daemon -generate=2

# Wait 30 seconds for initialization
sleep 30

# Check status
shahcoin-cli getmininginfo
```

---

## ✅ Verify Mining is Working

```bash
# Should show mining info
$ shahcoin-cli getmininginfo
{
  "blocks": 0,
  "difficulty": 0.000244,
  "networkhashps": 0,
  "pooledtx": 0,
  "chain": "main"
}

# Check connections (should be > 0)
$ shahcoin-cli getconnectioncount
3

# Check if blockchain is synced
$ shahcoin-cli getblockchaininfo
```

---

## 💰 Check Your Mining Rewards

```bash
# Check balance
shahcoin-cli getbalance

# List transactions
shahcoin-cli listtransactions

# Check immature balance (mining rewards waiting for 100 confirmations)
shahcoin-cli getbalances
```

---

## 📊 Monitor Mining

```bash
# Watch debug log
tail -f ~/.shahcoin/main/debug.log

# Create monitoring script
cat > monitor.sh << 'EOF'
#!/bin/bash
while true; do
    clear
    echo "=== ShahCoin Mining Status ==="
    echo ""
    shahcoin-cli getmininginfo
    echo ""
    echo "Balance:"
    shahcoin-cli getbalance
    echo ""
    sleep 10
done
EOF

chmod +x monitor.sh
./monitor.sh
```

---

## 🎯 Optimal Settings

### CPU Core Count

```bash
# For 2 cores
shahcoind -daemon -generate=1

# For 4 cores  
shahcoind -daemon -generate=2

# For 8 cores
shahcoind -daemon -generate=4

# For 16+ cores
shahcoind -daemon -generate=8
```

**Rule of thumb**: Use 50-75% of your CPU cores

---

## 🐛 Troubleshooting

### Problem: No blocks being found

```bash
# Check if mining is active
ps aux | grep shahcoind

# Should show: shahcoind -daemon -generate=X

# If not, restart with mining:
shahcoin-cli stop
shahcoind -daemon -generate=2
```

### Problem: No connections

```bash
# Add seed nodes manually
shahcoin-cli addnode "5.78.123.11:8369" "add"
shahcoin-cli addnode "91.99.202.196:8369" "add"
shahcoin-cli addnode "91.99.160.98:8369" "add"

# Check connections
shahcoin-cli getpeerinfo
```

### Problem: RPC error -28 "warming up"

**This issue was fixed in commit d340bd4**

If you see this, update to the latest version:
```bash
cd shahcoin
git pull
make clean
make
sudo make install
```

---

## 📖 More Information

- **Full Mining Guide**: See `MINING_GUIDE.md`
- **Configuration Options**: See `MINING_GUIDE.md#advanced-configuration`
- **GPU Mining**: See `MINING_GUIDE.md#gpu-mining`
- **Pool Mining**: See `MINING_GUIDE.md#pool-mining`

---

## 💡 Quick Tips

1. **Start small** - Test with 1-2 threads first
2. **Monitor temperature** - Keep your system cool
3. **Be patient** - At low difficulty, blocks come fast
4. **Backup wallet** - Save your `wallet.dat` regularly
5. **Join network** - More nodes = stronger network

---

## 🆘 Need Help?

- **Documentation**: `MINING_GUIDE.md`
- **Technical Fix Docs**: `WARMUP_FIX_DOCUMENTATION.md`
- **GitHub Issues**: https://github.com/SHAHCoinvip/shahcoin/issues
- **Email**: info@shah.vip
- **Website**: https://shah.vip

---

**Happy Mining!** ⛏️

*Current network difficulty: 0.000244 (very easy - great time to start mining!)*

