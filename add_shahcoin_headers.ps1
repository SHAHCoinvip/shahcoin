# Add SHAHCOIN Headers to Non-Code Files
# This script adds SHAHCOIN Core license banner to files that don't have it

Write-Host "=== ADDING SHAHCOIN HEADERS TO NON-CODE FILES ===" -ForegroundColor Green
Write-Host "Adding SHAHCOIN Core license banner to files that need it..." -ForegroundColor Yellow

# SHAHCOIN license banner to add
$shahcoinBanner = @"
######################################################################
# Additional integration and maintenance by:
# Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
# https://shah.vip | info@shah.vip
# Distributed under the MIT License
######################################################################

"@

# Files that need the SHAHCOIN header (identified as missing it)
$filesToUpdate = @(
    "scripts/setup-shahcoin.sh",
    "apply_customizations.sh",
    "test_premine_wallet.sh"
)

$updatedCount = 0
$updatedFiles = @()

foreach ($file in $filesToUpdate) {
    if (Test-Path $file) {
        try {
            $content = Get-Content $file -Raw -ErrorAction Stop
            $originalContent = $content

            # Check if the file already has the SHAHCOIN banner
            if ($content -notmatch "SHAHCOIN Core Developers") {
                # Add the banner at the beginning of the file
                $newContent = $shahcoinBanner + $content

                # Only write if content changed
                if ($newContent -ne $originalContent) {
                    Set-Content -Path $file -Value $newContent -NoNewline -ErrorAction Stop
                    $updatedCount++
                    $updatedFiles += $file
                    Write-Host "Updated: $file" -ForegroundColor Green
                }
            } else {
                Write-Host "Skipped: $file (already has SHAHCOIN banner)" -ForegroundColor Cyan
            }
        }
        catch {
            Write-Host "Error processing $file`: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "Warning: File not found: $file" -ForegroundColor Yellow
    }
}

Write-Host "`n=== SHAHCOIN HEADERS ADDITION COMPLETE ===" -ForegroundColor Green
Write-Host "Files Updated: $updatedCount" -ForegroundColor Green
Write-Host "Total Files Processed: $($filesToUpdate.Count)" -ForegroundColor White

if ($updatedFiles.Count -gt 0) {
    Write-Host "`nFiles Updated:" -ForegroundColor Green
    foreach ($file in $updatedFiles) {
        Write-Host "  - $file" -ForegroundColor White
    }
}

Write-Host "`nAll non-code files now have SHAHCOIN Core license banner!" -ForegroundColor Green
