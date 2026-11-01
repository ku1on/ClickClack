#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
RELEASE_DIR="${BUILD_DIR}/ClickClack_artefacts/Release"

VST3_DEST="${HOME}/Library/Audio/Plug-Ins/VST3"
AU_DEST="${HOME}/Library/Audio/Plug-Ins/Components"
APP_DEST="/Applications"

echo "=========================================================="
echo " CLICKCLACK — Installation Process                        "
echo "=========================================================="

mkdir -p "${VST3_DEST}"
mkdir -p "${AU_DEST}"

# Install VST3
if [ -d "${RELEASE_DIR}/VST3/ClickClack.vst3" ]; then
    echo "Installing ClickClack.vst3 to ${VST3_DEST}..."
    rm -rf "${VST3_DEST}/ClickClack.vst3"
    cp -R "${RELEASE_DIR}/VST3/ClickClack.vst3" "${VST3_DEST}/"
    echo "✔ VST3 installed successfully."
fi

# Install AU (Component)
if [ -d "${RELEASE_DIR}/AU/ClickClack.component" ]; then
    echo "Installing ClickClack.component to ${AU_DEST}..."
    rm -rf "${AU_DEST}/ClickClack.component"
    cp -R "${RELEASE_DIR}/AU/ClickClack.component" "${AU_DEST}/"
    echo "✔ AU installed successfully."
fi

# Install Standalone App if present
if [ -d "${RELEASE_DIR}/Standalone/ClickClack.app" ]; then
    echo "Installing ClickClack.app to ${APP_DEST}..."
    rm -rf "${APP_DEST}/ClickClack.app"
    cp -R "${RELEASE_DIR}/Standalone/ClickClack.app" "${APP_DEST}/"
    echo "✔ Standalone App installed successfully."
fi

echo "Refreshing macOS Audio Unit cache..."
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo ""
echo "=========================================================="
echo " INSTALLATION COMPLETE!                                   "
echo " Ableton Live 11/12 can now discover ClickClack in VST3/AU"
echo "=========================================================="
