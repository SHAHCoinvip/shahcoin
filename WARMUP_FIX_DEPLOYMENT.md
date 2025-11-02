# 🔧 ShahCoin Warmup Fix - Deployment Guide

## Problem Identified

The nodes were stuck in "warming up" state because of an issue with `ActivateBestChain` being called on the genesis block during initialization. This prevented `SetRPCWarmupFinished()` from being reached.

## Fix Applied

**File**: `src/init.cpp`

### Changes Made:

1. **Lines 1858-1874**: Modified the `ActivateBestChain` call to only activate when there are blocks beyond genesis:
   - Genesis block tip is already set by `LoadChainTip()`
   - No need to call `ActivateBestChain` for height 0
   - Only call it when `height > 0`

2. **Lines 2248-2255**: Added detailed logging around `SetRPCWarmupFinished()`:
   - Logs when RPC warmup is being finished
   - Shows active tip information
   - Clear confirmation when RPC becomes ready

3. **Lines 2274-2278**: Added final success confirmation logs

## Quick Deployment (Manual)

### Option 1: Quick Deploy via SSH

For each server, run:

```bash
# Seed1
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 << 'EOF'
pkill shahcoind
rm -rf /tmp/shahcoin-build
cd /tmp && mkdir shahcoin-build && cd shahcoin-build
# ... upload and build ...
/usr/local/bin/shahcoind -daemon
EOF
```

### Option 2: Use the Fix Script

From WSL in the shahcoin directory:

```bash
cd /mnt/c/Users/hamid/shahcoin-core/shahcoin
bash scripts/fix-and-redeploy.sh
```

**Note**: This takes 10-15 minutes as it rebuilds shahcoin on each server.

## Quick Manual Rebuild (Fastest)

If you want to rebuild locally and upload just the binaries:

```bash
cd /mnt/c/Users/hamid/shahcoin-core/shahcoin

# Build locally
./autogen.sh
./configure --disable-tests --disable-bench --without-gui
make -j$(nproc)

# Deploy binary to each server
for SERVER in "5.78.123.11:kseeLFTpmqFWawJbHnfk" \
              "91.99.202.196:MPfbgtUfnA9JmfR3xeL3" \
              "91.99.160.98:wuLqsAa4R3kjupKprJb7"; do
    IFS=':' read -r IP PASS <<< "$SERVER"
    echo "Deploying to $IP..."
    
    # Stop daemon
    sshpass -p "$PASS" ssh root@$IP 'pkill shahcoind'
    
    # Upload binaries
    sshpass -p "$PASS" scp src/shahcoind root@$IP:/usr/local/bin/
    sshpass -p "$PASS" scp src/shahcoin-cli root@$IP:/usr/local/bin/
    sshpass -p "$PASS" scp src/shahcoin-tx root@$IP:/usr/local/bin/
    
    # Restart daemon
    sshpass -p "$PASS" ssh root@$IP '/usr/local/bin/shahcoind -daemon'
    
    echo "✅ $IP deployed"
done
```

## Verification

After deployment, check each node:

```bash
# Check if process is running
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 'pgrep shahcoind'

# Check RPC status (wait 30 seconds after start)
sleep 30
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 '/usr/local/bin/shahcoin-cli getblockchaininfo'

# Check debug log for success message
sshpass -p 'kseeLFTpmqFWawJbHnfk' ssh root@5.78.123.11 'grep "SetRPCWarmupFinished" /root/.shahcoin/main/debug.log'
```

### Expected Output

You should see in the debug log:
```
AppInitMain: About to call RPCNotifyBlockChange and SetRPCWarmupFinished...
AppInitMain: Active tip for RPC: <hash> at height 0
AppInitMain: RPCNotifyBlockChange completed
AppInitMain: ✅ SetRPCWarmupFinished() called - RPC should now be ready!
========================================
✅ AppInitMain: ALL INITIALIZATION COMPLETED SUCCESSFULLY
✅ RPC warmup finished - node is READY
========================================
```

And RPC should respond:
```json
{
  "chain": "main",
  "blocks": 0,
  "headers": 0,
  "bestblockhash": "<genesis_hash>",
  "difficulty": 0.000244140625,
  ...
}
```

## What Changed in the Code

### Before (Problematic):
```cpp
// Always called ActivateBestChain, even for genesis
BlockValidationState state;
if (!chainman.ActiveChainstate().ActivateBestChain(state, nullptr)) {
    return InitError(...);
}
```

### After (Fixed):
```cpp
// Only activate if we have blocks beyond genesis
CBlockIndex* pindex = chainman.ActiveChain().Tip();
if (pindex && pindex->nHeight > 0) {
    BlockValidationState state;
    if (!chainman.ActiveChainstate().ActivateBestChain(state, nullptr)) {
        return InitError(...);
    }
} else {
    // Genesis block tip is already set by LoadChainTip()
    std::cout << "Skipping ActivateBestChain for genesis block" << std::endl;
}
```

## Server Details

| Server | IP | Password | Purpose |
|--------|----|---------|---------| 
| Seed1 | 5.78.123.11 | kseeLFTpmqFWawJbHnfk | Seed Node |
| Seed2 | 91.99.202.196 | MPfbgtUfnA9JmfR3xeL3 | Seed Node |
| Explorer | 91.99.160.98 | wuLqsAa4R3kjupKprJb7 | Explorer |

## Timeline

1. **Compile fix locally** (~10 minutes)
2. **Upload binaries to servers** (~1 minute)
3. **Restart nodes** (~30 seconds)
4. **Verify RPC ready** (~30 seconds)

**Total**: ~12 minutes

## Troubleshooting

If nodes still show "warming up":
1. Check if `AppInitMain` completed: `grep "ALL INITIALIZATION COMPLETED" debug.log`
2. Check for errors: `grep -i error debug.log | tail -20`
3. Check if process crashed: `ps aux | grep shahcoin`
4. Check system resources: `free -h && df -h`

## Next Steps After Success

1. ✅ Verify all 3 nodes respond to RPC
2. ✅ Test block generation: `shahcoin-cli generatetoaddress 1 <address>`
3. ✅ Verify network connectivity between nodes
4. ✅ Update GitHub repository with the fix
5. ✅ Document the issue and resolution

---

**Fix Date**: $(date)
**Status**: Ready to deploy

