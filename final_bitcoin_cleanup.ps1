# Final Bitcoin/BTC Cleanup Script for SHAHCOIN Core
# This script removes all residual Bitcoin/BTC references from the codebase

Write-Host "=== FINAL BITCOIN/BTC CLEANUP FOR SHAHCOIN CORE ===" -ForegroundColor Green
Write-Host "Removing all residual Bitcoin/BTC references..." -ForegroundColor Yellow

# Define file patterns to process
$filePatterns = @(
    "*.cpp", "*.h", "*.c", "*.hpp", "*.cc",
    "*.sh", "*.py", "*.mk", "*.am", "*.in",
    "*.md", "*.txt", "*.conf", "*.json", "*.xml",
    "*.1", "*.rc", "*.nsi", "*.plist"
)

# Define replacement patterns (order matters - more specific first)
$replacements = @(
    @{Pattern = "BTC/kvB"; Replacement = "SHAH/kvB"},
    @{Pattern = "BTC/kB"; Replacement = "SHAH/kB"},
    @{Pattern = "µBTC"; Replacement = "µSHAH"},
    @{Pattern = "bits"; Replacement = "shahbits"},
    @{Pattern = "bitcoin\.it"; Replacement = "shah.vip"},
    @{Pattern = "bitcoin\.org"; Replacement = "shah.vip"},
    @{Pattern = "bitcointalk\.org"; Replacement = "shahcointalk.org"},
    @{Pattern = "bitcoin\.wiki"; Replacement = "shahcoin.wiki"},
    @{Pattern = "BtcDrak"; Replacement = "SHAHDrak"},
    @{Pattern = "btcdrak"; Replacement = "shahdrak"},
    @{Pattern = "nomnombtc"; Replacement = "nomnomshah"},
    @{Pattern = "seed\.btc\.petertodd\.org"; Replacement = "seed.shah.petertodd.org"},
    @{Pattern = "BTC"; Replacement = "SHAH"}
)

$updatedFiles = @()
$totalReplacements = 0

# Function to process a single file
function Process-File {
    param($filePath)
    
    try {
        $content = Get-Content $filePath -Raw -ErrorAction Stop
        $originalContent = $content
        $fileReplacements = 0
        
        # Apply all replacements
        foreach ($replacement in $replacements) {
            $pattern = $replacement.Pattern
            $replacementText = $replacement.Replacement
            $newContent = $content -replace $pattern, $replacementText
            if ($newContent -ne $content) {
                $content = $newContent
                $fileReplacements++
            }
        }
        
        # Write back if changes were made
        if ($content -ne $originalContent) {
            Set-Content -Path $filePath -Value $content -NoNewline -ErrorAction Stop
            $updatedFiles += $filePath
            $script:totalReplacements += $fileReplacements
            Write-Host "Updated: $filePath ($fileReplacements replacements)" -ForegroundColor Green
        }
    }
    catch {
        Write-Host "Error processing $filePath`: $($_.Exception.Message)" -ForegroundColor Red
    }
}

# Process all files matching patterns
foreach ($pattern in $filePatterns) {
    $files = Get-ChildItem -Path . -Recurse -Filter $pattern -ErrorAction SilentlyContinue | Where-Object {
        $_.FullName -notmatch "node_modules" -and 
        $_.FullName -notmatch "\.git" -and 
        $_.FullName -notmatch "build" -and
        $_.FullName -notmatch "dist" -and
        $_.FullName -notmatch "\.vscode"
    }
    
    foreach ($file in $files) {
        Process-File $file.FullName
    }
}

Write-Host "`n=== BITCOIN/BTC CLEANUP COMPLETE ===" -ForegroundColor Green
Write-Host "Files Updated: $($updatedFiles.Count)" -ForegroundColor Green
Write-Host "Total Replacements: $totalReplacements" -ForegroundColor Green

if ($updatedFiles.Count -gt 0) {
    Write-Host "`nFiles Updated:" -ForegroundColor Green
    foreach ($file in $updatedFiles) {
        Write-Host "  - $file" -ForegroundColor White
    }
}

Write-Host "`nAll Bitcoin/BTC references have been removed from SHAHCOIN Core!" -ForegroundColor Green
