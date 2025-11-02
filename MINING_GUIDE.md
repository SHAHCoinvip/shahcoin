# ⛏️ ShahCoin Mining Guide

**Complete guide to mining SHAH on mainnet**

---

## 📋 Table of Contents

1. [Mining Overview](#mining-overview)
2. [Solo Mining](#solo-mining)
3. [Pool Mining](#pool-mining)
4. [CPU Mining Configuration](#cpu-mining-configuration)
5. [GPU Mining](#gpu-mining)
6. [Mining Rewards](#mining-rewards)
7. [Troubleshooting](#troubleshooting)

---

## 🎯 Mining Overview

### Network Details
- **Algorithm**: SHA-256d (multi-algorithm support)
- **Block Time**: 2.5 minutes target
- **Block Reward**: 50 SHAH (with halving schedule)
- **Difficulty**: Adjusts dynamically
- **Network Hash Rate**: Check with `getmininginfo`

### Current Network Status
```bash
shahcoin-cli getmininginfo
```

Expected output:
```json
{
  "blocks": 0,
  "difficulty": 0.000244,
  "networkhashps": 0,
  "pooledtx": 0,
  "chain": "main"
}
```

---

## 🖥️ Solo Mining

### Method 1: Built-in CPU Miner

#### Configuration File Method

**Create/Edit**: `~/.shahcoin/shahcoin.conf` (Linux/Mac) or `%APPDATA%\shahcoin\shahcoin.conf` (Windows)

```conf
# ShahCoin Mining Configuration
# Generated for mainnet solo mining

##
## Network Settings
##
server=1
listen=1
daemon=1

# Connect to network
addnode=5.78.123.11:8369
addnode=91.99.202.196:8369
addnode=91.99.160.98:8369

##
## RPC Settings
##
rpcuser=shahcoinminer
rpcpassword=CHANGE_THIS_TO_RANDOM_PASSWORD
rpcport=8368
rpcallowip=127.0.0.1
rpcbind=127.0.0.1

##
## Mining Settings
##
# Note: -generate flag is now used at command line, not in config

##
## Performance
##
dbcache=450
maxmempool=300
maxconnections=125

##
## Logging
##
debug=0
printtoconsole=0
logtimestamps=1
```

#### Starting Mining

**Option A: Command Line (Recommended)**
```bash
# Start daemon with mining enabled (2 CPU threads)
shahcoind -daemon -generate=2

# Or with more threads
shahcoind -daemon -generate=4
```

**Option B: Start daemon then enable mining via RPC**
```bash
# Start daemon
shahcoind -daemon

# Wait for it to sync
sleep 30

# Check status
shahcoin-cli getblockchaininfo

# Note: The old 'generate' RPC command has been replaced
# Use -generate flag when starting shahcoind instead
```

#### Monitoring Mining

```bash
# Check mining status
shahcoin-cli getmininginfo

# Check block count
shahcoin-cli getblockcount

# Check balance
shahcoin-cli getbalance

# Check network hash rate
shahcoin-cli getnetworkhashps

# View recent blocks
shahcoin-cli listsinceblock

# Get wallet info
shahcoin-cli getwalletinfo
```

---

### Method 2: External CPU Miner (cpuminer)

**Install cpuminer:**
```bash
# Linux
sudo apt-get install -y libcurl4-openssl-dev libssl-dev libjansson-dev automake autotools-dev build-essential
git clone https://github.com/pooler/cpuminer
cd cpuminer
./autogen.sh
./configure CFLAGS="-O3"
make

# Start mining
./minerd -a sha256d -o http://127.0.0.1:8368 -u shahcoinminer -p YOUR_RPC_PASSWORD --coinbase-addr=YOUR_SHAH_ADDRESS
```

---

## 🏊 Pool Mining

### Connecting to a Mining Pool

```bash
# Using cpuminer with pool
./minerd -a sha256d \
  -o stratum+tcp://pool.shah.network:3333 \
  -u YOUR_SHAH_ADDRESS \
  -p x
```

### Setting Up Your Own Mining Pool

**Coming Soon**: Full guide for setting up a ShahCoin mining pool using:
- NOMP (Node Open Mining Portal)
- Stratum protocol
- Pool frontend

---

## 🔧 CPU Mining Configuration

### Optimal Thread Count

```bash
# Check your CPU cores
nproc  # Linux/Mac
echo %NUMBER_OF_PROCESSORS%  # Windows

# Use 50-75% of cores for mining
# Example: 8 cores → use 4-6 threads
shahcoind -daemon -generate=6
```

### Performance Tuning

**For VPS/Server:**
```conf
# ~/.shahcoin/shahcoin.conf
dbcache=450          # Reduce for low-memory systems
maxmempool=300       # Memory pool size
maxconnections=125   # Network connections
```

**For Desktop:**
```conf
dbcache=1000
maxmempool=500
maxconnections=125
```

---

## 🎮 GPU Mining

### NVIDIA (CUDA)

**Using ccminer:**
```bash
# Install ccminer
git clone https://github.com/tpruvot/ccminer
cd ccminer
./autogen.sh
./configure
make

# Start mining
./ccminer -a sha256d \
  -o http://127.0.0.1:8368 \
  -u shahcoinminer \
  -p YOUR_RPC_PASSWORD \
  --coinbase-addr=YOUR_SHAH_ADDRESS
```

### AMD (OpenCL)

**Using sgminer:**
```bash
# Start mining
./sgminer -k sha256d \
  -o http://127.0.0.1:8368 \
  -u shahcoinminer \
  -p YOUR_RPC_PASSWORD
```

---

## 💰 Mining Rewards

### Block Rewards Schedule

```
Block Range          Reward      Total Blocks    Total SHAH
0                    8,000,050   1              8,000,050 (Genesis + premine)
1 - 210,000          50          210,000        10,500,000
210,001 - 420,000    25          210,000        5,250,000
420,001 - 630,000    12.5        210,000        2,625,000
... (continues with halvings every 210,000 blocks)
```

### Coinbase Maturity
- **Maturity**: 100 blocks
- Mined coins cannot be spent until 100 confirmations

### Check Your Mining Rewards

```bash
# List all transactions
shahcoin-cli listtransactions

# Check immature balance (mining rewards < 100 confirmations)
shahcoin-cli getbalance "*" 0 true

# Check mature balance
shahcoin-cli getbalance
```

---

## 🔧 Advanced Configuration

### High-Performance Mining Node

```conf
# ~/.shahcoin/shahcoin.conf
# Optimized for dedicated mining

server=1
daemon=1
listen=1

# Mining
# Use -generate flag at startup instead

# Network
addnode=5.78.123.11:8369
addnode=91.99.202.196:8369
addnode=91.99.160.98:8369
maxconnections=200

# Performance
dbcache=2000
maxmempool=500
par=8  # Parallel script verification threads

# RPC
rpcuser=shahcoinminer
rpcpassword=USE_A_STRONG_RANDOM_PASSWORD
rpcport=8368
rpcallowip=127.0.0.1
rpcbind=127.0.0.1
rpcthreads=4
rpcworkqueue=64

# Logging
debug=0
logips=0
logtimestamps=1
```

### Starting High-Performance Miner

```bash
# Start with mining enabled
shahcoind -daemon -generate=8

# Monitor in real-time
tail -f ~/.shahcoin/main/debug.log | grep -i "block\|found\|mined"
```

---

## 🐛 Troubleshooting

### Issue: No Blocks Being Found

**Symptoms:**
```bash
shahcoin-cli getmininginfo
# networkhashps: 0
# difficulty: 0.000244
```

**Solutions:**

1. **Check if mining is actually running:**
```bash
# Mining should be started with -generate flag
ps aux | grep shahcoind
# Should show: shahcoind -daemon -generate=X
```

2. **Verify network connection:**
```bash
shahcoin-cli getpeerinfo
# Should show connected peers
```

3. **Check if blockchain is synced:**
```bash
shahcoin-cli getblockchaininfo
# verificationprogress should be 1.0 or close to it
```

4. **Restart with mining enabled:**
```bash
pkill shahcoind
sleep 5
shahcoind -daemon -generate=4
sleep 30
shahcoin-cli getmininginfo
```

---

### Issue: Mining Rewards Not Showing

**Check immature balance:**
```bash
# Immature = mining rewards waiting for 100 confirmations
shahcoin-cli getbalances

# List all transactions including immature
shahcoin-cli listtransactions "*" 100
```

---

### Issue: Low Hash Rate

**Solutions:**

1. **Increase thread count:**
```bash
# Stop daemon
shahcoin-cli stop

# Restart with more threads
shahcoind -daemon -generate=8  # Adjust based on CPU cores
```

2. **Use external miner:**
- cpuminer (CPU)
- ccminer (NVIDIA GPU)
- sgminer (AMD GPU)

3. **Check CPU usage:**
```bash
top  # Linux/Mac
# Look for shahcoind process
```

---

### Issue: RPC Connection Refused

**Solutions:**

1. **Check if daemon is running:**
```bash
ps aux | grep shahcoind
```

2. **Check RPC credentials:**
```bash
cat ~/.shahcoin/shahcoin.conf | grep rpc
```

3. **Verify RPC is enabled:**
```conf
# Must have in shahcoin.conf:
server=1
rpcuser=shahcoinminer
rpcpassword=your_password
```

---

## 📊 Mining Statistics

### Calculate Expected Time to Find Block

```python
# Python script to estimate block time
difficulty = 0.000244  # Get from: shahcoin-cli getdifficulty
hashrate = 1000000     # Your hash rate in H/s (1 MH/s example)

network_hashrate = difficulty * 2**32 / 150  # 150 = block time target
time_to_block = (network_hashrate / hashrate) * 150

print(f"Expected time to find block: {time_to_block / 60:.2f} minutes")
```

### Monitor Network

```bash
# Create monitoring script
cat > monitor-mining.sh << 'EOF'
#!/bin/bash
while true; do
    clear
    echo "=== ShahCoin Mining Monitor ==="
    echo ""
    echo "Network Info:"
    shahcoin-cli getmininginfo
    echo ""
    echo "Wallet Balance:"
    shahcoin-cli getbalance
    echo ""
    echo "Connections:"
    shahcoin-cli getconnectioncount
    echo ""
    sleep 10
done
EOF

chmod +x monitor-mining.sh
./monitor-mining.sh
```

---

## 🎯 Quick Start Examples

### Example 1: Start Solo Mining on VPS

```bash
# 1. Stop any running instance
shahcoin-cli stop 2>/dev/null || pkill shahcoind

# 2. Start with mining (adjust threads for your CPU)
shahcoind -daemon -generate=2

# 3. Wait for initialization
sleep 45

# 4. Check status
shahcoin-cli getmininginfo
shahcoin-cli getblockchaininfo

# 5. Monitor
tail -f ~/.shahcoin/main/debug.log
```

---

### Example 2: Start Mining on Desktop

```bash
# Windows PowerShell
cd "C:\Program Files\ShahCoin\daemon"
.\shahcoind.exe -daemon -generate=4

# Check status (wait 30 seconds first)
.\shahcoin-cli.exe getmininginfo
```

---

### Example 3: Mine to Specific Address

When you start mining with `-generate`, coins are mined to your default wallet address. To mine to a specific address:

```bash
# 1. Create or get address
shahcoin-cli getnewaddress "mining"

# 2. Set it as default (optional)
# Mining rewards will go to addresses in your wallet

# 3. Start mining
shahcoind -daemon -generate=4
```

---

## 📝 Mining Checklist

- [ ] ShahCoin daemon installed
- [ ] Wallet created and backed up
- [ ] Mining address generated
- [ ] Configuration file created
- [ ] Network connections established (check with `getpeerinfo`)
- [ ] Blockchain synced (check with `getblockchaininfo`)
- [ ] Mining started with `-generate` flag
- [ ] Mining status verified (check with `getmininginfo`)
- [ ] Monitoring set up

---

## 🔗 Additional Resources

- **ShahCoin Explorer**: Check your mining rewards and blocks
- **Network Stats**: Monitor network hash rate and difficulty
- **Mining Pools**: (Coming soon)
- **Mining Calculator**: Estimate profitability

---

## 💡 Tips for Successful Mining

1. **Start Small**: Begin with 1-2 threads to test
2. **Monitor Temperature**: Keep CPU/GPU cool
3. **Stable Connection**: Ensure reliable internet
4. **Backup Wallet**: Regularly backup your wallet.dat
5. **Join Community**: Get help and share experiences
6. **Be Patient**: At low difficulty, blocks come fast; as difficulty rises, it takes longer

---

## 🆘 Getting Help

**If you encounter issues:**

1. Check debug log: `~/.shahcoin/main/debug.log`
2. Verify configuration: `cat ~/.shahcoin/shahcoin.conf`
3. Test RPC: `shahcoin-cli getinfo`
4. Check GitHub issues: https://github.com/SHAHCoinvip/shahcoin/issues
5. Community support: info@shah.vip

---

**Happy Mining!** ⛏️💰

*Remember: Your 8M SHAH premine is already secure in the genesis block. Mining is for earning additional rewards and supporting the network!*

