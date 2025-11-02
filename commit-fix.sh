#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "📦 Committing ShahCoin Warmup Fix to GitHub"
echo "============================================"
echo ""

# Configure git
echo "1️⃣ Configuring git..."
git config --global user.email "info@shah.vip"
git config --global user.name "ShahCoin Core"
echo "✓ Git configured"
echo ""

# Check status
echo "2️⃣ Checking git status..."
git status
echo ""

# Stage the changes
echo "3️⃣ Staging changes..."
git add src/init.cpp
echo "✓ Staged src/init.cpp"
echo ""

# Show what will be committed
echo "4️⃣ Changes to be committed:"
git diff --cached --stat
echo ""

# Commit
echo "5️⃣ Creating commit..."
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

echo ""
echo "✓ Commit created successfully!"
echo ""

# Show commit
echo "6️⃣ Commit details:"
git log -1 --stat
echo ""

echo "7️⃣ Ready to push to GitHub"
echo ""
echo "⚠️  IMPORTANT: GitHub Authentication"
echo "======================================"
echo ""
echo "GitHub no longer accepts password authentication."
echo "You need a Personal Access Token:"
echo ""
echo "1. Go to: https://github.com/settings/tokens"
echo "2. Click 'Generate new token (classic)'"
echo "3. Give it a name like 'ShahCoin Development'"
echo "4. Select 'repo' permissions"
echo "5. Click 'Generate token'"
echo "6. COPY THE TOKEN (you won't see it again!)"
echo ""
echo "When prompted for password, paste the token instead."
echo ""
read -p "Press Enter when you have your token ready..."
echo ""

# Push
echo "8️⃣ Pushing to GitHub..."
git push origin main

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 SUCCESS! Fix pushed to GitHub!"
    echo ""
    echo "View your commit at:"
    echo "https://github.com/SHAHCoinvip/shahcoin/commits/main"
else
    echo ""
    echo "❌ Push failed. Common issues:"
    echo "  - Wrong username/token"
    echo "  - Token doesn't have 'repo' permissions"
    echo "  - Network issue"
    echo ""
    echo "You can try pushing again:"
    echo "  git push origin main"
fi

