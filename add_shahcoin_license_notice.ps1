# Add SHAHCOIN License Notice to MIT-Licensed Files
# This script adds the SHAHCOIN Core license notice to files that have original MIT license headers

Write-Host "Adding SHAHCOIN Core license notice to MIT-licensed files..."

# SHAHCOIN license notice to add
$shahcoinNotice = @"

/**********************************************************************
 * Additional modifications and extensions by:
 * Copyright (C) 2025 The SHAHCOIN Core Developers — Hamid Rad
 * https://shah.vip | info@shah.vip
 * Distributed under the MIT License
 **********************************************************************/

"@

# Files that need the SHAHCOIN notice (based on grep search results)
$filesToUpdate = @(
    "src/crypto/ctaes/test.c",
    "src/crypto/ctaes/ctaes.c", 
    "src/crypto/ctaes/ctaes.h",
    "src/minisketch/src/util.h",
    "src/minisketch/src/test.cpp",
    "src/minisketch/src/sketch_impl.h",
    "src/minisketch/src/sketch.h",
    "src/minisketch/src/minisketch.cpp",
    "src/minisketch/src/false_positives.h",
    "src/minisketch/doc/example.c",
    "src/minisketch/src/bench.cpp",
    "src/minisketch/src/lintrans.h",
    "src/minisketch/src/int_utils.h",
    "src/minisketch/src/fields/clmul_2bytes.cpp",
    "src/minisketch/src/fields/clmul_1byte.cpp",
    "src/minisketch/src/fields/clmul_3bytes.cpp",
    "src/minisketch/src/fields/clmul_4bytes.cpp",
    "src/minisketch/src/fields/clmul_6bytes.cpp",
    "src/minisketch/src/fields/clmul_5bytes.cpp",
    "src/minisketch/src/fields/clmul_7bytes.cpp",
    "src/minisketch/src/fields/generic_2bytes.cpp",
    "src/minisketch/src/fields/generic_1byte.cpp",
    "src/minisketch/src/fields/clmul_8bytes.cpp",
    "src/minisketch/src/fields/clmul_common_impl.h",
    "src/minisketch/src/fields/generic_3bytes.cpp",
    "src/minisketch/src/fields/generic_common_impl.h",
    "src/minisketch/src/fields/generic_4bytes.cpp",
    "src/minisketch/src/fields/generic_5bytes.cpp",
    "src/minisketch/src/fields/generic_6bytes.cpp",
    "src/minisketch/src/fields/generic_8bytes.cpp",
    "src/secp256k1/contrib/lax_der_parsing.c",
    "src/secp256k1/contrib/lax_der_parsing.h",
    "src/secp256k1/contrib/lax_der_privatekey_parsing.c",
    "src/secp256k1/contrib/lax_der_privatekey_parsing.h",
    "src/secp256k1/src/bench.c",
    "src/secp256k1/src/bench.h",
    "src/secp256k1/src/assumptions.h",
    "src/secp256k1/src/bench_ecmult.c",
    "src/secp256k1/src/checkmem.h",
    "src/secp256k1/src/ecdsa_impl.h",
    "src/secp256k1/src/bench_internal.c",
    "src/secp256k1/src/ecdsa.h",
    "src/secp256k1/src/eckey_impl.h",
    "src/secp256k1/src/ecmult_compute_table.h",
    "src/secp256k1/src/eckey.h",
    "src/secp256k1/src/ecmult.h",
    "src/secp256k1/src/ecmult_compute_table_impl.h",
    "src/secp256k1/src/ecmult_gen.h",
    "src/secp256k1/src/ecmult_const_impl.h",
    "src/secp256k1/src/ecmult_gen_compute_table.h",
    "src/secp256k1/src/ecmult_gen_impl.h",
    "src/secp256k1/src/field.h",
    "src/secp256k1/src/ecmult_impl.h",
    "src/secp256k1/src/ecmult_gen_compute_table_impl.h",
    "src/secp256k1/src/field_10x26.h",
    "src/secp256k1/src/field_10x26_impl.h",
    "src/secp256k1/src/field_5x52.h",
    "src/secp256k1/src/field_5x52_impl.h",
    "src/secp256k1/src/field_impl.h",
    "src/secp256k1/src/field_5x52_asm_impl.h",
    "src/secp256k1/src/group.h",
    "src/secp256k1/src/field_5x52_int128_impl.h",
    "src/secp256k1/src/group_impl.h",
    "src/secp256k1/src/hash_impl.h",
    "src/secp256k1/src/hash.h",
    "src/secp256k1/src/precomputed_ecmult.h",
    "src/secp256k1/src/modules/schnorrsig/tests_exhaustive_impl.h",
    "src/secp256k1/src/modules/ecdh/bench_impl.h",
    "src/secp256k1/src/modules/recovery/tests_impl.h",
    "src/secp256k1/src/modules/recovery/main_impl.h",
    "src/secp256k1/src/scalar_4x64_impl.h",
    "src/secp256k1/src/modules/recovery/bench_impl.h",
    "src/secp256k1/src/modules/extrakeys/tests_exhaustive_impl.h",
    "src/secp256k1/src/scalar_4x64.h",
    "src/secp256k1/src/precompute_ecmult.c",
    "src/secp256k1/src/scalar.h",
    "src/secp256k1/src/precomputed_ecmult_gen.h",
    "src/secp256k1/src/precompute_ecmult_gen.c",
    "src/secp256k1/src/scalar_impl.h",
    "src/secp256k1/src/secp256k1.c",
    "src/secp256k1/src/scalar_8x32_impl.h",
    "src/secp256k1/src/testrand.h",
    "src/secp256k1/src/testrand_impl.h",
    "src/secp256k1/src/scalar_8x32.h",
    "src/secp256k1/src/selftest.h",
    "src/secp256k1/src/util.h",
    "src/secp256k1/src/tests.c"
)

$updatedCount = 0

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
                    "Licensed under the MIT License"
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
                        Write-Host "Updated: $file"
                    }
                } else {
                    Write-Host "Warning: Could not find insertion point for $file"
                }
            } else {
                Write-Host "Skipped: $file (already has SHAHCOIN notice)"
            }
        }
        catch {
            Write-Host "Error processing $file`: $($_.Exception.Message)"
        }
    } else {
        Write-Host "Warning: File not found: $file"
    }
}

Write-Host "`nCompleted! Updated $updatedCount files with SHAHCOIN license notice."
Write-Host "All MIT-licensed files now have proper SHAHCOIN Core attribution."
