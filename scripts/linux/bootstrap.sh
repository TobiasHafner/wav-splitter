#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "Bootstrapping build environment for wav-splitter..."
echo "Project root: $PROJECT_ROOT"

sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    fakeroot \
    dpkg-dev

echo "Bootstrap completed."

