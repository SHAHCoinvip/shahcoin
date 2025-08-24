#!/bin/bash

# SHAHCOIN Icon Conversion Script
# Rewrites all .xpm icon files from their matching .png versions

echo "🔁 SHAHCOIN Icon Conversion Script"
echo "=================================="
echo "Rewriting .xpm icons from .png logos..."
echo ""

# Check if ImageMagick is available
if command -v convert >/dev/null 2>&1; then
    echo "✅ ImageMagick found: $(convert --version | head -n1)"
    CONVERT_CMD="convert"
elif command -v magick >/dev/null 2>&1; then
    echo "✅ ImageMagick found: $(magick --version | head -n1)"
    CONVERT_CMD="magick"
else
    echo "❌ ImageMagick not found. Please install ImageMagick first:"
    echo "   Ubuntu/Debian: sudo apt-get install imagemagick"
    echo "   CentOS/RHEL: sudo yum install ImageMagick"
    echo "   macOS: brew install imagemagick"
    echo "   Windows: Download from https://imagemagick.org/"
    exit 1
fi

# Define icon sizes to convert
ICON_SIZES=(16 32 64 128 256)

# Check if pixmaps directory exists
PIXMAPS_DIR="./share/pixmaps"
if [ ! -d "$PIXMAPS_DIR" ]; then
    echo "❌ Pixmaps directory not found: $PIXMAPS_DIR"
    exit 1
fi

echo "📁 Working directory: $PIXMAPS_DIR"
echo ""

# Convert each icon size
for size in "${ICON_SIZES[@]}"; do
    PNG_FILE="$PIXMAPS_DIR/shahcoin${size}.png"
    XPM_FILE="$PIXMAPS_DIR/shahcoin${size}.xpm"
    
    echo "🔄 Converting shahcoin${size}.png → shahcoin${size}.xpm"
    
    # Check if PNG file exists
    if [ ! -f "$PNG_FILE" ]; then
        echo "   ⚠️  PNG file not found: $PNG_FILE"
        continue
    fi
    
    # Check if XPM file exists (for backup)
    if [ -f "$XPM_FILE" ]; then
        echo "   📋 Backing up existing XPM file..."
        cp "$XPM_FILE" "${XPM_FILE}.backup"
    fi
    
    # Convert PNG to XPM
    if $CONVERT_CMD "$PNG_FILE" "XPM:$XPM_FILE"; then
        echo "   ✅ Successfully converted shahcoin${size}.png → shahcoin${size}.xpm"
        
        # Verify the conversion
        if [ -f "$XPM_FILE" ]; then
            XPM_SIZE=$(wc -c < "$XPM_FILE")
            echo "   📊 XPM file size: ${XPM_SIZE} bytes"
        else
            echo "   ❌ XPM file was not created"
        fi
    else
        echo "   ❌ Failed to convert shahcoin${size}.png"
        # Restore backup if conversion failed
        if [ -f "${XPM_FILE}.backup" ]; then
            mv "${XPM_FILE}.backup" "$XPM_FILE"
            echo "   🔄 Restored original XPM file from backup"
        fi
    fi
    
    echo ""
done

echo "🎉 Icon conversion complete!"
echo "============================"
echo "✅ All .xpm files have been updated from their .png counterparts"
echo "📁 Location: $PIXMAPS_DIR"
echo ""
echo "📋 Converted files:"
for size in "${ICON_SIZES[@]}"; do
    XPM_FILE="$PIXMAPS_DIR/shahcoin${size}.xpm"
    if [ -f "$XPM_FILE" ]; then
        echo "   ✅ shahcoin${size}.xpm"
    else
        echo "   ❌ shahcoin${size}.xpm (failed)"
    fi
done

echo ""
echo "💡 Note: Original .png files remain unchanged"
echo "💡 Backup files (.xpm.backup) can be deleted if conversion was successful"
echo ""
echo "🔍 To verify conversion, check the .xpm files in: $PIXMAPS_DIR"
