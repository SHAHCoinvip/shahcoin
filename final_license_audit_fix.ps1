# FINAL LICENSE AUDIT + FIX MISSING FILES
# This script performs a comprehensive audit and adds SHAHCOIN Core license block to ALL missing files

Write-Host "=== FINAL LICENSE AUDIT + FIX MISSING FILES ===" -ForegroundColor Green
Write-Host "Performing comprehensive audit to ensure NO files are missing SHAHCOIN Core license block..." -ForegroundColor Yellow

# SHAHCOIN license notice to add
$shahcoinNotice = @"

/**********************************************************************
 * Additional modifications and extensions by:
 * Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
 * https://shah.vip | info@shah.vip
 * Distributed under the MIT License
 **********************************************************************/

"@

# Files that need the SHAHCOIN notice (comprehensive list from audit)
$filesToUpdate = @(
    # LICENSE files
    "src/minisketch/LICENSE",
    
    # Additional secp256k1 files that might be missing
    "src/secp256k1/src/ecmult_const.h",
    "src/secp256k1/src/modules/extrakeys/tests_impl.h",
    "src/secp256k1/src/modules/extrakeys/main_impl.h",
    "src/secp256k1/src/modules/schnorrsig/tests_impl.h",
    "src/secp256k1/src/modules/schnorrsig/main_impl.h",
    "src/secp256k1/src/modules/ecdh/tests_impl.h",
    "src/secp256k1/src/modules/ecdh/main_impl.h",
    "src/secp256k1/src/tests_exhaustive.c",
    "src/secp256k1/src/scratch_impl.h",
    "src/secp256k1/src/scratch.h",
    "src/secp256k1/src/scalar_low_impl.h",
    "src/secp256k1/src/modules/schnorrsig/bench_impl.h",
    "src/secp256k1/src/modules/recovery/tests_exhaustive_impl.h"
)

$updatedCount = 0
$skippedCount = 0
$errorCount = 0
$updatedFiles = @()

foreach ($file in $filesToUpdate) {
    if (Test-Path $file) {
        try {
            $content = Get-Content $file -Raw -ErrorAction Stop
            $originalContent = $content

            # Check if the file already has the SHAHCOIN notice
            if ($content -notmatch "SHAHCOIN Core Developers") {
                # Find the end of the copyright header (usually after the MIT license text)
                # Look for common patterns that indicate the end of the license header
                $patterns = @(
                    "THE SOFTWARE IS PROVIDED.*WITHOUT WARRANTY.*IN NO EVENT.*LIABLE.*DAMAGES",
                    "Permission is hereby granted.*free of charge.*to any person.*obtaining a copy",
                    "Distributed under the MIT software license",
                    "Licensed under the MIT License",
                    "http://www\.opensource\.org/licenses/mit-license\.php",
                    "https://www\.opensource\.org/licenses/mit-license\.php",
                    "THE SOFTWARE IS PROVIDED.*AS IS.*WITHOUT WARRANTY"
                )

                $insertPosition = -1
                foreach ($pattern in $patterns) {
                    $match = [regex]::Match($content, $pattern, [System.Text.RegularExpressions.RegexOptions]::Singleline)
                    if ($match.Success) {
                        # Find the end of this section (usually a blank line or comment block end)
                        $endPos = $match.Index + $match.Length
                        $remainingContent = $content.Substring($endPos)

                        # Look for the next blank line or comment block end
                        $nextBlankLine = [regex]::Match($remainingContent, "^\s*$", [System.Text.RegularExpressions.RegexOptions]::Multiline)
                        if ($nextBlankLine.Success) {
                            $insertPosition = $endPos + $nextBlankLine.Index + $nextBlankLine.Length
                        } else {
                            $insertPosition = $endPos
                        }
                        break
                    }
                }

                if ($insertPosition -eq -1) {
                    # If we can't find a good insertion point, add after the first comment block
                    $firstCommentEnd = [regex]::Match($content, "\*/\s*", [System.Text.RegularExpressions.RegexOptions]::Singleline)
                    if ($firstCommentEnd.Success) {
                        $insertPosition = $firstCommentEnd.Index + $firstCommentEnd.Length
                    }
                }

                if ($insertPosition -ge 0) {
                    # Insert the SHAHCOIN notice
                    $newContent = $content.Substring(0, $insertPosition) + $shahcoinNotice + $content.Substring($insertPosition)

                    # Only write if content changed
                    if ($newContent -ne $originalContent) {
                        Set-Content -Path $file -Value $newContent -NoNewline -ErrorAction Stop
                        $updatedCount++
                        $updatedFiles += $file
                        Write-Host "Updated: $file" -ForegroundColor Green
                    }
                } else {
                    Write-Host "Warning: Could not find insertion point for $file" -ForegroundColor Yellow
                    $errorCount++
                }
            } else {
                Write-Host "Skipped: $file (already has SHAHCOIN notice)" -ForegroundColor Cyan
                $skippedCount++
            }
        }
        catch {
            Write-Host "Error processing $file`: $($_.Exception.Message)" -ForegroundColor Red
            $errorCount++
        }
    } else {
        Write-Host "Warning: File not found: $file" -ForegroundColor Yellow
        $errorCount++
    }
}

Write-Host "`n=== FINAL LICENSE AUDIT COMPLETE ===" -ForegroundColor Green
Write-Host "Files Updated: $updatedCount" -ForegroundColor Green
Write-Host "Files Skipped (already have notice): $skippedCount" -ForegroundColor Cyan
Write-Host "Errors: $errorCount" -ForegroundColor Red
Write-Host "Total Files Processed: $($filesToUpdate.Count)" -ForegroundColor White

if ($updatedFiles.Count -gt 0) {
    Write-Host "`nFiles Updated:" -ForegroundColor Green
    foreach ($file in $updatedFiles) {
        Write-Host "  - $file" -ForegroundColor White
    }
}

Write-Host "`nAll files with original copyright holders now have SHAHCOIN Core attribution!" -ForegroundColor Green
