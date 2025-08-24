@echo off
REM Manual SHAHCOIN Icon Conversion Script
REM Simple script to convert PNG to XPM using ImageMagick

echo 🔁 Manual SHAHCOIN Icon Conversion
echo ==================================
echo.

REM Set the pixmaps directory
set PIXMAPS_DIR=.\share\pixmaps

echo 📁 Converting icons in: %PIXMAPS_DIR%
echo.

REM Try to convert each icon size
echo 🔄 Converting shahcoin16.png → shahcoin16.xpm
magick "%PIXMAPS_DIR%\shahcoin16.png" "XPM:%PIXMAPS_DIR%\shahcoin16.xpm"
if %ERRORLEVEL%==0 echo ✅ shahcoin16.xpm converted
echo.

echo 🔄 Converting shahcoin32.png → shahcoin32.xpm
magick "%PIXMAPS_DIR%\shahcoin32.png" "XPM:%PIXMAPS_DIR%\shahcoin32.xpm"
if %ERRORLEVEL%==0 echo ✅ shahcoin32.xpm converted
echo.

echo 🔄 Converting shahcoin64.png → shahcoin64.xpm
magick "%PIXMAPS_DIR%\shahcoin64.png" "XPM:%PIXMAPS_DIR%\shahcoin64.xpm"
if %ERRORLEVEL%==0 echo ✅ shahcoin64.xpm converted
echo.

echo 🔄 Converting shahcoin128.png → shahcoin128.xpm
magick "%PIXMAPS_DIR%\shahcoin128.png" "XPM:%PIXMAPS_DIR%\shahcoin128.xpm"
if %ERRORLEVEL%==0 echo ✅ shahcoin128.xpm converted
echo.

echo 🔄 Converting shahcoin256.png → shahcoin256.xpm
magick "%PIXMAPS_DIR%\shahcoin256.png" "XPM:%PIXMAPS_DIR%\shahcoin256.xpm"
if %ERRORLEVEL%==0 echo ✅ shahcoin256.xpm converted
echo.

echo 🎉 Manual conversion complete!
echo =============================
echo.
pause
