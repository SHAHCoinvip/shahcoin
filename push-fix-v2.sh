#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔧 Attempting GitHub Push - Method 2"
echo "====================================="
echo ""

# Try using git credential store with explicit token
echo "https://SHAHCoinvip:${GITHUB_TOKEN}@github.com" > ~/.git-credentials
git config --global credential.helper store

# Set remote URL with token embedded
git remote set-url origin "https://SHAHCoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git"

echo "Pushing commit d340bd4..."
git push origin main

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 SUCCESS! Push completed!"
    echo "View at: https://github.com/SHAHCoinvip/shahcoin/commits/main"
else
    echo ""
    echo "❌ Still failed. Let's check token permissions."
    echo ""
    echo "Please verify your token has:"
    echo "  ✓ 'repo' - Full control of private repositories"
    echo "  ✓ 'workflow' - Update GitHub Action workflows (optional)"
    echo ""
    echo "Check at: https://github.com/settings/tokens"
    echo ""
    echo "Or try manual push:"
    echo "  git push https://SHAHCoinvip:YOUR_TOKEN@github.com/SHAHCoinvip/shahcoin.git main"
fi

# Clean up
rm -f ~/.git-credentials
git config --global --unset credential.helper

