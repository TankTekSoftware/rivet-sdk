# Installs an APK on an Android device or emulator and launches it. Emulator builds run in an
# emulator, booting the first Android Virtual Device if none is running; device builds run on a
# connected device, or else an emulator. ANDROID_SERIAL picks one by its serial (as `adb devices`
# lists it). The run_<target> targets made by rivet_configure_app() call it, and it also works on
# its own with any APK:
#
#   cmake -DADB=<adb> -DEMULATOR=<emulator> -DAPK=<App.apk> -DPACKAGE=<app ID> [-DUSE_EMULATOR=ON]
#         -P run-device.cmake
cmake_minimum_required(VERSION 3.25)

if(NOT EXISTS "${ADB}")
    message(FATAL_ERROR
        "Found no adb at ${ADB}. Install it with Android Studio's SDK Manager (SDK Tools > Android "
        "SDK Platform-Tools)."
    )
endif()

# Runs adb with the given arguments, and stops if it fails. OUTPUT_VARIABLE <var> keeps what it
# printed.
function(run_adb)
    cmake_parse_arguments(PARSE_ARGV 0 RUN "" "OUTPUT_VARIABLE" "")
    execute_process(
        COMMAND "${ADB}" ${RUN_UNPARSED_ARGUMENTS}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output
    )
    if(NOT result EQUAL 0)
        list(JOIN RUN_UNPARSED_ARGUMENTS " " command)
        message(FATAL_ERROR "adb ${command} failed:\n${output}")
    endif()
    if(RUN_OUTPUT_VARIABLE)
        set(${RUN_OUTPUT_VARIABLE} "${output}" PARENT_SCOPE)
    endif()
endfunction()

# Gives the serials of the emulators and of the other devices that are ready to use.
function(list_devices emulators_var devices_var)
    run_adb(devices OUTPUT_VARIABLE output)
    string(REGEX MATCHALL "[^\r\n]+" lines "${output}")

    set(emulators "")
    set(devices "")
    foreach(line IN LISTS lines)
        if(line MATCHES "^([^ \t]+)[ \t]+device$")
            set(serial "${CMAKE_MATCH_1}")
            if(serial MATCHES "^emulator-")
                list(APPEND emulators "${serial}")
            else()
                list(APPEND devices "${serial}")
            endif()
        endif()
    endforeach()

    set(${emulators_var} "${emulators}" PARENT_SCOPE)
    set(${devices_var} "${devices}" PARENT_SCOPE)
endfunction()

# Boots the first Android Virtual Device and gives its serial once Android has started in it.
function(boot_emulator serial_var)
    if(NOT EXISTS "${EMULATOR}")
        message(FATAL_ERROR
            "No emulator is running, and found none to start at ${EMULATOR}. Install it with Android "
            "Studio's SDK Manager (SDK Tools > Android Emulator)."
        )
    endif()

    # AVD names are made of letters, digits, '.', '_' and '-'; other lines are the emulator's own
    # messages.
    execute_process(COMMAND "${EMULATOR}" -list-avds OUTPUT_VARIABLE output ERROR_QUIET)
    string(REGEX MATCHALL "[^\r\n]+" lines "${output}")
    set(avds "")
    foreach(line IN LISTS lines)
        if(line MATCHES "^[A-Za-z0-9._-]+$")
            list(APPEND avds "${line}")
        endif()
    endforeach()
    if(NOT avds)
        message(FATAL_ERROR
            "No emulator is running, and there's no Android Virtual Device to start. Make one in "
            "Android Studio's Device Manager."
        )
    endif()
    list(GET avds 0 avd)

    # The emulator is started apart from this script, so it keeps running once the script ends,
    # and without its output, which the script would otherwise wait on until the emulator quits.
    message(STATUS "Starting the ${avd} emulator")
    if(CMAKE_HOST_WIN32)
        execute_process(COMMAND powershell -NoProfile -NonInteractive -Command
            "Start-Process -FilePath '${EMULATOR}' -ArgumentList '-avd','${avd}' -WindowStyle Hidden"
        )
    else()
        execute_process(COMMAND sh -c "nohup \"$0\" -avd \"$1\" >/dev/null 2>&1 &" "${EMULATOR}" "${avd}")
    endif()

    # It's the only emulator, so -e finds it. The first boot of an AVD can take a few minutes.
    execute_process(COMMAND "${ADB}" -e wait-for-device TIMEOUT 300 RESULT_VARIABLE result)
    if(result EQUAL 0)
        execute_process(
            COMMAND "${ADB}" -e shell "while [ \"$(getprop sys.boot_completed)\" != 1 ]; do sleep 1; done"
            TIMEOUT 300
            RESULT_VARIABLE result
        )
    endif()
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "The ${avd} emulator didn't finish starting (${result}). Start it from Android Studio's "
            "Device Manager to see why."
        )
    endif()

    run_adb(-e get-serialno OUTPUT_VARIABLE serial)
    string(STRIP "${serial}" serial)
    set(${serial_var} "${serial}" PARENT_SCOPE)
endfunction()

run_adb(start-server)

if(DEFINED ENV{ANDROID_SERIAL})
    set(serial "$ENV{ANDROID_SERIAL}")
else()
    list_devices(emulators devices)

    # Emulator builds are made for this computer's architecture, which devices needn't have.
    if(USE_EMULATOR)
        set(candidates ${emulators})
    else()
        set(candidates ${devices} ${emulators})
    endif()

    if(candidates)
        list(GET candidates 0 serial)
        list(LENGTH candidates count)
        if(count GREATER 1)
            message(STATUS "Using ${serial}. Set ANDROID_SERIAL to use another (adb devices lists them).")
        endif()
    elseif(USE_EMULATOR)
        boot_emulator(serial)
    else()
        message(FATAL_ERROR
            "No Android device is connected. Connect one by USB with USB debugging turned on "
            "(Settings > Developer options), or build an android-emulator preset to run in the "
            "emulator."
        )
    endif()
endif()

message(STATUS "Installing ${APK} on ${serial}")
execute_process(
    COMMAND "${ADB}" -s "${serial}" install -r "${APK}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
if(NOT result EQUAL 0)
    # Android only updates an app with an APK signed by the same key, which the debug and release
    # keys aren't.
    if(output MATCHES "INSTALL_FAILED_UPDATE_INCOMPATIBLE")
        string(APPEND output
            "\n${PACKAGE} is already installed, signed with another key. Uninstall it first: "
            "\"${ADB}\" -s ${serial} uninstall ${PACKAGE}"
        )
    endif()
    message(FATAL_ERROR "Couldn't install ${APK} on ${serial}:\n${output}")
endif()

# -S stops the app first if it's running already, and -W waits until it has started.
run_adb(-s "${serial}" shell am start -S -W -n "${PACKAGE}/android.app.NativeActivity" OUTPUT_VARIABLE output)
if(output MATCHES "Error")
    message(FATAL_ERROR "Couldn't launch ${PACKAGE} on ${serial}:\n${output}")
endif()

execute_process(
    COMMAND "${ADB}" -s "${serial}" shell pidof -s "${PACKAGE}"
    OUTPUT_VARIABLE pid
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(pid)
    set(log_filter " --pid=${pid}")
endif()
message(STATUS
    "Launched ${PACKAGE} on ${serial}. raylib logs to the device log, which this shows: "
    "\"${ADB}\" -s ${serial} logcat${log_filter}"
)
