@echo off
REM SHAHCOIN Icon Conversion Script for Windows
REM Rewrites all .xpm icon files from their matching .png versions

echo 🔁 SHAHCOIN Icon Conversion Script
echo ==================================
echo Rewriting .xpm icons from .png logos...
echo.

REM Check if ImageMagick is available
where magick >nul 2>&1
if %ERRORLEVEL%==0 (
    echo ✅ ImageMagick found
    set CONVERT_CMD=magick
    goto :convert_icons
)

where convert >nul 2>&1
if %ERRORLEVEL%==0 (
    echo ✅ ImageMagick convert found
    set CONVERT_CMD=convert
    goto :convert_icons
)

REM Try common ImageMagick installation paths
if exist "C:\Program Files\ImageMagick-7.1.1-Q16\magick.exe" (
    echo ✅ ImageMagick found in Program Files
    set CONVERT_CMD="C:\Program Files\ImageMagick-7.1.1-Q16\magick.exe"
    goto :convert_icons
)

if exist "C:\Program Files\ImageMagick-7.0.11-Q16\magick.exe" (
    echo ✅ ImageMagick found in Program Files
    set CONVERT_CMD="C:\Program Files\ImageMagick-7.0.11-Q16\magick.exe"
    goto :convert_icons
)

if exist "C:\Program Files (x86)\ImageMagick-7.1.1-Q16\magick.exe" (
    echo ✅ ImageMagick found in Program Files (x86)
    set CONVERT_CMD="C:\Program Files (x86)\ImageMagick-7.1.1-Q16\magick.exe"
    goto :convert_icons
)

echo ❌ ImageMagick not found. Please install ImageMagick first:
echo    Download from: https://imagemagick.org/script/download.php#windows
echo    Make sure to check "Add application directory to your system path" during installation
echo.
pause
exit /b 1

:convert_icons
REM Define icon sizes to convert
set ICON_SIZES=16 32 64 128 256

REM Check if pixmaps directory exists
set PIXMAPS_DIR=.\share\pixmaps
if not exist "%PIXMAPS_DIR%" (
    echo ❌ Pixmaps directory not found: %PIXMAPS_DIR%
    pause
    exit /b 1
)

echo 📁 Working directory: %PIXMAPS_DIR%
echo.

REM Convert each icon size
for %%s in (%ICON_SIZES%) do (
    set PNG_FILE=%PIXMAPS_DIR%\shahcoin%%s.png
    set XPM_FILE=%PIXMAPS_DIR%\shahcoin%%s.xpm
    
    echo 🔄 Converting shahcoin%%s.png → shahcoin%%s.xpm
    
    REM Check if PNG file exists
    if not exist "!PNG_FILE!" (
        echo    ⚠️  PNG file not found: !PNG_FILE!
        goto :continue
    )
    
    REM Check if XPM file exists (for backup)
    if exist "!XPM_FILE!" (
        echo    📋 Backing up existing XPM file...
        copy "!XPM_FILE!" "!XPM_FILE!.backup" >nul
    )
    
    REM Convert PNG to XPM
    %CONVERT_CMD% "!PNG_FILE!" "XPM:!XPM_FILE!"
    if %ERRORLEVEL%==0 (
        echo    ✅ Successfully converted shahcoin%%s.png → shahcoin%%s.xpm
        
        REM Verify the conversion
        if exist "!XPM_FILE!" (
            for %%A in ("!XPM_FILE!") do set XPM_SIZE=%%~zA
            echo    📊 XPM file size: !XPM_SIZE! bytes
        ) else (
            echo    ❌ XPM file was not created
        )
    ) else (
        echo    ❌ Failed to convert shahcoin%%s.png
        REM Restore backup if conversion failed
        if exist "!XPM_FILE!.backup" (
            move "!XPM_FILE!.backup" "!XPM_FILE!" >nul
            echo    🔄 Restored original XPM file from backup
        )
    )
    
    echo.
    :continue
)

echo 🎉 Icon conversion complete!
echo ============================
echo ✅ All .xpm files have been updated from their .png counterparts
echo 📁 Location: %PIXMAPS_DIR%
echo.
echo 📋 Converted files:
for %%s in (%ICON_SIZES%) do (
    set XPM_FILE=%PIXMAPS_DIR%\shahcoin%%s.xpm
    if exist "!XPM_FILE!" (
        echo    ✅ shahcoin%%s.xpm
    ) else (
        echo    ❌ shahcoin%%s.xpm (failed)
    )
)

echo.
echo 💡 Note: Original .png files remain unchanged
echo 💡 Backup files (.xpm.backup) can be deleted if conversion was successful
echo.
echo 🔍 To verify conversion, check the .xpm files in: %PIXMAPS_DIR%
echo.
pause
