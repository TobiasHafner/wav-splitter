#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"

echo "Cleaning build directory..."
rm -rf "$BUILD_DIR"
echo "Cleaning dist directory..."
rm -rf "$DIST_DIR"

echo "Clean completed."

