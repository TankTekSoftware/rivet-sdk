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
    echo "error: no iPhone simulator is available; create one in Xcode first." >&2
    exit 1
fi

xcrun simctl bootstatus "$device" -b

# Show the simulator, in DeviceHub.app from Xcode 27 on, or in Simulator.app before that.
developer_dir=${DEVELOPER_DIR:-$(xcode-select -p)}
if [ -d "$developer_dir/../Applications/DeviceHub.app" ]; then
    open "$developer_dir/../Applications/DeviceHub.app"
elif [ -d "$developer_dir/Applications/Simulator.app" ]; then
    open "$developer_dir/Applications/Simulator.app" --args -CurrentDeviceUDID "$device"
else
    echo "warning: found no app in Xcode that shows simulators; $bundle_id runs without a window." >&2
fi

xcrun simctl install "$device" "$app"
: > "$log" # simctl appends, and the log should only hold this run
xcrun simctl launch --terminate-running-process --stdout="$log" --stderr="$log" "$device" "$bundle_id"

echo "Launched $bundle_id. Its output goes to $log"
