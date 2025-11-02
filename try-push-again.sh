#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

echo "🔑 GitHub Push - Final Attempt"
echo "==============================="
echo ""

echo "Testing token authentication..."
echo ""

# Test 1: Try with current token and lowercase username
echo "Method 1: Lowercase username..."
git push "https://shahcoinvip:${GITHUB_TOKEN}@github.com/shahcoinvip/shahcoin.git" main 2>&1

if [ $? -eq 0 ]; then
    echo "✅ SUCCESS with Method 1!"
    exit 0
fi

# Test 2: Try uppercase repo name
echo ""
echo "Method 2: Different URL format..."
git push "https://${GITHUB_TOKEN}@github.com/shahcoinvip/shahcoin.git" main 2>&1

if [ $? -eq 0 ]; then
    echo "✅ SUCCESS with Method 2!"
    exit 0
fi

echo ""
echo "❌ Both methods failed."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📝 You need to create a NEW GitHub token:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "1. Go to: https://github.com/settings/tokens/new"
echo ""
echo "2. Fill in:"
echo "   Name: ShahCoin Push Access"
echo "   Expiration: 90 days (or longer)"
echo ""
echo "3. Select permissions:"
echo "   ✅ repo (check the MAIN box - this auto-checks all sub-items)"
echo "      ✅ repo:status"
echo "      ✅ repo_deployment"
echo "      ✅ public_repo"
echo "      ✅ repo:invite"
echo "      ✅ security_events"
echo ""
echo "   DO NOT use:"
echo "   ❌ public_repo only"
echo ""
echo "4. Click 'Generate token'"
echo ""
echo "5. COPY the token immediately"
echo ""
echo "6. Run:"
echo "   export GITHUB_TOKEN='your_new_token_here'"
echo "   git push \"https://shahcoinvip:\${GITHUB_TOKEN}@github.com/shahcoinvip/shahcoin.git\" main"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Your commit d340bd4 is ready and waiting to be pushed!"
echo ""

