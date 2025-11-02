# 🔑 GitHub Token Permissions Guide

## Problem
Your token has "public_repo" access but needs "repo" (full) access to push.

## Solution: Update Token Permissions

### Option 1: Update Existing Token ⭐ (Recommended)

1. **Go to**: https://github.com/settings/tokens
2. **Find your token** in the list
3. **Click**: "Edit" or the token name
4. **Check these boxes**:
   - ✅ **repo** (Full control of private repositories)
     - This includes: repo:status, repo_deployment, public_repo, repo:invite
5. **Scroll down** and click **"Update token"**
6. **Copy the token** (it won't change, but confirm you have it)

### Option 2: Create New Token

If you can't edit the existing token:

1. **Go to**: https://github.com/settings/tokens
2. **Click**: "Generate new token" → "Generate new token (classic)"
3. **Name**: "ShahCoin Development - Full Access"
4. **Select scopes**:
   - ✅ **repo** (check the main "repo" box - this enables all sub-options)
5. **Generate token**
6. **COPY THE TOKEN IMMEDIATELY** (you won't see it again!)

### Then Push Again

```bash
# Use the updated/new token
export GITHUB_TOKEN='YOUR_NEW_TOKEN_HERE'

# Try pushing
git push "https://SHAHCoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git" main
```

## Why This is Needed

- ❌ **"public_repo"** = Read public repos + limited write
- ✅ **"repo"** = Full read/write access to all repos (public & private)

Even for public repositories, GitHub requires the **full "repo" scope** to push commits.

---

## Quick Test After Updating

```bash
cd /mnt/c/Users/hamid/shahcoin-core/shahcoin

# Set your updated token
export GITHUB_TOKEN='YOUR_UPDATED_TOKEN'

# Try the push
git push "https://SHAHCoinvip:${GITHUB_TOKEN}@github.com/SHAHCoinvip/shahcoin.git" main
```

If successful, you'll see:
```
Enumerating objects: 7, done.
Counting objects: 100% (7/7), done.
...
To https://github.com/SHAHCoinvip/shahcoin.git
   xxxxxx..d340bd4  main -> main
```

## Your Commit is Ready!

✅ Commit d340bd4 is already created locally
✅ Just needs the token permission to push
✅ All your changes are saved

Once you update the token, the push will work! 🚀

