# Fix Makefile Copyright Headers
# This script fixes all old "Shahcoin Core developers" references to "SHAHCOIN Core Developers"

Write-Host "=== FIXING MAKEFILE COPYRIGHT HEADERS ===" -ForegroundColor Green
Write-Host "Updating all old Shahcoin Core developers references..." -ForegroundColor Yellow

# Files that need fixing
$filesToUpdate = @(
    "Makefile",
    "src/Makefile",
    "doc/man/Makefile"
)

$updatedCount = 0
$updatedFiles = @()

foreach ($file in $filesToUpdate) {
    if (Test-Path $file) {
        try {
            $content = Get-Content $file -Raw -ErrorAction Stop
            $originalContent = $content

            # Fix old copyright headers
            $content = $content -replace "Copyright \(c\) [0-9]{4}-[0-9]{4} The Shahcoin Core developers", "Copyright (C) 2025 The SHAHCOIN Core Developers"
            $content = $content -replace "Copyright \(c\) [0-9]{4} The Shahcoin Core developers", "Copyright (C) 2025 The SHAHCOIN Core Developers"
            $content = $content -replace "COPYRIGHT_HOLDERS_FINAL = The Shahcoin Core developers", "COPYRIGHT_HOLDERS_FINAL = The SHAHCOIN Core Developers"

            # Only write if content changed
            if ($content -ne $originalContent) {
                Set-Content -Path $file -Value $content -NoNewline -ErrorAction Stop
                $updatedCount++
                $updatedFiles += $file
                Write-Host "Updated: $file" -ForegroundColor Green
            } else {
                Write-Host "No changes needed: $file" -ForegroundColor Cyan
            }
        }
        catch {
            Write-Host "Error processing $file`: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "Warning: File not found: $file" -ForegroundColor Yellow
    }
}

Write-Host "`n=== MAKEFILE COPYRIGHT FIX COMPLETE ===" -ForegroundColor Green
Write-Host "Files Updated: $updatedCount" -ForegroundColor Green
Write-Host "Total Files Processed: $($filesToUpdate.Count)" -ForegroundColor White

if ($updatedFiles.Count -gt 0) {
    Write-Host "`nFiles Updated:" -ForegroundColor Green
    foreach ($file in $updatedFiles) {
        Write-Host "  - $file" -ForegroundColor White
    }
}

Write-Host "`nAll Makefile copyright headers now use SHAHCOIN Core Developers!" -ForegroundColor Green
