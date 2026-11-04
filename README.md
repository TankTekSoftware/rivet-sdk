# Rivet SDK
A C++ game sdk, for cross-platform development

## Building

Builds use the CMake presets, either directly or through the Makefile (`make.bat` on Windows):

```sh
make PRESET=linux-debug
make.bat windows-debug
```

Without a preset, `make` builds the release preset for the computer it runs on, and `make.bat` builds `windows-release`.

| Presets | Build for | Build on |
| --- | --- | --- |
| `windows-debug`, `windows-release` | Windows 10/11 | Windows, or Linux with MinGW-w64 |
| `linux-debug`, `linux-release` | Linux | Linux |
| `macos-debug`, `macos-release` | macOS | macOS |

CMake only lists the presets that the computer it runs on can build (`cmake --list-presets`).

Each preset builds into `bin/<preset>/`.

## iOS

iOS builds need macOS with Xcode, and CMake 3.28 or newer. They follow [this guide to raylib on iOS](https://debugplay.com/posts/raylib-on-IOS): raylib comes from the [raylib-iOS](https://github.com/ghera/raylib-iOS) fork, which runs OpenGL ES on Metal through ANGLE. CMake generates an Xcode project (`out/build/<preset>/Rivet.xcodeproj`), and Xcode builds each game as a signed app bundle.

| Presets | Build for |
| --- | --- |
| `ios-simulator-debug`, `ios-simulator-release` | The iOS Simulator |
| `ios-debug`, `ios-release` | iPhone and iPad |

### Simulator

```sh
make PRESET=ios-simulator-debug
cmake --build --preset=ios-simulator-debug --target run_Pong
```

`run_Pong` installs Pong in the Simulator and launches it, booting the newest iPhone if no simulator is running. The game's output goes to `Pong.log`, next to `Pong.app`.

### Device

iOS only installs builds signed by a development team. Give CMake your team ID (shown in Xcode > Settings > Accounts) when you configure, either as an environment variable or with `-D`:

```sh
export RIVET_IOS_DEVELOPMENT_TEAM=ABCDE12345
make PRESET=ios-debug
```

The build tree remembers the team, so it only has to be given once per tree, and Xcode manages the certificate and provisioning profile. Then open `out/build/ios-debug/Rivet.xcodeproj` and run the `Pong` scheme on your device. A team picked in Xcode's Signing & Capabilities editor also works, but only until CMake regenerates the project.

Without a team, device builds stop with "Signing for "Pong" requires a development team". To only check that they compile, build them unsigned:

```sh
cmake --build --preset=ios-debug -- CODE_SIGNING_ALLOWED=NO
```

## Android

Android builds work on Windows, macOS and Linux. They need:

- The Android SDK, with an NDK, Build-Tools 35 or newer, an SDK platform and, to run in the emulator, the Android Emulator. Android Studio's SDK Manager installs all of these. CMake finds the SDK where Android Studio puts it, or at `ANDROID_HOME`, and uses the newest NDK in it, or the one at `ANDROID_NDK_HOME`.
- A JDK, 17 or newer: the one at `JAVA_HOME`, one on the `PATH`, or the one that comes with Android Studio.
- Ninja, as for desktop builds.

There is no Gradle project. raylib's own CMake builds its Android backend, the NDK builds each game as a shared library, and the SDK's tools package it into an APK. Android's NativeActivity loads the game, so the app has no Java code.

| Presets | Build for |
| --- | --- |
| `android-emulator-debug`, `android-emulator-release` | The Android Emulator, on this computer's architecture (x86_64, or arm64 on Apple silicon) |
| `android-debug`, `android-release` | Phones and tablets (arm64-v8a) |

Each build makes `bin/<preset>/<game>/<Game>.apk`, such as `bin/android-emulator-debug/pong/Pong.apk`. Apps run on Android 7.0 (API 24) and later, and target the newest SDK platform installed. Configure with `-DANDROID_PLATFORM=android-<API level>` or `-DRIVET_ANDROID_TARGET_SDK=<API level>` to change these.

### Emulator

```sh
make PRESET=android-emulator-debug
cmake --build --preset=android-emulator-debug --target run_Pong
```

`run_Pong` installs Pong in the emulator and launches it, starting the first Android Virtual Device if no emulator is running (create one in Android Studio's Device Manager). raylib logs to the device log, and `run_Pong` prints the `adb logcat` command that shows Pong's part of it.

### Device

Turn on USB debugging on the device (Settings > Developer options) and connect it, then:

```sh
make PRESET=android-debug
cmake --build --preset=android-debug --target run_Pong
```

When several devices are connected, set `ANDROID_SERIAL` to the one to use (`adb devices` lists their serials).

### Signing

Android only installs signed apps. Without a keystore, builds are signed with the debug key that Android Studio and Gradle share (`~/.android/debug.keystore`, created on the first build if it's missing). That suits testing, but not publishing. To sign with your own key, give CMake the keystore when you configure, either as environment variables or with `-D`, and give the keystore's password as an environment variable when you build. apksigner reads the password as it signs, so it's never stored:

```sh
export RIVET_ANDROID_KEYSTORE=/path/to/release.keystore
export RIVET_ANDROID_KEY_ALIAS=upload    # only needed when the keystore holds more than one key
export RIVET_ANDROID_KEYSTORE_PASSWORD=...
make PRESET=android-release
```

The build tree remembers the keystore and alias. Android only updates an installed app with an APK signed by the same key, so switching keys means uninstalling the app first; `run_<target>` explains how when it happens.

Google Play takes Android App Bundles (`.aab`) rather than APKs for new apps. These builds make APKs, for installing directly or for other stores.

### Limitations

- raylib's Android backend can't follow the screen as it changes size, so the game is held in one orientation (see `ORIENTATION` below), and it can't run in split screen or in a resizable window.
- A configuration change that the app doesn't handle itself, such as turning Bold text on or off in the accessibility settings, makes Android recreate the app, so the game starts over.
- The back button does nothing (raylib ignores it); leave the game with the home gesture or button.
- Apps get Android's default icon, and there's no way yet to package game assets into the APK.

## Making a game a mobile app

Games include `Rivet/EntryPoint.hpp` as on desktop. iOS runs the game loop itself, so on iOS the header provides the callbacks that raylib's iOS backend calls, instead of `main()`. On Android, raylib's backend calls the game's `main()` itself.

In the game's `CMakeLists.txt`, add the executable with `rivet_add_executable()` instead of `add_executable()`. Android loads games as shared libraries, so that's what it builds there; everywhere else it is `add_executable()`. Then:

```cmake
rivet_add_executable(Pong ${SOURCES})

rivet_configure_app(Pong
    BUNDLE_ID "com.tanktek.pong"
    DISPLAY_NAME "Rivet Pong"
    ORIENTATION portrait
)
```

On iOS this makes the executable an app bundle, with an Info.plist, the embedded ANGLE frameworks and code signing, and adds the `run_<target>` target. On Android it makes a signed APK, whose manifest starts the game in a NativeActivity, and adds the `run_<target>` target. Android holds the game in `ORIENTATION`, which is `portrait` or `landscape` (the default); iOS rotates freely. Android also needs a version number that grows with each release, which comes from the project's version: `project(Pong VERSION 1.2.3)` makes it 10203. On desktop, `rivet_configure_app()` does nothing.
