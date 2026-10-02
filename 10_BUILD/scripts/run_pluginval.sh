#!/usr/bin/env bash
# Validate the built FOUNDATION VST3 (and AU) with pluginval on macOS (strictness 10).
#   ./10_BUILD/scripts/run_pluginval.sh [Debug|RelWithDebInfo|Release]
# pluginval is located via $FOUNDATION_PLUGINVAL, PATH, or 10_BUILD/tools/pluginval.app.
# Exit code 0 = passed, 1 = failed, 2 = pluginval not available.
set -uo pipefail

configuration="${1:-Release}"
configure_preset="${FOUNDATION_CONFIGURE_PRESET:-macos-xcode}"
root="$(cd "$(dirname "$0")/../.." && pwd)"

pluginval="${FOUNDATION_PLUGINVAL:-}"
if [[ -z "$pluginval" ]]; then
    pluginval="$(command -v pluginval || true)"
fi
if [[ -z "$pluginval" && -x "$root/10_BUILD/tools/pluginval.app/Contents/MacOS/pluginval" ]]; then
    pluginval="$root/10_BUILD/tools/pluginval.app/Contents/MacOS/pluginval"
fi
if [[ -z "$pluginval" ]]; then
    echo "pluginval not found" >&2
    exit 2
fi

artefacts="$(dirname "$root")/FOUNDATION_build/$configure_preset/11_PLUGIN/Foundation_artefacts/$configuration"
status=0

for plugin in "$artefacts/VST3/FOUNDATION.vst3" "$artefacts/AU/FOUNDATION.component"; do
    if [[ -e "$plugin" ]]; then
        echo "pluginval: $plugin"
        "$pluginval" --strictness-level 10 --validate-in-process --validate "$plugin" || status=1
    fi
done

exit $status
