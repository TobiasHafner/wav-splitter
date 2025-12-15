#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if ! git -C "$PROJECT_ROOT" describe --tags --exact-match >/dev/null 2>&1; then
    echo "ERROR: Not on an exact tag"
    exit 1
fi

if [[ -n "$(git -C "$PROJECT_ROOT" status --porcelain)" ]]; then
    echo "ERROR: Working tree is dirty"
    exit 1
fi

PACKAGE_NAME="wav-splitter"
ARCH="amd64"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
DEB_DIR="$DIST_DIR/$PACKAGE_NAME"

get_debian_version() {
    local desc

    desc="$(git -C "$PROJECT_ROOT" describe --tags --dirty --long --always 2>/dev/null || echo "v0.0.0")"

    # Strip leading "v"
    desc="${desc#v}"

    # If this is an exact tag (e.g. 0.1.0)
    if [[ "$desc" =~ ^[0-9] ]]; then
        :
    else
        echo "0.1.0"
        return
    fi

    # Convert git describe format to Debian version
    # 0.1.0-3-gabc123[-dirty]
    if [[ "$desc" =~ ^([0-9][^ -]*)-([0-9]+)-(g[0-9a-f]+)(-dirty)?$ ]]; then
        local base="${BASH_REMATCH[1]}"
        local commits="${BASH_REMATCH[2]}"
        local hash="${BASH_REMATCH[3]}"
        local dirty="${BASH_REMATCH[4]}"

        if [[ -n "$dirty" ]]; then
            echo "${base}+git${commits}+${hash}~dirty"
        else
            echo "${base}+git${commits}+${hash}"
        fi
    else
        # Exact tag case
        echo "$desc"
    fi
}

VERSION="$(get_debian_version)"

echo "Creating Debian package for $PACKAGE_NAME ($VERSION)..."
echo "Project root: $PROJECT_ROOT"

"$PROJECT_ROOT/scripts/linux/cleanbuild.sh"

rm -rf "$DIST_DIR"
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"

cp "$BUILD_DIR/$PACKAGE_NAME" "$DEB_DIR/usr/bin/"

cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: sound
Priority: optional
Architecture: $ARCH
Maintainer: Tobias Hafner
Description: Command-line tool to split multichannel WAV files into mono tracks
 wav-splitter splits and merges multichannel WAV recordings from
 Midas M32 and Behringer X32 multitrack sessions.
License: Apache-2.0
EOF

fakeroot dpkg-deb --build "$DEB_DIR"

echo "Debian package created:"
echo "  $DEB_DIR.deb"

