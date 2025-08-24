#!/usr/bin/env python3
"""
SHAHCOIN .github Folder Branding Check
Specifically checks all files in the .github folder for old branding patterns.
"""

import os
import re
from pathlib import Path

def check_github_files():
    """Check all files in .github folder for old branding"""
    print("🔍 SHAHCOIN .github Folder Branding Check")
    print("=" * 50)
    
    # Define old patterns to check for
    old_patterns = {
        "www.shah.vip": r"shahcoincore\.org",
        "www.shah.vip": r"shahcoin\.org", 
        "github.com/SHAHCoinvip/shahcoin": r"github\.com/shahcoin/shahcoin",
        "github.com/SHAHCoinvip": r"github\.com/shahcoin-core",
        "info@shah.vip": r"security@shahcoincore\.org",
    }
    
    # Define new patterns that should exist
    new_patterns = {
        "shah.vip": r"shah\.vip",
        "github.com/SHAHCoinvip/shahcoin": r"github\.com/SHAHCoinvip/shahcoin",
        "github.com/SHAHCoinvip": r"github\.com/SHAHCoinvip",
        "info@shah.vip": r"info@shah\.vip",
    }
    
    github_dir = Path('.github')
    if not github_dir.exists():
        print("❌ .github directory not found")
        return
    
    # Get all files in .github directory
    all_files = []
    for pattern in ['**/*.yml', '**/*.yaml', '**/*.md', '**/*.txt']:
        all_files.extend(github_dir.glob(pattern))
    
    print(f"📁 Found {len(all_files)} files in .github folder")
    print()
    
    old_patterns_found = 0
    new_patterns_found = 0
    
    for filepath in all_files:
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            print(f"🔍 Checking: {filepath}")
            
            # Check for old patterns
            old_found = []
            for pattern_name, pattern in old_patterns.items():
                if re.search(pattern, content, re.IGNORECASE):
                    old_found.append(pattern_name)
            
            if old_found:
                print(f"   ❌ Found old patterns: {', '.join(old_found)}")
                old_patterns_found += len(old_found)
            else:
                print(f"   ✅ No old patterns found")
            
            # Check for new patterns
            new_found = []
            for pattern_name, pattern in new_patterns.items():
                if re.search(pattern, content, re.IGNORECASE):
                    new_found.append(pattern_name)
            
            if new_found:
                print(f"   ✅ Found new patterns: {', '.join(new_found)}")
                new_patterns_found += len(new_found)
            else:
                print(f"   ⚠️  No new patterns found")
            
            print()
            
        except Exception as e:
            print(f"   ❌ Error reading {filepath}: {e}")
            print()
    
    # Summary
    print("📊 .github Folder Summary")
    print("=" * 50)
    print(f"✅ New branding patterns found: {new_patterns_found}")
    print(f"❌ Old branding patterns found: {old_patterns_found}")
    
    if old_patterns_found == 0:
        print("🎉 SUCCESS: All old branding patterns removed from .github folder!")
    else:
        print("⚠️  WARNING: Some old branding patterns still exist in .github folder")
    
    if new_patterns_found > 0:
        print("✅ SUCCESS: New branding patterns are in place in .github folder!")
    else:
        print("❌ ERROR: No new branding patterns found in .github folder")
    
    print()
    
    # Overall status
    if old_patterns_found == 0 and new_patterns_found > 0:
        print("🚀 .GITHUB FOLDER: COMPLETE AND VERIFIED")
    else:
        print("⚠️  .GITHUB FOLDER: REQUIRES ATTENTION")

if __name__ == "__main__":
    check_github_files()
