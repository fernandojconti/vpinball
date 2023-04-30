# Visual Pinball Standalone

*An open source **cross platform** pinball table simulator.*

This project is a modified version of [VPinballX](https://github.com/vpinball/vpinball) that is designed to run on non-Windows platforms.

[![Watch the video](https://img.youtube.com/vi/jK3TbGvTuIA/0.jpg)](https://www.youtube.com/watch?v=xjkgzIVL_QU)

## Background

Visual Pinball was built over 20 years ago using Microsoft technologies such as OLE Compound Documents, DirectX, VBScript, COM, and ATL -- many of which have no cross platform equivalents or support.

To make a successful port, we would need tackle several tasks:

> ***OLE Compound Documents***
> - Visual Pinball Table (`.vpx`) files are stored in [Compound File Binary Format](https://en.wikipedia.org/wiki/Compound_File_Binary_Format).
>
> **Solution**
> - Implement [POLE - portable library for structured storage](https://dimin.net/software/pole/) to load `.vpx` files.
> - Create `PoleStorage` wrapper to match `IStorage` interface.
> - Create `PoleStream` wrapper to match `IStream` interface.

> ***Windows Registry***
> - Visual Pinball stores and retrieves its settings from the Windows registry.
>
> **Solution**
> - Use the `ENABLE_INI` preprocessor definition which uses [RapidXML](https://github.com/timniederhausen/rapidxml) to save and retrieve settings from a file.

> ***VBScript***
> - Visual Pinball uses [VBScript](https://learn.microsoft.com/en-us/previous-versions/t0aew7h6(v=vs.85)) as the scripting engine for tables.
>
> **Solution**
> - Leverage the VBScript engine from [Wine](https://github.com/wine-mirror/wine/tree/master/dlls/vbscript).
> - Fix bugs in Wine's VBScript engine:
>   - [Bug 53644](https://bugs.winehq.org/show_bug.cgi?id=53644) - vbscript can not compile classes with lists of private / public / dim declarations
>   - ~~[Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670) - vbscript can not compile if expressions with reversed gte, lte, (=>, =<)~~
>   - ~~[Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676) - vbscript can not exec_script - invalid number of arguments for Randomize~~
>   - ~~[Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678) - vbscript can not compile CaseClausules that do not use a colon~~
>   - ~~[Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766) - vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds~~
>   - [Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767) - vbscript fails to handle ReDim when variable is not yet created
>   - ~~[Bug 53782](https://bugs.winehq.org/show_bug.cgi?id=53782) - vbscript can not compile ReDim with list of variables~~
>   - ~~[Bug 53783](https://bugs.winehq.org/show_bug.cgi?id=53783) - vbscript can not compile private const expressions~~
>   - ~~[Bug 53807](https://bugs.winehq.org/show_bug.cgi?id=53807) - vbscript fails to redim original array in function when passed byref~~
>   - [Bug 53844](https://bugs.winehq.org/show_bug.cgi?id=53844) - vbscript invoke_vbdisp not handling let property correctly for VT_DISPATCH arguments
>   - ~~[Bug 53866](https://bugs.winehq.org/show_bug.cgi?id=53866) - vbscript fails to handle SAFEARRAY in for...each~~
>   - ~~[Bug 53867](https://bugs.winehq.org/show_bug.cgi?id=53867) - vbscript fails to retrieve property array by index~~
>   - ~~[Bug 53868](https://bugs.winehq.org/show_bug.cgi?id=53868) - vbscript fails to return TypeName for VT_DISPATCH~~
>   - ~~[Bug 53873](https://bugs.winehq.org/show_bug.cgi?id=53873) - vbscript fails to compile Else If when If is on same line~~
>   - [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877) - vbscript compile_assignment assertion when assigning multidimensional array by indices
>   - ~~[Bug 53888](https://bugs.winehq.org/show_bug.cgi?id=53888) - vbscript does not allow Mid on non VT_BSTR~~
>   - [Bug 53889](https://bugs.winehq.org/show_bug.cgi?id=53889) - vbscript does not support Get_Item call on IDispatch objects
>   - [Bug 54221](https://bugs.winehq.org/show_bug.cgi?id=54221) - vbscript: missing support for GetRef
>   - ~~[Bug 54234](https://bugs.winehq.org/show_bug.cgi?id=54234) - vbscript fails to compile when colon follows Else in If...Else~~
>   - [Bug 54291](https://bugs.winehq.org/show_bug.cgi?id=54291) - vbscript stuck in endless for loop when UBound on Empty and On Error Resume Next
>   - ~~[Bug 54456](https://bugs.winehq.org/show_bug.cgi?id=54456) - vbscript memory leak in For Each with SafeArray as group~~
>   - ~~[Bug 54457](https://bugs.winehq.org/show_bug.cgi?id=54457) - vbscript memory leaks in interp_redim_preserve~~
>   - ~~[Bug 54458](https://bugs.winehq.org/show_bug.cgi?id=54458) - vbscript memory leaks in Global_Split~~
>   - ~~[Bug 54489](https://bugs.winehq.org/show_bug.cgi?id=54489) - vbscript Abs on BSTR returns invalid value~~
>   - ~~[Bug 54490](https://bugs.winehq.org/show_bug.cgi?id=54490) - vbscript fails to compile when statement follows ElseIf~~
>   - ~~[Bug 54493](https://bugs.winehq.org/show_bug.cgi?id=54493) - vbscript fails to compile concat when used without space and expression begins with H~~
>   - [Bug 54731](https://bugs.winehq.org/show_bug.cgi?id=54731) - vbscript: stack_pop_bool doesn't support floats or ole color


> - Add support for `Scripting.FileSystemObject` and `Scripting.Dictionary` leveraging Wine's `scrrun` code.
> - Add support for `E_NOTIMPL` commands to Wine's VBScript engine:
>   - `Execute`
>   - `ExecuteGlobal`
>   - `Eval`
>   - `GetRef`

> ***COM / ATL***
> - Visual Pinball uses [COM](https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model) as a bridge between the scripting engine and itself. The scripting engine uses COM to access other components such as [VPinMAME](https://github.com/vpinball/pinmame) and [B2S.Server](https://github.com/vpinball/b2s-backglass).
> - Visual Pinball uses [ATL](https://learn.microsoft.com/en-us/cpp/atl/active-template-library-atl-concepts?view=msvc-170) to dispatch events from itself to the scripting engine, ex: `DISPID_GameEvents_Init` and `DISPID_HitEvents_Hit`
>
> **Solution**
> - Leverage just enough Wine source code to set up the platform to match Windows, ex: `wchar_t` is 2 bytes in Windows while 4 bytes on most other platforms. (`_WINE_UNICODE_NATIVE_`)
> - Provide stubs for as much COM and ATL code as possible by using Wine and [ReactOS](https://github.com/reactos/reactos/tree/master/sdk/lib/atl) source code.
> - Embed Wine's VBScript engine instead of trying to replicate Windows [RPC](https://learn.microsoft.com/en-us/windows/win32/rpc/rpc-start-page).
> - Replace Windows `typelib` magic with class methods for `GetIDsOfNames`, `Invoke`, `FireDispID`, and `GetDocumentation`
> - Create two `idlparser` utilities that read [Interface Definition File](https://learn.microsoft.com/en-us/windows/win32/midl/interface-definition-idl-file) and automatically generates C and C++ code for `GetIDsOfNames`, `Invoke`, `FireDispID`, and `GetDocumentation`
> - Embed `libpinmame` to provide support for PinMAME based games. [Future]

> ***Graphics***
> - Visual Pinball uses either DirectX or OpenGL for graphics.
>
> **Solution**
> - MacOS uses OpenGL 4.1.
> - Linux (Ubuntu 22.04) uses OpenGL 4.6.
> - Android, iOS, tvOS, and Raspberry PI use OpenGLES 3.0.

> ***Sound***
> - Visual Pinball uses DirectSound for playing `.wav` files and [BASS](http://www.un4seen.com/) for playing all other audio files.
>
> **Solution**
> - Use the `ONLY_USE_BASS` preprocessor definition which uses BASS for all audio files including `.wav` files.

> ***Input***
> - Visual Pinball uses XInput and either DirectInput 8 or SDL2 for getting keyboard and joystick info.
>
> **Solution**
> - Make a `SDLK_TO_DIK` translation table that converts SDL Keys to DirectInput Keys.
> - Add additional support for `SDL_KEYUP` and `SDL_KEYDOWN` events.

> ***Updating***
> - The Visual Pinball repository is constantly updated with new features and bug fixes.
>
> **Solution**
> - Updates must be easy to rebase, so all changes must be wrapped in:
>
>   ```
>   #ifdef __STANDALONE__
>   #endif
>   ```
>   or
>   ```
>   #ifndef __STANDALONE__
>   #else
>   #endif
>   ```

## Compiling

### MacOS (arm64)

In a terminal execute the following:
```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/macos-arm64
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-macos-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

*Note:* Make sure `SDL2`, `SDL2_ttf`, and `freeimage` are not installed by `brew`, otherwise you may have compile issues

### MacOS (x64)

In a terminal execute the following:
```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/macos-x64
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-macos-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

*Note:* Make sure `SDL2`, `SDL2_ttf`, and `freeimage` are not installed by `brew`, otherwise you may have compile issues

### iOS (arm64)

In a terminal execute the following:
```
brew install cmake bison curl ios-deploy fastlane
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/ios
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-ios-arm64.txt CMakeLists.txt
cmake -G Xcode -B XCode
open XCode/vpinball.xcodeproj
```

### tvOS

In a terminal execute the following:
```
brew install cmake bison curl ios-deploy fastlane
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/tvos
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-tvos-arm64.txt CMakeLists.txt
cmake -G Xcode -B XCode
open XCode/vpinball.xcodeproj
```

*Note:* BASS for tvOS is not publicly available for open source projects. You will need to buy an iOS license. More information can be found [here](https://www.un4seen.com/).

### Android (arm64-v8a)

```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
export JAVA_HOME=$(/usr/libexec/java_home -v 11.0.16.1)
export ANDROID_HOME=/Users/jmillard/Library/Android/sdk
export ANDROID_NDK=/Users/jmillard/Library/Android/sdk/ndk/25.1.8937393
export ANDROID_NDK_HOME=/Users/jmillard/Library/Android/sdk/ndk/25.1.8937393
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/android
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-android-arm64-v8a.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
cd standalone/android/android-project
./gradlew installDebug
$ANDROID_HOME/platform-tools/adb shell am start -n org.vpinball.app/org.vpinball.app.VPinballActivity
$ANDROID_HOME/platform-tools/adb -d logcat org.vpinball.app
```

### Linux (Ubuntu 22.04)

In a terminal execute the following:
```
sudo apt install git build-essential cmake bison curl
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/linux
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Linux (Fedora 37)

In a terminal execute the following:
```
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc-c++ cmake bison curl
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/linux
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### RK3588 (Armbian)

Start with a [Armbian 23.02 Jammy LI](https://www.armbian.com/orangepi-5/) image and execute the following:

In a terminal execute the following:
```
sudo add-apt-repository ppa:liujianfeng1994/panfork-mesa
sudo add-apt-repository ppa:liujianfeng1994/rockchip-multimedia
sudo apt update
sudo apt dist-upgrade
sudo apt install mali-g610-firmware rockchip-multimedia-config
sudo apt-get install git pkg-config cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libgles2-mesa libudev-dev
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/rk3588
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-rk3588-aarch64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Raspberry Pi 4

Start with a [Raspberry Pi OS Lite (64-Bit)](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit) image and execute the following:
```
sudo apt-get install git pkg-config cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libgles2-mesa libudev-dev
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/rpi
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-rpi-aarch64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

## Running

Go to the build directory:
```
cd build
```

To launch a table, execute the following:
```
./VPinballX_GL -play <table.vpx>
```

To launch a table in windowed mode, execute the following:
```
./VPinballX_GL -DisableTrueFullscreen -play <table.vpx>
```

To extract a table script, execute the following:
```
./VPinballX_GL -extractvbs <table.vpx>
```

To list all available fullscreen resolutions, execute the following: 
```
./VPinballX_GL -listres
```

To list all available audio devices, execute the following:
```
./VPinballX_GL -listsnd
```

## Joystick

The joystick is currently mapped to an XBox controller layout.

- Left Flipper: `Left Shoulder`
- Right Flipper: `Right Shoulder`
- Center Tilt: `D-pad Up`
- Left Title: `D-pad Left`
- Right Tilt: `D-pad Right`
- Plunger: `D-pad Down`
- Add Credit: `A`
- Start: `B`
- FPS: `X`
- Cancel: `Y`
- Fire: `Left Stick`

## Debugging

Debugging can be done using [Visual Studio Code](https://code.visualstudio.com/).

### MacOS

Perform the steps outlined above in *Compiling* and *Running*.

In Visual Studio Code:
  - Install the [`C/C++ Extension Pack`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension.
  - Open the `vpinball` folder.
  - If prompted, select the latest version of clang, ex: `Clang 14.0.0 arm64-apple-darwin21.6.0`
  - Go to `Settings` -> `CMake: Debug Config` and click `Edit in settings.json`
  - Update `settings.json` with:
  ```
      "cmake.debugConfig": {
         "args": [ "-DisableTrueFullscreen", "-Play", "res/exampleTable.vpx" ],
      }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar

### Raspberry Pi 4

Perform the steps outlined above in *Compiling* and *Running*.

Since we are using the Lite version of Raspberry Pi OS, there is no X11 to run the full version Visual Studio Code. Instead will we use the [Remote Development](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) extension in Visual Studio Code running on a host computer.

**NOTE**: The 2GB version of the Raspberry Pi 4 can easily run out of memory and become unresponsive.

Make sure SSH is configured and enabled on your Raspberry Pi.

On the host computer, in Visual Studio Code:
  - Install the [`Remote Development`](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) extension.
  - Select `Remote-SSH: Connect to Host...` from the Command Palette (F1, ⇧⌘P)
  - Enter the `user@address` of the Raspberry Pi.
  - Once connected, install the [`C/C++ Extension Pack`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension.
  - If prompted, select the latest version of gcc, ex: `GCC 10.2.1 aarch64-linux-gnu`
  - Go to `Settings` (`Remote SSH`) -> `CMake: Debug Config` and click `Edit in settings.json`
  - Update `settings.json` with:
  ```
    "cmake.parallelJobs": 1,
    "cmake.generator": "Unix Makefiles",
    "cmake.debugConfig": {
        "args": [ "-Play", "res/exampleTable.vpx" ],
    }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar

### Change Log

To keep up with all the changes in master, and make it easier to rebase, this branch is a single commit -- until most tables work with no vbs changes.

The downside of this approach is not accurately keeping track of history:

* 04/30/23
  * Switch `VPinMAMEController` to always use RAW mode
  * Allow DMD color to be set via ini file in `.pinmame/vpinball`

* 04/29/23
  * Updated `libpinmame` to use new user data pointer
  * Added ZeDMD support for PinMAME games

* 04/27/23
  * Added ZeDMD support
  * Implement RenderMode in FlexDMD

* 04/25/23
  * Finally fixed ImGui flickering issue in OpenGLES (Renderdoc / @Niwak)
  * Added FlexDMD asset folder to iOS and tvOS builds
  * Re-enable AO for OpenGLES builds and fix `GREY8` -> `GL_R8` (@Niwak)
  * Remove `System.Drawing.Common` stubs

* 04/23/23
  * Fix several FlexDMD label and image rendering issues

* 04/20/23
  * Start to implement FlexDMD labels
  * Fix Linux builds (@pablo99)

* 04/19/23
  * Add `--flexdmddir` option to `vpxm`
  * Move `vpinball.log` to `m_szMyPrefPath`
  * Fix FlexDMD images from not showing

* 04/17/23
  * Add `SDL_Image` for `FlexDMD` GIF support

* 04/14/23
  * More work on implementing FlexDMD
  * Update `DTArray` and `STArray` workarounds to use `DropTarget` and `StandupTarget` classes
  * Added `Working Tables` section

* 04/08/23
  * Fix `FormatNumber` to support VT_ERROR when argument is omitted
  * Stub more FlexDMD functions

* 04/06/23
  * Replace `create_directories` with `create_directory`

* 04/05/23
  * Move `VPinballX.ini` to `m_szMyPrefPath`
  * Revert `m_szMyPath` and `m_wzMyPath` not including trailing path separator
  * Fixed `get_SolMask` and `put_SolMask` to use new PWM support in PinMAME 3.6
  * Bump `SDL2` to `2.26.5`

* 04/04/23
  * Remove automatically including user directory in cmake files
  * Generate user folder on first run to fix iOS crash (fixes Cue Ball Wizard)
  * Update `m_szMyPath` and `m_wzMyPath` to not include trailing path separator (for consistency)

* 04/03/23
  * Inject live table with a flasher based DMD (`ImplicitDMD2`)
  * Inject live table with a convience timer (`ImplicitTimer`)
  * Bump `libserum` to `1.4.0`
  * Update to support fixed `BRIGHTNESS` vs `RAW` libpinmame DMD mode
  * Add buttons in Live UI popup menu to switch to Desktop / Cabinet / and FSS modes

* 03/26/23
  * Wine hack to add more types in `stack_pop_bool` (fixes SS, Big Indian, and Whirlwind)

* 03/24/23
  * Added rk3588 build
  * Split up CI to build SBC versions separately
  * Cleaned up instructions

* 03/23/23
  * Fix `core.vbs` patch to work in Windows builds
  * Added support for `Sound` and `SoundBG` in `VPinballX.ini`
  * Added rk3588 (Orange Pi 5) build instructions

* 03/22/23
  * Bump SDL to 2.26.4 and apply [keyboard fix](https://github.com/libsdl-org/SDL/commit/54ca4d387954e687db0d28758d43cf08a1cc1353) for MacOS builds
  * Bump SDL_ttf to 2.20.2 and remove MacOS harfbuzz patch

* 03/20/23
  * Fixed multiple items being added / overwritten to live table
  * Force implicit DMD to have name `ImplicitDMD`
  * Bump `libserum` to 1.4.0
  * Fixed table exit crash when `CodeViewer` is destroyed in Linux

* 03/19/23
  * Fixed `PoleStorage` to return correct `HRESULT` when file not found

* 03/17/23
  * Update Android to properly calculate DPI for Live UI
  * Fixed iOS Launch Screen issues in `CMakeLists.txt` and `Info.plist`
  * Fixed `implicitDMD` not being added to script engine
  * Added MacOS x64 builds
  * Added `m_serumColorized` flag to prevent rotations before colorize

* 03/16/23
  * Update `PlayMusic` to replace backslash with forward slash
  * Force quit on vbscript errors

* 03/15/23
  * Added support for rotations in `libserum`.
  * Added `-listsnd` command line option to dump out available sound devices

* 03/14/23
  * Added iOS launch storyboard to allow for fullscreen

* 03/13/23
  * More work on shutdown with updated LiveUI
  * Work on build on rk3588 (Orange Pi 5, Armbian)

* 03/11/23
  * Updated `vpxm` script to support NVRAM files
  * Updated to use new libpinmame `PinmameIsPaused`
  * Updated to properly quit on iOS, tvOS, and Android

* 03/10/23
  * Fixed converting `VARIANT_BOOL` to `BSTR` conversion crash on Linux
  * Remove `Debugger` in Live UI
  * Replace `Quit to Editor` with `Quit` in Live UI
  * Implement Pause and Resume
  * Add touch support for iOS and Android

* 03/09/23
  * Add support for [libserum](https://github.com/zesinger/libserum)
  * Bump SDL2 to 2.26.4 

* 03/06/23
  * Work on properly shutting down table so `libpinmame` can write nvram files
  * Stub `ShowCursor` to fix compile errors
  * Hack Wine vbscript `clean_props` and `release_script` to prevent seg fault on table exit
 
* 03/05/23
  * Added support for `get_RawDmdColoredPixels`
  * Check pointers in `Decal` and `Textbox` destructors

* 03/01/23
  * Redesigned `PoleStorage`/`PoleStream` again to support multiple threads
  * Fixed `m_logicalNumberOfProcessors` support
  * Added `external_open_storage` method and moved `StgOpenStorage` to `wine.c`

* 02/27/23
  * Added `HighDPI` configuration option. ie: `<HighDPI>0</HighDPI>` disables on macos and iOS
  * Added `-listres` command line option to dump out available fullscreen resolutions
  * Fixed fullscreen support for systems with multiple displays
  * Use intensity values (`0-255`) in `get_ChangedLamps` only for SAM games

* 02/24/23
  * Fix crash when `exception.bstrDescription` is NULL
  * Fix `LocalStringW` and `GetUniqueName` not initializing with 0
  * Add an implicit DMD for tables that do not have one, ie `TextBox00`

* 02/24/23
  * Additional code cleanup
  * Replace `wine_stubs.c` with `wine.c`
  * Rework external logging code
  * Move cmake files into `standalone/cmake`

* 02/23/23
  * Work to get macos compiling with GCC instead of clang
  * More cleanup to `wine_stubs.c`

* 02/22/23
  * Migrate `main_standalone.h` into `main.h`

* 02/21/23
  * Stub more FlexDMD (Galaga now starts up)
  * Refactor `PoleStorage` and `PoleStream` to more closely match `IStream` IDL

* 02/20/23
  * Start to cleanup `main_standalone.h`
  * Move `PoleStorage` and `PoleStream` to separate class files
  * Use `dinput.h` from Wine
  * Move ATL support files to `standalone/inc/atl`
  * Replace `_stricmp` with `lstrcmpi` for consistency

* 02/19/23
  * Add support for pov
  * Stub out some more of FlexDMD
  * Stub out `_Bitmap` from `System.Drawing.Common`

* 02/18/23
  * Add `get_DIP`, `put_DIP`, `get_Lamp`, and `get_Solenoid` to `VPinMAMEController.` (Addams Family now works)

* 02/17/23
  * Rework iOS and tvOS builds to generate ipa files
  * Add output name switch to `vpxm`

* 02/16/23
  * Rework `IDLParserToC` generator to produce easier to read code
  * Optimize `IDictionary` and `IFileSystem3` proxies
  * Fix Linux build

* 02/15/23
  * Rework `IDLParserToCpp` generator to produce easier to read code
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2214) fix compile issue with non hex after concat without space (Fixes Beavis and Butthead by @chugalaefoo)

* 02/14/23
  * Bump Wine to 8.1 [23c10c92](https://gitlab.winehq.org/wine/wine/-/tree/23c10c928b68918515b6ec195d90b09ef5936451)
  * Rework Wine changes to be as minimal as possible
  * Fix missing `Item` function for `Scripting.Dictionary` (Fixes Beavis and Butthead by @chugalaefoo)
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2175) fix VarAbs function for BSTR with positive values (Fixes Beavis and Butthead by @chugalaefoo)
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2188) Fix compile when statement after ElseIf or after separator (Fixed AC/DC compile issue)
  * Move `idl.sh` to `scripts/widlgen`
  * Update mobile and linux builds to use SDL 2.26.3

* 02/09/23
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2141) Fix memory leak in owned safearray iterator
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2142) Fix compile when colon follows Else on new line

* 02/08/23
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2132) Fix memory leak in interp_redim_preserve
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/2131) Fix memory leak in Split()

* 02/07/23
  * Wine hack to fix memory leak with "owned" `SAFEARRAY` in `interp_newenum` and `interp_enumnext`
  * Add `EXTERNAL_DIR` and `APK` settings to `vpxm` script

* 02/06/23
  * Wine hack to fix memory leak in `interp_redim_preserve`

* 02/04/23
  * Fix several memory leaks (`GetRef` and `GlobalSplit`)
  * Fix issue where `CComVariant` would release additional reference after `Detach` in `Close`
  * Added an `AddRef` to `ITypeInfoImpl` which finally fixes deleting objects when ref count = 0!
  * Fixed Linux buffer overflows in `Global_FormatNumber`

* 01/31/23
  * Add `vpxm` script and move `vpx` script to `standalone/scripts`

* 01/29/23
  * Rework IDL generator to use binary search trees for `GetIDsOfNames` and `FireDispID`

* 01/28/23
  * Stub more of `IPinUpDisplay` interface

* 01/27/23
  * Start to implement `IPinUpDisplay` interface
  * Reorganize standalone includes from `/inc` to `/standalone/inc`

* 01/26/23
  * Wine hack to support `typename` for class objects. (Fixes lamps in BM)
  * Update Android to use SDL 2.26.2

* 01/25/23
  * More FlexDMD work (BM is alive!)
  * Replace `VariantCopy` with `VariantCopyInd` in proxies. (Fixes BM `swLeft/RightOrbTrigger2.uservalue` issues)
  * Wine hack to implement missing `GetLocale` and `SetLocale` functions
  * Tweak Game Controller support for iOS
  * Update iOS to use SDL 2.26.2

* 01/24/23
  * Add script to generate code from IDL files using WIDL
  * Rearrange some directories and clean up VPinMAME and WMP 
  * Start work on implementing FlexDMD

* 01/21/23
  * Finished `WMPCore`, `WMPSettings`, and `WMPControls` (BM has working music)

* 01/19/23
  * More work on `WMPCore`, `WMPSettings`, and `WMPControls` interfaces

* 01/18/23
  * Skip detecting Siri Remote as `SDL_GameController` in tvOS
  * Set `MaxTexDimension` to `2048` and disable HIGHDPI for tvOS (Fixes AC/DC)
  * Disable Wine `TRACE` and `DEBUG` messages
  * More work on `WMPCore` interfaces

* 01/17/23
  * Wine `ElseIf` hack to not require `NL` after `Then` (Fixed AC/DC)
  * Start to implement `WMPCore`, `WMPControls`, and `WMPSettings`
  * Switch `SDL_Joystick` to `SDL_GameController`
  * Implement `SDL_GameController` rumble support
  * Remapped `Start` and `Cancel` for iOS and tvOS

* 01/16/23
  * Fix `RtlCompareUnicodeStrings` to properly compare wide strings with specified lengths
  * Wine `invoke_variant_prop` hack to support getting array variables in classes, ie: `If Lampz.IsLight(5) = true`
  * Wine `assign_value_script_ctx` hack to support setting array variables in classes, ie: `Lampz.IsLight(5) = true`
  * Implement `tvOS` version
  * Update joystick to match XBox controller layout

* 01/13/23
  * Wine `Global_CreateObject` hack to return `Nothing` instead of error when `CreateObject` fails. (Most scripts check for `Nothing`)
  * Cache `BASS_ATTRIB_FREQ`. (Partially fixes [#224](https://github.com/vpinball/vpinball/issues/224)) 
  * Attempt to fix BASS looping and restarts. (Partially fixes [#224](https://github.com/vpinball/vpinball/issues/224)) 
  * Cleanup `vpx` shell script. (Add to your path to run `VPinballX_GL` from anywhere)
  * Created an IDL enum parser to add enums to `ScriptGlobalTable`, ie: `SequencerState` (support VPin Workshop tables)
  * Updated Basic shader to fix reflections on OpenGLES (Thanks @Niwak!)
  * Fixed `get_ChangedLamps` to return modern intensity-levels for `lampNo > 80` (Fixes GI in TWD!)

* 01/12/23
  * Wine `do_icall` hack to support `GetRef` variable calls with no params, ie: `MotorCallback` (Fixes flippers in TWD)
  * Wine `is_matching_key` hack to support `VT_VARIANT|VT_BYREF`

* 01/11/23
  * Wine scrrun `dictionary_get_HashVal` hack to return hash value for `VT_VARIANT|VT_BYREF` (TWD now plays!)
  * Wine vbscript `UBound`, and `LBound` hack to return `0` when `VT_EMPTY` (MM now plays with zero script changes!)
  * Added additional logging for `Eval`, `Execute`, `ExecuteGlobal`

* 01/10/23
  * Redesign `GetRef` again, this time as IDispatch (T2 now plays with zero script changes!)
  * Added delay to make sure PinMAME is running before trying to initialize switches. (CFTBL now runs with zero script changes!)
  * Reworked wine debug logging to use `plog`

* 01/09/23
  * Implement `VPinMAMEController` mechs

* 01/08/23
  * More work on `IGames`, `IGame`, and `IGameSettings` (Rock now works with zero script changes!)

* 01/07/23
  * Stub VPinMAME interfaces (ie, `IRom`, `IRoms`)
  * Add new `libpinmame` methods for setting `SolMask` and `GetLEDs` (Rock renders Alpha Numeric display)
   
* 01/03/23
  * Fix RPI4 `libsdl2-2.0.so.0` library `SONAME`

* 01/02/23
  * Remove `RPATH` and replace with `$ORIGIN` for Linux/RPI builds
  * Replace `RGB_FP16` with `RGBA_FP16` for OpenGLES builds (RPI4 finally renders!)

* 01/01/23
  * Happy New Year!
  * Work on RPI4 builds

* 12/31/22
  * Fixed startup issues when calling `PinmameGetChangedGIs`/`PinmameGetChangedLamps` before PinMAME is fully running
  * Fixed `VPinMAMEController::get_Version` `BSTR` allocation
  * Fixed non-defaulted `m_capExtDMD` which lead to crashes or incorrect DMD Shader assignment

* 12/30/22
  * Work on Linux builds

* 12/29/22
  * Fix `get_ChangedLamps` to support libpinmame `255` state 
  * Wine vbscript hack to allow statement separators after `ELSE` (fixes Cuphead)
  * Replaced `SDL_Log` with `plog` (`plog` moved to root directory)
  * Updated Android builds to recursively copy assets (ie to copy `pinmame/roms`)

* 12/28/22
  * Rework `GetRef` to fix `vpmCreateBall`
  * Convert `mQue` in `core.vbs` to multiple single dimension arrays
  * Updated `IDLParserToCpp` to use `VariantCopyInd` for `VARIANT` params
  * Update `Eval`, `Execute`, and `ExecuteGlobal` to convert `arg` to `BSTR` using `to_string`
  * More work on `VinMAMEController` (Rock/T2 partially working)

* 12/22/22
  * Rework macos builds to use external.sh and build SDL2, SDL2_ttf, FreeImage, and libpinmame
  * Fix music directory to be cross platform

* 12/21/22
  * Start to implement VPinMAMEController

* 12/20/22
  * Attempt to fix HIGHDPI rendering issues with decals and lights
  * Add shim to allow `controller.vbs` and `Controller.vbs` in `GetTextFile` for case sensitive filesystems
  * Update `check_script_collisions` to allow multiple `Dim`s of `B2SOn` and `Controller` in `ExecuteGlobal` (Fixes Volley)

* 12/17/22
  * Reworked `GetMyPath` to use `SDL_GetBasePath()`
  * Fixed repeating `SDL_KEYUP` and `SDL_KEYDOWN` events in Android build
  * Updated CI to make Android builds

* 12/15/22
  * Fixed several GLES 3.0 rendering issues
  * Added support for retina displays on MacOS and iOS via `SDL_WINDOW_ALLOW_HIGHDPI`

* 12/12/22
  * Added `android-project`
  * Cleaned up Android build instructions
  * Replaced `printf` and `std::cout` with `SDL_Log`

* 12/08/22
  * Started working on Android native library builds

* 12/03/22
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1561) Accept private and public const global declarations.
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1611) Handle "case" statements without separators.
  * Rework Wine debug stubs to fix strdup memory leak 
  * Update shader #defines to support OpenGLES compilation 
  * Replace shader integers with floats to support OpenGLES compilation 
  
* 11/27/22
  * OpenGLES updates to allow iOS app to run - working imgui, sound, input, and mouse support.

* 11/24/22
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1534) Handle another variant of LTE/GTE tokens.

* 11/23/22
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1495) Add support for redim byref. 
  * Add support for iOS simulator build (arm64 only)
  * Skip rendering when OpenGLES so app does not crash (plays, but no graphics)

* 11/20/22
  * `#ifndef __OPENGLES__` around code not available in OpenGLES
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1409) Handle index read access to array properties
  * More work on iOS build

* 11/16/22
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1385) Wine vbscript `Else` new line fix
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1368) Use CRT allocation functions
  * Start to work on iOS build
  * Partial Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1391) Implement Format functions.
  * Rearrange CMake search paths and fix SDL2 includes for Linux
  * Move RapidXML to separate folder

* 11/14/22
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1347) Wine vbscript `Global_TypeName` fix
  * Update to glad 2
  * Add Linux x64 build to github workflow

* 11/12/22
  * Wine scrrun hack to allow dictionary hash values for `VT_VARIANT|VT_BYREF`.
  * Remove vbscript hack to allow `VT_SAFEARRAY` and fix idl parser to use `VT_VARIANT|VT_ARRAY`.
  * Remove vbscript `interp_newenum` hack support `for each...next` with `SAFEARRAY`
  * Rearrange `main_standalone.h` to use `GetDocumentation` in `InternalAddRef` and `InternalRelease`
  * Temporarily disable delete in `Release` until tracking down extra `InternalRelease`
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1328) Wine vbscript `Redim` lists fix
  * Implement [offical](https://gitlab.winehq.org/wine/wine/-/merge_requests/1288) Wine vbscript `Global_Mid` fix
  * Implement [offical](https://bugs.winehq.org/attachment.cgi?id=73461&action=diff) Wine vbscript `Global_TypeName` patch
  * Remove saving width and height in `VPinballX.ini` when exiting table

* 11/07/22
  * Wine vbscript hack to allow `Redim` lists
  * Wine vbscript `Global_FormatNumber` hack add `FormatNumber` support
  * Wine vbscript `Global_Mid` hack to allow `Mid` on non VT_BSTR
  * Wine vbscript `do_icall` hack to support `Get_Item` in `Collection` (Fixes Road Race)

* 11/04/22
  * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1246) Wine vbscript `Global_Rnd` and `Global_Randomize` fixes 
  * Wine vbscript `Global_Eval` hack to fix `IsEmpty(Eval("BallSize"))`
  * Wine vbscript `Global_GetRef` hack to workaround `Set GICallback = GetRef("NullSub")`
  * Wine vbscript `parser.y` hack to allow `Private Const`
  * Wine vbscript `parser.y` hack to allow `Else If` on same line (Fixes Four Million B.C.)
  * Wine vbscript `parser.y` hack to allow `Case Else` without colon
  * Trim whitespace in `VPinballX.ini` and save player options when exiting table

* 11/02/22
  * First working non-example table with no vbscript errors (Grand Tour)
  * Wine vbscript `Global_TypeName` hack to fix `if TypeName(balls(x)) = "IBall" then` 
  * Wine vbscript `invoke_variant_prop` hack to support array access `aObj.ModIn(x)`
  * Wine vbscript `interp_newenum` hack to support `for each...next` with `SAFEARRAY`
  * Implement `GetIDsOfNames` and `Invoke` for `Scripting.Dictionary` in Wine `scrrun`
  * Implement `GetIDsOfNames` and `Invoke` for `Scripting.FileSystemObject` in Wine `scrrun`
  * Implement stubs for Wine `ReadFile`, `WriteFile`, `GetFullPathNameW`, `GetFileAttributesW`, `CreateFileW` so tables can read and write data
  * Rework `Invoke` for voinball interfaces to not use `CComVariant` wrappers
  * Several updates to `IDLParserToC` and `IDLParserToCpp` including adding `GetDocumentation` to support `TypeName`
  * Do not define `_rtol` in Linux

## Workarounds

### DTArray (Drop Targets)

Wine's vbscript engine cannot handle multi-dimension array assignments. For example:

```
DTArray(i)(4) = DTAnimate(DTArray(i)(0),DTArray(i)(1),DTArray(i)(2),DTArray(i)(3),DTArray(i)(4))
```

1) Switch to use the `DropTarget` class, by adding the following code:

```
Class DropTarget
  Private m_primary, m_secondary, m_prim, m_sw, m_animate

  Public Property Get Primary(): Set Primary = m_primary: End Property
  Public Property Let Primary(primary): Set m_primary = primary: End Property

  Public Property Get Secondary(): Set Secondary = m_secondary: End Property
  Public Property Let Secondary(secondary): Set m_secondary = secondary: End Property

  Public Property Get Prim(): Set Prim = m_prim: End Property
  Public Property Let Prim(prim): Set m_prim = prim: End Property

  Public Property Get Sw(): Sw = m_sw: End Property
  Public Property Let Sw(sw): m_sw = sw: End Property

  Public Property Get Animate(): Animate = m_animate: End Property
  Public Property Let Animate(animate): m_animate = animate: End Property

  Public default Function init(primary, secondary, prim, sw, animate)
    Set m_primary = primary
    Set m_secondary = secondary
    Set m_prim = prim
    m_sw = sw
    m_animate = animate

    Set Init = Me
  End Function
End Class
```

2) Update the DT definitions to use `DropTarget` instead of `Array`:

```
DT7 = Array(dt1, dt1a, pdt1, 7, 0)
DT27 = Array(dt2, dt2a, pdt2, 27, 0)
DT37 = Array(dt3, dt3a, pdt3, 37, 0)
```

becomes:

```
Set DT7 = (new DropTarget)(dt1, dt1a, pdt1, 7, 0)
Set DT27 = (new DropTarget)(dt2, dt2a, pdt2, 27, 0)
Set DT37 = (new DropTarget)(dt3, dt3a, pdt3, 37, 0)
```

3) Search and replace:

| From | To | Vi |
| --- | --- | --- |
| `DTArray(i)(0)` | `DTArray(i).primary` | `:%s/DTArray(i)(0)/DTArray(i).primary/g` |
| `DTArray(i)(1)` | `DTArray(i).secondary` | `:%s/DTArray(i)(1)/DTArray(i).secondary/g` |
| `DTArray(i)(2)` | `DTArray(i).prim` | `:%s/DTArray(i)(2)/DTArray(i).prim/g` |
| `DTArray(i)(3)` | `DTArray(i).sw` | `:%s/DTArray(i)(3)/DTArray(i).sw/g` |
| `DTArray(i)(4)` | `DTArray(i).animate` | `:%s/DTArray(i)(4)/DTArray(i).animate/g` |

### STArray (Standup Targets)

Wine's vbscript engine cannot handle multi-dimension array assignments. For example:

```
STArray(i)(3) = STCheckHit(Activeball,STArray(i)(0))
```

1) Switch to use the `StandupTarget` class, by adding the following code:

```
Class StandupTarget
  Private m_primary, m_prim, m_sw, m_animate

  Public Property Get Primary(): Set Primary = m_primary: End Property
  Public Property Let Primary(primary): Set m_primary = primary: End Property

  Public Property Get Prim(): Set Prim = m_prim: End Property
  Public Property Let Prim(prim): Set m_prim = prim: End Property

  Public Property Get Sw(): Sw = m_sw: End Property
  Public Property Let Sw(sw): m_sw = sw: End Property

  Public Property Get Animate(): Animate = m_animate: End Property
  Public Property Let Animate(animate): m_animate = animate: End Property

  Public default Function init(primary, prim, sw, animate)
    Set m_primary = primary
    Set m_prim = prim
    m_sw = sw
    m_animate = animate

    Set Init = Me
  End Function
End Class
```

2) Update the ST definitions to use `StandupTarget` instead of `Array`:

```
ST41 = Array(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
ST42 = Array(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
ST43 = Array(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)
```

becomes:

```
Set ST41 = (new StandupTarget)(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
Set ST42 = (new StandupTarget)(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
Set ST43 = (new StandupTarget)(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)
```

3) Search and replace:

| From | To | Vi |
| --- | --- | --- |
| `STArray(i)(0)` | `STArray(i).primary` | `:%s/STArray(i)(0)/STArray(i).primary/g` |
| `STArray(i)(1)` | `STArray(i).prim` | `:%s/STArray(i)(1)/STArray(i).prim/g` |
| `STArray(i)(2)` | `STArray(i).sw` | `:%s/STArray(i)(2)/STArray(i).sw/g` |
| `STArray(i)(3)` | `STArray(i).animate` | `:%s/STArray(i)(3)/STArray(i).animate/g` |

### BSize and BMass Constants

Some scripts define `BSize` and `BMass` as `Const` and `core.vbs` re-defines as `Dim`. If this causes errors:

1) Replace the following:

```
Const BSize = 25
Const BMass = 1.7
```

with

```
Dim BSize : BSize = 25
Dim BMass : BMass = 1.7
```

or convert to use:

```
Const BallSize = 50
Const BallMass = 1
```

## Miscellaneous

To build header file for VPinMAME.idl: (Thanks to @bshanks and @gcenx83)

```
brew install llvm 
git clone git://source.winehq.org/git/wine.git
git clone git@github.com:vpinball/pinmame.git
cd wine
export PATH=/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin
#export PATH="$(brew --prefix llvm)/bin:$PATH"
#export PATH="$(brew --prefix bison)/bin:$PATH"
./configure --without-freetype
make -j10
tools/widl/widl -o ../vpinmame_i.h --nostdinc -Ldlls/\* -Iinclude -D__WINESRC__ -D_UCRT ../pinmame/src/win32com/VPinMAME.idl
```

## Working Tables

Here are some tables that run with no modifications needed:

| Table | Notes |
| --- | --- |
| `Attack from Mars LE v4.vpx` | |
| `Attack from Mars v4.vpx` | |
| `Beavis And Butthead (cHuG_Original_1.2).vpx` | |
| `Big Bang Bar (Capcom 1996) VPW v1.0.vpx` | |
| `Big Indian (Gottlieb 1974) v4.vpx` | |
| `Black Knight (Williams 1980) v4.vpx` | |
| `Blood Machines 2.0.vpx` | |
| `Boomerang (Bally 1974).vpx` | |
| `Capersville (Bally 1966).vpx` | |
| `Creature From The Black Lagoon (Bally 1992) 1.2.vpx` | |
| `Cuphead Pro (D. Goblett & Co 2020).vpx` | |
| `Flicker (Bally 1974).vpx` | |
| `Four Million B.C. (Bally 1971).vpx` | |
| `Funhouse (Williams 1990)2.0b.vpx` | |
| `Grand Tour (Bally 1964).vpx` | |
| `JP's Captain Fantastic (Bally 1975) v4.vpx` | |
| `Mariner (Bally 1971).vpx` | |
| `Nudgy (Bally 1947).vpx` | |
| `OXO (Williams 1973).vpx` | |
| `Pirate Gold (Chicago Coin 1969).vpx` | |
| `QBerts Quest (Gottlieb 1983).vpx` | |
| `Red and Ted's Road Show (Williams 1994) VPW v1.3.6.vpx` | |
| `Rock (Gottlieb 1985) v4.vpx` | |
| `Sea Ray (Bally 1971).vpx` | |
| `Star_Trek_The_Next_Generation_Williams_1993_VPW_Mod_v1.0.vpx` | |
| `TOTAN4k 1.504.vpx` | |
| `Terminator 2 (Williams 1991) g5k_002.vpx` | |
| `The Addams Family (Bally1992) v2.3.2.vpx` | |
| `The Wiggler (Bally 1967).vpx` | |
| `Triple Strike (Williams 1975) v5.0.1.vpx` | |
| `Triple X (Williams 1973).vpx` | |
| `Tropic Fun (Williams 1973).vpx` | |
| `Volley (Gottlieb 1976).vpx` | |
| `Whirlwind 4K 1.1.vpx` | |

## Shoutouts

Wine and the amazing people who support the VBScript engine
- Robert Wilhelm
- Nikolay Sivov
- Jacek Caban  

The people who make this such an exciting hobby to be a part of
- @Bord, @Cupiii, @ecurtz, @freezy, @Iaaki, @Lucidish, @mkalkbrenner, @Niwak, @Scottacus64, @Thalamus, @toxie, @Zedrummer
- and the rest of the Visual Pinball community!
