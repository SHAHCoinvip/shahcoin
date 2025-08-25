# Bitcoin/BTC Cleanup Verification Script
# This script verifies that all Bitcoin/BTC references have been removed

Write-Host "=== BITCOIN/BTC CLEANUP VERIFICATION ===" -ForegroundColor Green
Write-Host "Verifying that all Bitcoin/BTC references have been removed..." -ForegroundColor Yellow

# Define patterns to search for
$searchPatterns = @(
    "Bitcoin",
    "BTC",
    "bitcoin\.it",
    "bitcoin\.org",
    "bitcointalk\.org",
    "bitcoin\.wiki"
)

$filePatterns = @(
    "*.cpp", "*.h", "*.c", "*.hpp", "*.cc",
    "*.sh", "*.py", "*.mk", "*.am", "*.in",
    "*.md", "*.txt", "*.conf", "*.json", "*.xml",
    "*.1", "*.rc", "*.nsi", "*.plist"
)

$remainingReferences = @()
$totalFilesChecked = 0

foreach ($pattern in $filePatterns) {
    $files = Get-ChildItem -Path . -Recurse -Filter $pattern -ErrorAction SilentlyContinue | Where-Object {
        $_.FullName -notmatch "node_modules" -and 
        $_.FullName -notmatch "\.git" -and 
        $_.FullName -notmatch "build" -and
        $_.FullName -notmatch "dist" -and
        $_.FullName -notmatch "\.vscode" -and
        $_.FullName -notmatch "autom4te\.cache"
    }
    
    foreach ($file in $files) {
        $totalFilesChecked++
        try {
            $content = Get-Content $file.FullName -Raw -ErrorAction Stop
            
            foreach ($searchPattern in $searchPatterns) {
                if ($content -match $searchPattern) {
                    $remainingReferences += @{
                        File = $file.FullName
                        Pattern = $searchPattern
                        Line = ($content -split "`n" | Select-String $searchPattern | Select-Object -First 1).Line
                    }
                }
            }
        }
        catch {
            # Skip files that can't be read
        }
    }
}

Write-Host "`n=== VERIFICATION RESULTS ===" -ForegroundColor Green
Write-Host "Files Checked: $totalFilesChecked" -ForegroundColor White

if ($remainingReferences.Count -eq 0) {
    Write-Host "✅ SUCCESS: No Bitcoin/BTC references found!" -ForegroundColor Green
    Write-Host "All Bitcoin/BTC references have been successfully removed from SHAHCOIN Core." -ForegroundColor Green
} else {
    Write-Host "❌ WARNING: Found $($remainingReferences.Count) remaining Bitcoin/BTC references:" -ForegroundColor Red
    
    foreach ($ref in $remainingReferences) {
        Write-Host "  - $($ref.File)" -ForegroundColor Yellow
        Write-Host "    Pattern: $($ref.Pattern)" -ForegroundColor Cyan
        if ($ref.Line) {
            Write-Host "    Line: $($ref.Line.Trim())" -ForegroundColor Gray
        }
        Write-Host ""
    }
}

Write-Host "`n=== VERIFICATION COMPLETE ===" -ForegroundColor Green
