# Rivet SDK
A C++ game sdk, for cross-platform development

## Building

Builds use the CMake presets, either directly or through the Makefile (`make.bat` on Windows):

```sh
make PRESET=desktop-release
```

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

Before a device build can be installed, it has to be signed by your development team. Set your team ID (shown in Xcode > Settings > Accounts) before configuring:

```sh
export RIVET_IOS_DEVELOPMENT_TEAM=ABCDE12345
make PRESET=ios-debug
```

Then open `out/build/ios-debug/Rivet.xcodeproj` and run the `Pong` scheme on your device. Without a team, device builds still compile, but they are left unsigned and can't be installed.

### Making a game an iOS app

Games include `Rivet/EntryPoint.hpp` as on desktop. iOS runs the game loop itself, so on iOS the header provides the callbacks that raylib's iOS backend calls, instead of `main()`. In the game's `CMakeLists.txt`, after `add_executable()`:

```cmake
rivet_configure_app(Pong
    BUNDLE_ID "com.tanktek.pong"
    DISPLAY_NAME "Rivet Pong"
)
```

On iOS this makes the executable an app bundle, with an Info.plist, the embedded ANGLE frameworks and code signing, and adds the `run_<target>` target. On desktop it does nothing.
