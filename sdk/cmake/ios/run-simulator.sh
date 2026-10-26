#!/bin/sh
# Installs an app in the iOS Simulator and launches it, booting a simulator first if none is
# running. The run_<target> targets made by rivet_configure_app() call it, and it also works on
# its own with any simulator build.
#
#   sh run-simulator.sh <path/to/App.app>
set -eu

app=${1%/}
bundle_id=$(/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$app/Info.plist")
log=${app%.app}.log

uuid='[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}'

# Use the simulator that is already running, or else the newest iPhone (runtimes are listed
# oldest first).
device=$(xcrun simctl list devices booted | grep -oE "$uuid" | head -n 1)
if [ -z "$device" ]; then
    device=$(xcrun simctl list devices available | grep "iPhone" | grep -oE "$uuid" | tail -n 1)
fi
if [ -z "$device" ]; then
    echo "error: no iPhone simulator is available; add one in Xcode > Window > Devices and Simulators." >&2
    exit 1
fi

xcrun simctl bootstatus "$device" -b
open -a Simulator --args -CurrentDeviceUDID "$device"

xcrun simctl install "$device" "$app"
xcrun simctl launch --terminate-running-process --stdout="$log" --stderr="$log" "$device" "$bundle_id"

echo "Launched $bundle_id. Its output goes to $log"
