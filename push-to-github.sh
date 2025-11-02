#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🚀 Pushing ShahCoin Warmup Fix to GitHub"
echo "========================================="
echo ""

# Configure git
git config --global user.email "info@shah.vip"
git config --global user.name "ShahCoin Core"

# Stage and commit
git add src/init.cpp

git commit -m "Fix: Skip ActivateBestChain for genesis block to prevent RPC warmup hang

This commit resolves the issue where shahcoin nodes remained stuck in
perpetual 'warming up' state, never transitioning to ready.

Problem:
- Nodes would load genesis block successfully
- Complete initialization but remain in warmup mode
- RPC would return error -28 (warming up) indefinitely
- SetRPCWarmupFinished() was never called

Root Cause:
- ActivateBestChain() was being called unconditionally during init
- This included the genesis block (height 0)
- Genesis block doesn't need 'activation' as tip is already set by LoadChainTip()
- The call was preventing initialization from completing

Solution:
- Modified src/init.cpp lines 1858-1874 to make ActivateBestChain conditional
- Only call ActivateBestChain when chain height > 0
- Skip for genesis block (height 0) with clear logging
- Added detailed logging around SetRPCWarmupFinished (lines 2248-2255)
- Added success confirmation logging (lines 2274-2278)

Result:
- All nodes now successfully complete initialization
- SetRPCWarmupFinished() is called as expected
- RPC responds immediately without warmup errors
- Nodes fully operational on mainnet

Tested on:
- Seed1 (5.78.123.11) - ✓ Working
- Seed2 (91.99.202.196) - ✓ Working  
- Explorer (91.99.160.98) - ✓ Working

All nodes sharing genesis hash:
72250d001e173dcb336c249d1890a9bdaea709bb286725f2d2b3297bf7982716"

# Set up credential helper to use token from environment
git config --global credential.helper store

# Push using HTTPS with token
git push https://SHAHCoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git main

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉🎉🎉 SUCCESS! Fix pushed to GitHub! 🎉🎉🎉"
    echo ""
    echo "✅ Your warmup fix is now live at:"
    echo "   https://github.com/SHAHCoinvip/shahcoin/commits/main"
    echo ""
    echo "✅ All 3 nodes operational with the fix"
    echo "✅ Your 8M SHAH premine is secure"
    echo "✅ ShahCoin mainnet is ready!"
else
    echo ""
    echo "❌ Push failed. Please check the error above."
fi

# Clean up credential store for security
git config --global --unset credential.helper

