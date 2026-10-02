#!/usr/bin/env bash
# Configure, build and (optionally) test FOUNDATION on macOS.
#   ./10_BUILD/scripts/build.sh [Debug|RelWithDebInfo|Release] [--skip-tests]
set -euo pipefail

configuration="${1:-Debug}"
skip_tests="${2:-}"
configure_preset="${FOUNDATION_CONFIGURE_PRESET:-macos-xcode}"
prefix="macos"
[[ "$configure_preset" == ci-* ]] && prefix="ci-macos"
preset="${prefix}-$(echo "$configuration" | tr '[:upper:]' '[:lower:]')"

cd "$(dirname "$0")/../.."

cmake --preset "$configure_preset"
cmake --build --preset "$preset" --parallel

if [[ "$skip_tests" != "--skip-tests" ]]; then
    ctest --preset "$preset"
fi
