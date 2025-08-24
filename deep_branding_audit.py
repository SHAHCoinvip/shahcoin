#!/usr/bin/env python3
"""
SHAHCOIN Deep Branding Audit & Correction Script
Performs a comprehensive file-by-file audit to ensure all branding is updated to 2025 SHAHCOIN standards.
"""

import os
import re
import glob
from pathlib import Path
from datetime import datetime

class BrandingAuditor:
    def __init__(self):
        # Define old patterns that should be replaced
        self.old_patterns = {
            # Email addresses
            "info@shah.vip": r"legal@shah\.vip",
            "info@shah.vip": r"security@shahcoincore\.org",
            "secp256k1-info@shah.vip": r"secp256k1-security@shahcoincore\.org",
            "any_old_email": r"[a-zA-Z0-9._%+-]+@(?!shah\.vip)[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}",
            
            # Domain references
            "www.shah.vip": r"shahcoincore\.org",
            "www.shah.vip": r"shahcoin\.org",
            "https://www.shah.vip": r"http://shahcoincore\.org",
            "https://www.shah.vip": r"https://shahcoincore\.org",
            
            # GitHub URLs
            "github.com/SHAHCoinvip/shahcoin": r"github\.com/shahcoin/shahcoin",
            "github.com/SHAHCoinvip": r"github\.com/shahcoin-core",
            
            # Copyright headers (old years)
            "copyright_2009_2023": r"Copyright \([cC]\) 2009-2023",
            "copyright_2009_2022": r"Copyright \([cC]\) 2009-2022",
            "copyright_2009_2021": r"Copyright \([cC]\) 2009-2021",
            "copyright_2009_2020": r"Copyright \([cC]\) 2009-2020",
            "copyright_2009_2019": r"Copyright \([cC]\) 2009-2019",
            "copyright_2009_2018": r"Copyright \([cC]\) 2009-2018",
            "copyright_2009_2017": r"Copyright \([cC]\) 2009-2017",
            "copyright_2009_2016": r"Copyright \([cC]\) 2009-2016",
            "copyright_2009_2015": r"Copyright \([cC]\) 2009-2015",
            "copyright_2009_2014": r"Copyright \([cC]\) 2009-2014",
            "copyright_2009_2013": r"Copyright \([cC]\) 2009-2013",
            "copyright_2009_2012": r"Copyright \([cC]\) 2009-2012",
            "copyright_2009_2011": r"Copyright \([cC]\) 2009-2011",
            "copyright_2009_2010": r"Copyright \([cC]\) 2009-2010",
            "copyright_2009_2009": r"Copyright \([cC]\) 2009-2009",
            
            # Old project names
            "SHAHCOIN Core": r"SHAHCOIN Core",
            "shahcoin-core": r"shahcoin-core",
            "SHAHCOIN Core": r"SHAHCOIN Core",
        }
        
        # Define new patterns that should exist
        self.new_patterns = {
            "info@shah.vip": r"info@shah\.vip",
            "https://www.shah.vip": r"https://www\.shah\.vip",
            "https://shah.vip": r"https://shah\.vip",
            "github.com/SHAHCoinvip/shahcoin": r"github\.com/SHAHCoinvip/shahcoin",
            "github.com/SHAHCoinvip": r"github\.com/SHAHCoinvip",
            "Copyright (C) 2025": r"Copyright \([cC]\) 2025",
            "SHAHCOIN Core": r"SHAHCOIN Core",
            "SHAHCOIN CORE": r"SHAHCOIN CORE",
        }
        
        # Standard license footer
        self.standard_license_footer = """Please contribute if you find SHAHCOIN Core useful. Visit
<https://www.shah.vip> for further information about the software.
The source code is available from <https://github.com/SHAHCoinvip/shahcoin>.

This is experimental software.
Distributed under the MIT software license, see the accompanying file COPYING
or <https://opensource.org/licenses/MIT>.

Primary Developer: Hamid Rad
Contact: info@shah.vip

Primary Developer: Hamid Rad
Contact: info@shah.vip"""
        
        # Directories to skip
        self.skip_dirs = {
            '.git', 'build', 'dist', 'node_modules', '__pycache__', 
            'test/tmp', '.vscode', '.idea', 'autom4te.cache'
        }
        
        # File extensions to process
        self.file_extensions = {
            '.cpp', '.h', '.md', '.sh', '.bat', '.conf', '.desktop', 
            '.rc', '.json', '.txt', '.xpm', '.yml', '.yaml', '.py',
            '.in', '.1', '.am', '.ac', '.pc', '.cmake', '.mk'
        }
        
        # Binary extensions to skip
        self.binary_extensions = {
            '.o', '.a', '.so', '.dll', '.exe', '.pyc', '.class',
            '.obj', '.lib', '.dylib', '.bin'
        }

    def should_skip_file(self, filepath):
        """Check if file should be skipped"""
        path_str = str(filepath)
        
        # Skip binary files
        if any(path_str.endswith(ext) for ext in self.binary_extensions):
            return True
            
        # Skip directories
        if any(skip_dir in path_str for skip_dir in self.skip_dirs):
            return True
            
        # Skip if no valid extension
        if not any(path_str.endswith(ext) for ext in self.file_extensions):
            return True
            
        return False

    def update_file_content(self, content, filename):
        """Update content with new SHAHCOIN branding"""
        original_content = content
        changes_made = []
        
        # 1. Update email addresses
        if re.search(r"legal@shah\.vip", content, re.IGNORECASE):
            content = re.sub(r"legal@shah\.vip", "info@shah.vip", content, flags=re.IGNORECASE)
            changes_made.append("email: info@shah.vip → info@shah.vip")
            
        if re.search(r"security@shahcoincore\.org", content, re.IGNORECASE):
            content = re.sub(r"security@shahcoincore\.org", "info@shah.vip", content, flags=re.IGNORECASE)
            changes_made.append("email: info@shah.vip → info@shah.vip")
            
        if re.search(r"secp256k1-security@shahcoincore\.org", content, re.IGNORECASE):
            content = re.sub(r"secp256k1-security@shahcoincore\.org", "info@shah.vip", content, flags=re.IGNORECASE)
            changes_made.append("email: secp256k1-info@shah.vip → info@shah.vip")
        
        # 2. Update domain references
        if re.search(r"shahcoincore\.org", content, re.IGNORECASE):
            content = re.sub(r"https?://shahcoincore\.org", "https://www.shah.vip", content, flags=re.IGNORECASE)
            content = re.sub(r"shahcoincore\.org", "www.shah.vip", content, flags=re.IGNORECASE)
            changes_made.append("domain: www.shah.vip → www.shah.vip")
            
        if re.search(r"shahcoin\.org", content, re.IGNORECASE):
            content = re.sub(r"https?://shahcoin\.org", "https://www.shah.vip", content, flags=re.IGNORECASE)
            content = re.sub(r"shahcoin\.org", "www.shah.vip", content, flags=re.IGNORECASE)
            changes_made.append("domain: www.shah.vip → www.shah.vip")
        
        # 3. Update GitHub URLs
        if re.search(r"github\.com/shahcoin/shahcoin", content, re.IGNORECASE):
            content = re.sub(r"github\.com/shahcoin/shahcoin", "github.com/SHAHCoinvip/shahcoin", content, flags=re.IGNORECASE)
            changes_made.append("github: github.com/SHAHCoinvip/shahcoin → github.com/SHAHCoinvip/shahcoin")
            
        if re.search(r"github\.com/shahcoin-core", content, re.IGNORECASE):
            content = re.sub(r"github\.com/shahcoin-core", "github.com/SHAHCoinvip", content, flags=re.IGNORECASE)
            changes_made.append("github: github.com/SHAHCoinvip → github.com/SHAHCoinvip")
        
        # 4. Update copyright headers
        copyright_patterns = [
            (r"Copyright \([cC]\) 2009-\d{4} The SHAHCOIN Core developers", "Copyright (C) 2025 The SHAHCOIN Core Developers"),
            (r"Copyright \([cC]\) 2009-\d{4} Shahcoin Developers", "Copyright (C) 2025 SHAHCOIN Developers"),
            (r"Copyright \([cC]\) 2009-\d{4} The SHAHCOIN Core Developers", "Copyright (C) 2025 The SHAHCOIN Core Developers"),
        ]
        
        for pattern, replacement in copyright_patterns:
            if re.search(pattern, content, re.IGNORECASE):
                content = re.sub(pattern, replacement, content, flags=re.IGNORECASE)
                changes_made.append(f"copyright: {pattern} → {replacement}")
        
        # 5. Update project names
        if re.search(r"SHAHCOIN Core", content):
            content = re.sub(r"SHAHCOIN Core", "SHAHCOIN Core", content)
            changes_made.append("project: SHAHCOIN Core → SHAHCOIN Core")
            
        if re.search(r"shahcoin-core", content, re.IGNORECASE):
            content = re.sub(r"shahcoin-core", "shahcoin-core", content, flags=re.IGNORECASE)
            changes_made.append("project: shahcoin-core → shahcoin-core")
        
        # 6. Update license footer blocks
        old_license_pattern = r'Please contribute if you find .*? useful\. Visit\s*<https://.*?> for further information about the software\.\s*The source code is available from <https://github\.com/.*?>\.\s*This is experimental software\.\s*Distributed under the MIT software license, see the accompanying file COPYING\s*or <https://opensource\.org/licenses/MIT>\.'
        
        if re.search(old_license_pattern, content, re.DOTALL):
            content = re.sub(old_license_pattern, self.standard_license_footer, content, flags=re.DOTALL)
            changes_made.append("license_footer: Updated to standard SHAHCOIN footer")
        
        return content, changes_made, content != original_content

    def audit_file(self, filepath):
        """Audit a single file for branding issues"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            # Check for old patterns
            old_patterns_found = []
            for pattern_name, pattern in self.old_patterns.items():
                if re.search(pattern, content, re.IGNORECASE):
                    old_patterns_found.append(pattern_name)
            
            # Check for new patterns
            new_patterns_found = []
            for pattern_name, pattern in self.new_patterns.items():
                if re.search(pattern, content, re.IGNORECASE):
                    new_patterns_found.append(pattern_name)
            
            # Update content if needed
            updated_content, changes_made, was_updated = self.update_file_content(content, filepath)
            
            if was_updated:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(updated_content)
            
            return {
                'filepath': str(filepath),
                'old_patterns_found': old_patterns_found,
                'new_patterns_found': new_patterns_found,
                'changes_made': changes_made,
                'was_updated': was_updated
            }
            
        except Exception as e:
            return {
                'filepath': str(filepath),
                'error': str(e),
                'old_patterns_found': [],
                'new_patterns_found': [],
                'changes_made': [],
                'was_updated': False
            }

    def run_audit(self):
        """Run the complete branding audit"""
        print("🔍 SHAHCOIN Deep Branding Audit & Correction")
        print("=" * 60)
        print(f"📅 Audit Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print()
        
        # Get all files in the project
        project_root = Path('.')
        all_files = []
        
        for pattern in ['**/*']:
            all_files.extend(project_root.glob(pattern))
        
        # Filter files
        files_to_audit = []
        for f in all_files:
            try:
                if f.is_file() and not self.should_skip_file(f):
                    files_to_audit.append(f)
            except (OSError, PermissionError):
                # Skip files that can't be accessed
                continue
        
        print(f"📁 Found {len(files_to_audit)} files to audit")
        print()
        
        # Audit results
        total_files = len(files_to_audit)
        updated_files = 0
        files_with_old_patterns = 0
        files_with_new_patterns = 0
        errors = 0
        
        print("🔍 Starting file-by-file audit...")
        print()
        
        for filepath in files_to_audit:
            result = self.audit_file(filepath)
            
            # Print results
            print(f"📄 {result['filepath']}")
            
            if 'error' in result:
                print(f"   ❌ Error: {result['error']}")
                errors += 1
            else:
                if result['was_updated']:
                    print(f"   ✅ Updated")
                    updated_files += 1
                    for change in result['changes_made']:
                        print(f"      🔁 {change}")
                else:
                    print(f"   ⏭️  No changes needed")
                
                if result['old_patterns_found']:
                    print(f"   ⚠️  Legacy patterns found: {', '.join(result['old_patterns_found'])}")
                    files_with_old_patterns += 1
                
                if result['new_patterns_found']:
                    print(f"   ✅ New patterns found: {', '.join(result['new_patterns_found'])}")
                    files_with_new_patterns += 1
            
            print()
        
        # Summary
        print("📊 Deep Branding Audit Summary")
        print("=" * 60)
        print(f"📁 Total files audited: {total_files}")
        print(f"✅ Files updated: {updated_files}")
        print(f"⚠️  Files with legacy patterns: {files_with_old_patterns}")
        print(f"✅ Files with new patterns: {files_with_new_patterns}")
        print(f"❌ Errors: {errors}")
        print()
        
        if files_with_old_patterns == 0:
            print("🎉 SUCCESS: All legacy branding patterns have been removed!")
        else:
            print("⚠️  WARNING: Some legacy branding patterns still exist")
        
        if files_with_new_patterns > 0:
            print("✅ SUCCESS: New branding patterns are in place!")
        else:
            print("❌ ERROR: No new branding patterns found")
        
        print()
        
        # Overall status
        if files_with_old_patterns == 0 and files_with_new_patterns > 0:
            print("🚀 DEEP BRANDING AUDIT: COMPLETE AND VERIFIED")
            print("✅ Ready for mainnet launch!")
        else:
            print("⚠️  DEEP BRANDING AUDIT: REQUIRES ATTENTION")
            print("Please review and fix any remaining issues.")
        
        print()
        print("🔗 New branding references:")
        print("• Domain: https://www.shah.vip")
        print("• GitHub: https://github.com/SHAHCoinvip/shahcoin")
        print("• Email: info@shah.vip")
        print("• Copyright: Copyright (C) 2025 The SHAHCOIN Core Developers")
        print("• Primary Developer: Hamid Rad")

def main():
    auditor = BrandingAuditor()
    auditor.run_audit()

if __name__ == "__main__":
    main()
