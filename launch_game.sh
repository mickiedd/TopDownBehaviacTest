#!/bin/bash
set -euo pipefail

# launch_game.sh
# Build then launch the Unreal Editor with the TopDownBehaviacTest project.
# Usage: ./launch_game.sh [path/to/Project.uproject]

# Resolve base directory (script location)
BASE_DIR="$(cd "$(dirname "$0")" && pwd)"

# Default uproject next to this script, or first arg if provided
UPROJECT_PATH="${1:-$BASE_DIR/TopDownBehaviacTest.uproject}"

# Paths to Unreal app and binary
UNREAL_APP="/Volumes/M2/Engine/UE_5.5/Engine/Binaries/Mac/UnrealEditor.app"
UNREAL_BIN="$UNREAL_APP/Contents/MacOS/UnrealEditor"
UBT="/Volumes/M2/Engine/UE_5.5/Engine/Build/BatchFiles/Mac/Build.sh"

# Log output
LOG_DIR="$BASE_DIR/Saved/Logs"
mkdir -p "$LOG_DIR"

TIMESTAMP="$(date +%Y.%m.%d-%H.%M.%S)"
RUN_LOG_PATH="$LOG_DIR/TopDownBehaviacTestEditor-$TIMESTAMP.log"
LATEST_LOG_PATH="$LOG_DIR/TopDownBehaviacTestEditor-latest.log"

# Arguments
LOG_ARG="-log"
ABS_LOG_ARG="-AbsLog=$RUN_LOG_PATH"
FORCE_LOG_FLUSH_ARG="-FORCELOGFLUSH"
STDOUT_ARG="-stdout"
SAVE_DIR_ARG="-SAVEDIR=$BASE_DIR/Saved"

if [ ! -f "$UPROJECT_PATH" ]; then
	echo "Error: .uproject not found: $UPROJECT_PATH" >&2
	exit 1
fi

# ── TypeScript compile ───────────────────────────────────────────────────────
echo "==> Compiling TypeScript..."
TSC=$(which tsc 2>/dev/null || true)
if [ -z "$TSC" ]; then
	echo "Warning: tsc not found in PATH, skipping TypeScript compile"
else
	"$TSC" -p "$BASE_DIR/TypeScript/tsconfig.json"
	echo "==> TypeScript compile succeeded."
fi

# ── Build ────────────────────────────────────────────────────────────────────
echo "==> Building TopDownBehaviacTestEditor (Development)..."
if [ ! -x "$UBT" ]; then
	echo "Error: UnrealBuildTool not found at $UBT" >&2
	exit 1
fi

"$UBT" TopDownBehaviacTestEditor Mac Development "$UPROJECT_PATH" -waitmutex
echo "==> Build succeeded."

# ── Launch ───────────────────────────────────────────────────────────────────
echo "==> Launching UnrealEditor..."
echo "==> Runtime log: $RUN_LOG_PATH"

rm -f "$LATEST_LOG_PATH"
ln -s "$RUN_LOG_PATH" "$LATEST_LOG_PATH"

if [ -x "$UNREAL_BIN" ]; then
	exec "$UNREAL_BIN" "$UPROJECT_PATH" "$LOG_ARG" "$ABS_LOG_ARG" "$FORCE_LOG_FLUSH_ARG" "$STDOUT_ARG" "$SAVE_DIR_ARG"
else
	echo "Warning: UnrealEditor binary not executable at $UNREAL_BIN; trying open -a fallback"
	chmod +x "$UNREAL_APP" 2>/dev/null || true
	open -a "$UNREAL_APP" "$UPROJECT_PATH" --args "$LOG_ARG" "$ABS_LOG_ARG" "$FORCE_LOG_FLUSH_ARG" "$STDOUT_ARG" "$SAVE_DIR_ARG"
fi
