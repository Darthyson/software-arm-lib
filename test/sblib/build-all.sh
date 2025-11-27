#!/bin/bash
# Build all CMake presets for sblib

set -e

# Parse command line arguments
CLEAN=false
CONFIGURE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --configure)
            CONFIGURE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--clean] [--configure]"
            echo "  --clean      Clean build directories before building"
            echo "  --configure  Force reconfigure before building"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Define all build presets
BUILD_PRESETS=(
    "debug-x86"
    "release-x86"
    "debug-x64"
    "release-x64"
)

# Define corresponding configure presets
declare -A CONFIGURE_PRESETS=(
    ["debug-x86"]="x86"
    ["release-x86"]="x86"
    ["debug-x64"]="x64"
    ["release-x64"]="x64"
)

echo "====================================="
echo "Building all sblib-test presets"
echo "====================================="
echo ""

SUCCESSFUL=()
FAILED=()

for preset in "${BUILD_PRESETS[@]}"; do
    echo "----------------------------------------"
    echo "Building preset: $preset"
    echo "----------------------------------------"
    
    # Get configure preset
    config_preset="${CONFIGURE_PRESETS[$preset]}"
    build_dir="cmake-build/$config_preset"
    
    # Configure if requested or if build directory doesn't exist
    if [ "$CONFIGURE" = true ] || [ "$CLEAN" = true ] || [ ! -d "$build_dir" ]; then
        echo "Configuring preset: $config_preset"
        
        if [ "$CLEAN" = true ] && [ -d "$build_dir" ]; then
            echo "Cleaning build directory: $build_dir"
            rm -rf "$build_dir"
        fi
        
        if ! cmake --preset "$config_preset"; then
            echo "✗ Failed to configure: $config_preset"
            FAILED+=("$preset")
            echo ""
            continue
        fi
    fi
    
    # Build
    echo "Building preset: $preset"
    if cmake --build --preset "$preset"; then
        echo "✓ Successfully built: $preset"
        SUCCESSFUL+=("$preset")
    else
        echo "✗ Failed to build: $preset"
        FAILED+=("$preset")
    fi
    
    echo ""
done

# Summary
echo "====================================="
echo "Build Summary sblib-test"
echo "====================================="
echo "Successful: ${#SUCCESSFUL[@]}/${#BUILD_PRESETS[@]}"
for preset in "${SUCCESSFUL[@]}"; do
    echo "  ✓ $preset"
done

if [ ${#FAILED[@]} -gt 0 ]; then
    echo "Failed: ${#FAILED[@]}/${#BUILD_PRESETS[@]}"
    for preset in "${FAILED[@]}"; do
        echo "  ✗ $preset"
    done
    exit 1
fi

echo ""
echo "All builds completed successfully!"
