#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

"$PROJECT_ROOT/scripts/linux/clean.sh"
"$PROJECT_ROOT/scripts/linux/build.sh"

