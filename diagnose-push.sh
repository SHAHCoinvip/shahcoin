#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔍 Diagnosing GitHub Push Issue"
echo "================================"
echo ""

# Check remote URL
echo "1️⃣ Current remote URL:"
git remote -v
echo ""

# Check if there are any cached credentials
echo "2️⃣ Clearing any cached credentials..."
git config --global --unset credential.helper
git config --unset credential.helper
echo "✓ Cleared"
echo ""

# Check the repo owner (case sensitivity matters!)
echo "3️⃣ Checking repository info..."
echo "Remote repo should be: SHAHCoinvip/shahcoin"
echo "Username for token: SHAHCoinvip"
echo ""

# Try with lowercase username
echo "4️⃣ Trying push with lowercase username..."
git push "https://shahcoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git" main

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 SUCCESS! It was a username case issue!"
    echo "The token works with 'shahcoinvip' (lowercase)"
    exit 0
fi

echo ""
echo "That didn't work either. Let's try a few more things..."
echo ""

# Try setting the remote and pushing
echo "5️⃣ Resetting remote URL and trying again..."
git remote remove origin 2>/dev/null || true
git remote add origin "https://shahcoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git"
git push -u origin main

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 SUCCESS! Push completed!"
    exit 0
fi

echo ""
echo "❌ Still failing. Possible issues:"
echo ""
echo "1. Token might need SSO authorization (if repo is under an org)"
echo "   Check: https://github.com/settings/tokens"
echo "   Look for 'Configure SSO' next to your token"
echo ""
echo "2. Token might be for a different GitHub account"
echo "   Verify you're logged into the correct account"
echo ""
echo "3. Repository permissions might have changed"
echo "   Check: https://github.com/SHAHCoinvip/shahcoin/settings"
echo ""
echo "4. Try generating a completely NEW token:"
echo "   https://github.com/settings/tokens/new"
echo "   - Name: ShahCoin Push Access"
echo "   - Expiration: 90 days"
echo "   - Select: ✓ repo (all sub-items)"
echo "   - Generate and try again"
echo ""

