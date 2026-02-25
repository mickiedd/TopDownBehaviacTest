#!/bin/bash
# Quick Behavior Tree Cache Cleaner
# Deletes .uasset files to force fresh XML import
# Usage: ./clear_bt_cache.sh [BT_name...]

AI_CONTENT_DIR="/Volumes/M2/Works/TopDownBehaviacTest/Content/AI"

if [ $# -eq 0 ]; then
    echo "🗑️  Clearing ALL behavior tree caches in ${AI_CONTENT_DIR}"
    echo ""
    find "${AI_CONTENT_DIR}" -name "BT_*.uasset" -type f | while read file; do
        echo "   Deleting: $(basename "$file")"
        rm -f "$file"
    done
    echo ""
    echo "✅ Done! Restart UE5 Editor to reimport from XML"
else
    echo "🗑️  Clearing specific behavior tree caches:"
    echo ""
    for bt_name in "$@"; do
        # Remove BT_ prefix if provided
        bt_name="${bt_name#BT_}"
        file="${AI_CONTENT_DIR}/BT_${bt_name}.uasset"
        
        if [ -f "$file" ]; then
            echo "   Deleting: BT_${bt_name}.uasset"
            rm -f "$file"
        else
            echo "   ⚠️  Not found: BT_${bt_name}.uasset"
        fi
    done
    echo ""
    echo "✅ Done! Restart UE5 Editor to reimport from XML"
fi
