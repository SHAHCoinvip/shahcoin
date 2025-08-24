#!/usr/bin/env python3
"""
SHAHCOIN Global License Header & Branding Update Script
Updates all outdated references across the entire codebase.
"""

import os
import re
import glob
from pathlib import Path

def update_file_content(content, filename):
    """Update content with new SHAHCOIN branding"""
    original_content = content
    
    # 1. Update copyright headers
    content = re.sub(
        r'Copyright \(c\) 2009-\d{4} The SHAHCOIN Core developers',
        'Copyright (C) 2025 The SHAHCOIN Core Developers',
        content,
        flags=re.IGNORECASE
    )
    
    content = re.sub(
        r'Copyright \(c\) 2009-\d{4} Shahcoin Developers',
        'Copyright (C) 2025 SHAHCOIN Developers',
        content,
        flags=re.IGNORECASE
    )
    
    content = re.sub(
        r'Copyright \(c\) 2009-\d{4} The SHAHCOIN Core Developers',
        'Copyright (C) 2025 The SHAHCOIN Core Developers',
        content,
        flags=re.IGNORECASE
    )
    
    # 2. Update domain references
    content = content.replace('shah.vip', 'shah.vip')
    content = content.replace('shah.vip', 'shah.vip')
    
    # 3. Update GitHub repository links
    content = content.replace('github.com/SHAHCoinvip/shahcoin', 'github.com/SHAHCoinvip/shahcoin')
    content = content.replace('github.com/SHAHCoinvip', 'github.com/SHAHCoinvip')
    
    # 4. Update email addresses
    content = content.replace('security@shah.vip', 'info@shah.vip')
    content = content.replace('secp256k1-security@shah.vip', 'info@shah.vip')
    
    # 5. Update package URLs and bug reports
    content = content.replace('https://shah.vip/', 'https://shah.vip/')
    content = content.replace('https://github.com/SHAHCoinvip/shahcoin/issues', 'https://github.com/SHAHCoinvip/shahcoin/issues')
    
    # 6. Update specific documentation links
    content = content.replace('https://doxygen.shah.vip', 'https://shah.vip/docs')
    content = content.replace('https://shah.vip/en/', 'https://shah.vip/')
    
    # 7. Update license footer blocks
    license_footer_pattern = r'Please contribute if you find SHAHCOIN Core useful. Visit
<https://www.shah.vip> for further information about the software.
The source code is available from <https://github.com/SHAHCoinvip/shahcoin>.

This is experimental software.
Distributed under the MIT software license, see the accompanying file COPYING
or <https://opensource.org/licenses/MIT>.

Primary Developer: Hamid Rad
Contact: info@shah.vip'''
    
    content = re.sub(license_footer_pattern, new_license_footer, content, flags=re.DOTALL)
    
    return content, content != original_content

def should_skip_file(filename):
    """Check if file should be skipped"""
    skip_extensions = {'.git', '.o', '.a', '.so', '.dll', '.exe', '.pyc', '.class'}
    skip_dirs = {'.git', 'build', 'dist', 'node_modules', '__pycache__'}
    
    # Skip binary files and build artifacts
    if any(filename.endswith(ext) for ext in skip_extensions):
        return True
    
    # Skip directories
    if any(skip_dir in filename for skip_dir in skip_dirs):
        return True
    
    return False

def process_file(filepath):
    """Process a single file"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        updated_content, changed = update_file_content(content, filepath)
        
        if changed:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(updated_content)
            return True
        return False
        
    except Exception as e:
        print(f"❌ Error processing {filepath}: {e}")
        return False

def main():
    """Main function to update all files"""
    print("🔧 SHAHCOIN Global License Header & Branding Update")
    print("=" * 60)
    
    # Get all files in the project
    project_root = Path('.')
    all_files = []
    
    for pattern in ['**/*.cpp', '**/*.h', '**/*.md', '**/*.txt', '**/*.yml', '**/*.yaml', 
                   '**/*.py', '**/*.sh', '**/*.bat', '**/*.conf', '**/*.in', '**/*.1']:
        all_files.extend(project_root.glob(pattern))
    
    # Filter out files to skip
    files_to_process = [f for f in all_files if not should_skip_file(str(f))]
    
    print(f"📁 Found {len(files_to_process)} files to process")
    print()
    
    updated_count = 0
    error_count = 0
    
    for filepath in files_to_process:
        print(f"🔄 Processing: {filepath}")
        
        if process_file(filepath):
            print(f"✅ Updated: {filepath}")
            updated_count += 1
        else:
            print(f"⏭️  No changes: {filepath}")
    
    print()
    print("🎉 Update Complete!")
    print("=" * 60)
    print(f"✅ Files updated: {updated_count}")
    print(f"❌ Errors: {error_count}")
    print(f"📊 Total processed: {len(files_to_process)}")
    print()
    print("📋 Summary of changes:")
    print("• Updated copyright headers to 'Copyright (C) 2025 The SHAHCOIN Core Developers'")
    print("• Replaced shah.vip → shah.vip")
    print("• Replaced shah.vip → shah.vip")
    print("• Updated GitHub links to github.com/SHAHCoinvip/shahcoin")
    print("• Updated email addresses to info@shah.vip")
    print("• Updated license footer blocks")
    print()
    print("🔍 Please review the changes and test the build!")

if __name__ == "__main__":
    main()
