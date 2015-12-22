# m2n
A small game (and game engine)

## Building

Requires premake5 in path.

### MinGW and GCC
On linux requires developement libraries for `luaJIT`, `sdl2`, `freetype2` and `asound` (ALSO/OSS)

    premake5 gmake
    make config=release -C build

### Visual studio
    
    premake5 vs2013

or

    premake5 vs2015

Generated project files will be available in the `build` directory

### Android
Create a local.properties file with the following content:

    sdk.dir=/path/to/your/android/sdk
    ndk.dir=/path/to/your/android/ndk

Compile and install debug with

    ./gradlew assembleDebug installDebug

Requires Android NDK and SDK Build Tools revision 22.0.1