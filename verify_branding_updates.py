#!/usr/bin/env python3
"""
SHAHCOIN Branding Update Verification Script
Verifies that all key branding updates were applied correctly.
"""

import os
import re
from pathlib import Path

def check_file_for_patterns(filepath, patterns):
    """Check if file contains any of the old patterns"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        found_patterns = []
        for pattern_name, pattern in patterns.items():
            if re.search(pattern, content, re.IGNORECASE):
                found_patterns.append(pattern_name)
        
        return found_patterns
    except Exception as e:
        return [f"ERROR: {e}"]

def main():
    """Main verification function"""
    print("🔍 SHAHCOIN Branding Update Verification")
    print("=" * 50)
    
    # Define patterns to check for (old branding that should NOT exist)
    old_patterns = {
        "Old Copyright 2023": r"Copyright \(c\) 2009-2023 The SHAHCOIN Core developers",
        "Old Copyright 2022": r"Copyright \(c\) 2009-2022 The SHAHCOIN Core developers",
        "Old Copyright 2021": r"Copyright \(c\) 2009-2021 The SHAHCOIN Core developers",
        "Old Domain www.shah.vip": r"shahcoincore\.org",
        "Old Domain www.shah.vip": r"shahcoin\.org",
        "Old GitHub shahcoin/shahcoin": r"github\.com/shahcoin/shahcoin",
        "Old GitHub shahcoin-core": r"github\.com/shahcoin-core",
        "Old Email info@shah.vip": r"security@shahcoincore\.org",
        "Old Email secp256k1-info@shah.vip": r"secp256k1-security@shahcoincore\.org",
    }
    
    # Define patterns to check for (new branding that SHOULD exist)
    new_patterns = {
        "New Copyright 2025": r"Copyright \(c\) 2009-2025 The SHAHCOIN CORE developers",
        "New Domain shah.vip": r"shah\.vip",
        "New GitHub SHAHCoinvip/shahcoin": r"github\.com/SHAHCoinvip/shahcoin",
        "New Email info@shah.vip": r"info@shah\.vip",
    }
    
    # Key files to check
    key_files = [
        "COPYING",
        "SECURITY.md",
        "README.md",
        "src/chainparams.cpp",
        "src/consensus/hybrid.cpp",
        "configure.ac",
        "build_msvc/shahcoin_config.h.in",
        "doc/man/shahcoind.1",
        "doc/release-notes.md",
    ]
    
    print("📋 Checking key files for old branding patterns...")
    print()
    
    old_patterns_found = 0
    new_patterns_found = 0
    
    for filepath in key_files:
        if not os.path.exists(filepath):
            print(f"⚠️  File not found: {filepath}")
            continue
        
        print(f"🔍 Checking: {filepath}")
        
        # Check for old patterns (should NOT exist)
        old_found = check_file_for_patterns(filepath, old_patterns)
        if old_found:
            print(f"   ❌ Found old patterns: {', '.join(old_found)}")
            old_patterns_found += len(old_found)
        else:
            print(f"   ✅ No old patterns found")
        
        # Check for new patterns (should exist)
        new_found = check_file_for_patterns(filepath, new_patterns)
        if new_found:
            print(f"   ✅ Found new patterns: {', '.join(new_found)}")
            new_patterns_found += len(new_found)
        else:
            print(f"   ⚠️  No new patterns found")
        
        print()
    
    # Summary
    print("📊 Verification Summary")
    print("=" * 50)
    print(f"✅ New branding patterns found: {new_patterns_found}")
    print(f"❌ Old branding patterns found: {old_patterns_found}")
    
    if old_patterns_found == 0:
        print("🎉 SUCCESS: All old branding patterns have been removed!")
    else:
        print("⚠️  WARNING: Some old branding patterns still exist")
    
    if new_patterns_found > 0:
        print("✅ SUCCESS: New branding patterns are in place!")
    else:
        print("❌ ERROR: No new branding patterns found")
    
    print()
    
    # Overall status
    if old_patterns_found == 0 and new_patterns_found > 0:
        print("🚀 BRANDING UPDATE: COMPLETE AND VERIFIED")
        print("✅ Ready for mainnet launch!")
    else:
        print("⚠️  BRANDING UPDATE: REQUIRES ATTENTION")
        print("Please review and fix any remaining issues.")
    
    print()
    print("🔗 New branding references:")
    print("• Domain: https://shah.vip")
    print("• GitHub: https://github.com/SHAHCoinvip/shahcoin")
    print("• Email: info@shah.vip")
    print("• Copyright: Copyright (C) 2025 The SHAHCOIN Core Developers")

if __name__ == "__main__":
    main()
