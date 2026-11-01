#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

echo "=========================================================="
echo " CLICKCLACK — Build Process (macOS Apple Silicon ARM64)  "
echo "=========================================================="

# Ensure Essentia static library exists
ESSENTIA_LIB="${PROJECT_DIR}/deps/essentia/lib/libessentia.a"
if [ ! -f "${ESSENTIA_LIB}" ]; then
    ESSENTIA_LIB="/tmp/clickclack_deps/essentia/build/src/libessentia.a"
fi
if [ ! -f "${ESSENTIA_LIB}" ]; then
    echo "Essentia static library not found!"
    echo "Building Essentia first..."
    cd /tmp/clickclack_deps/essentia
    python3 waf build -j8
fi

echo "Configuring ClickClack via CMake..."
cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64"

echo "Building ClickClack targets (VST3, AU, Standalone)..."
cmake --build "${BUILD_DIR}" --config Release -j8

echo ""
echo "=========================================================="
echo " BUILD SUCCESSFUL!"
echo " Artifacts located at: ${BUILD_DIR}/ClickClack_artefacts/Release"
echo "=========================================================="
ls -la "${BUILD_DIR}/ClickClack_artefacts/Release" 2>/dev/null || true
