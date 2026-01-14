#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
PACKAGE_NAME="wav-splitter"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"

# --- Version from git ---
get_version() {
    local desc
    desc="$(git -C "$PROJECT_ROOT" describe --tags --dirty --long --always 2>/dev/null || echo "v0.0.0")"
    desc="${desc#v}"  # strip leading v
    echo "$desc"
}

VERSION="$(get_version)"
ARCHIVE_NAME="$PACKAGE_NAME-$VERSION"
ARCHIVE_PATH="$DIST_DIR/$ARCHIVE_NAME.tar.gz"

echo "Creating tar.gz archive for $PACKAGE_NAME ($VERSION)..."
echo "Project root: $PROJECT_ROOT"

# --- Build project ---
"$PROJECT_ROOT/scripts/linux/cleanbuild.sh"

# --- Prepare staging directory ---
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR/$ARCHIVE_NAME"

cp "$BUILD_DIR/$PACKAGE_NAME" "$DIST_DIR/$ARCHIVE_NAME/"
cp "$PROJECT_ROOT/README.md" "$DIST_DIR/$ARCHIVE_NAME/" 2>/dev/null || true
cp "$PROJECT_ROOT/LICENSE" "$DIST_DIR/$ARCHIVE_NAME/" 2>/dev/null || true

# --- Create tar.gz ---
tar -czf "$ARCHIVE_PATH" -C "$DIST_DIR" "$ARCHIVE_NAME"

# --- Cleanup staging folder ---
rm -rf "$DIST_DIR/$ARCHIVE_NAME"

echo "tar.gz archive created:"
echo "  $ARCHIVE_PATH"

