#!/bin/bash
# Behaviac Behavior Tree Reimport Script
# Simple and reliable: Delete cache + Instructions
# Usage: ./reimport_bt.sh [BT_name]
# Example: ./reimport_bt.sh BT_Test_Simple

CONTENT_DIR="/Volumes/M2/Works/TopDownBehaviacTest/Content/AI"

if [ $# -eq 0 ]; then
    echo "❌ Usage: $0 <BT_name>"
    echo "   Example: $0 BT_Test_Simple"
    echo "   Example: $0 Test_Simple  (BT_ prefix optional)"
    exit 1
fi

# Normalize name
BT_NAME="${1#BT_}"
BT_NAME="BT_${BT_NAME}"

XML_FILE="${CONTENT_DIR}/${BT_NAME}.xml"
UASSET_FILE="${CONTENT_DIR}/${BT_NAME}.uasset"

echo "🔄 Preparing to reimport: ${BT_NAME}"
echo ""

# Check if XML exists
if [ ! -f "${XML_FILE}" ]; then
    echo "❌ XML file not found: ${XML_FILE}"
    exit 1
fi

XML_SIZE=$(stat -f%z "${XML_FILE}")
echo "✅ XML file found: ${XML_FILE}"
echo "   Size: ${XML_SIZE} bytes"
echo "   Modified: $(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "${XML_FILE}")"
echo ""

# Delete uasset if exists
if [ -f "${UASSET_FILE}" ]; then
    UASSET_SIZE=$(stat -f%z "${UASSET_FILE}")
    echo "🗑️  Deleting cached .uasset file..."
    echo "   Size: ${UASSET_SIZE} bytes"
    rm -f "${UASSET_FILE}"
    echo "   ✅ Deleted"
else
    echo "ℹ️  No existing .uasset cache found (first-time import)"
fi

echo ""
echo "📝 Next Steps:"
echo ""
echo "   Option 1 - Use Editor Reimport (Recommended):"
echo "   ==========================================="
echo "   1. Open UE5 Editor"
echo "   2. Right-click '${BT_NAME}' in Content Browser"
echo "   3. Click 'Reimport' → Select XML file"
echo ""
echo "   Option 2 - Auto-reimport on Editor Restart:"
echo "   =========================================="
echo "   1. Restart the UE5 Editor"
echo "   2. The .uasset will auto-generate from XML"
echo ""
echo "   Option 3 - Drag & Drop:"
echo "   ======================"
echo "   1. Open UE5 Editor"
echo "   2. Drag '${XML_FILE##*/}' into Content Browser → AI folder"
echo ""

# Check if editor is running
if pgrep -f "UnrealEditor.*TopDownBehaviacTest" > /dev/null; then
    echo "⚠️  UE5 Editor is currently running!"
    echo "   For Option 1: Use Reimport button now"
    echo "   For Option 2: Restart the editor"
fi

exit 0
