-- The solution
solution 'm2n'
    configurations { 'Debug', 'Release', 'Debug32', 'Release32', 'Debug64', 'Release64' }
    location       'build'
    language       'C'
    includedirs {
        'src',
        'extlibs/include'
    }

    -- Set default configurations depending on the current OS' architecture
    configmap {
        ['Debug']   = os.is64bit() and 'Debug64' or 'Debug32',
        ['Release'] = os.is64bit() and 'Release64' or 'Release32'
    }

    -- Debug config
    filter { 'configurations:Debug*' }
        targetsuffix '-d'
        defines      { 'DEBUG' }
        flags        { 'ExtraWarnings', 'Symbols' }

    -- Release config
    filter { 'configurations:Release*' }
        kind         'WindowedApp'
        defines      { 'NDEBUG' }
        flags        { 'Optimize' }

    -- MinGW specific
    filter { 'action:gmake', 'system:windows' }
        links        { 'mingw32' }

    -- Linux specific
    filter { 'system:linux' }
        links        { 'pthread' }

    -- Linking dirs
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        libdirs      { 'extlibs/libs-mingw/x86' }
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        libdirs      { 'extlibs/libs-mingw/x64' }
    filter { 'action:vs*', 'system:windows', 'architecture:x32' }
        libdirs      { 'extlibs/libs-vs2013/x86' }
    filter { 'action:vs*', 'system:windows', 'architecture:x64' }
        libdirs      { 'extlibs/libs-vs2013/x64' }

    -- Archituectures
    filter 'configurations:*32'
        architecture 'x32'

    filter 'configurations:*64'
        architecture 'x64'

-- PhysFS
project 'physfs'
    targetname     'physfs'
    language       'C'
    kind           'StaticLib'

    includedirs {
        'extlibs/include/physfs'
    }

    files {
        'extlibs/src/PhysFS/lzma/C/7zCrc.c',
        'extlibs/src/PhysFS/lzma/C/Compress/Lzma/LzmaDecode.c',
        'extlibs/src/PhysFS/*.c',
        'extlibs/src/PhysFS/lzma/C/Archive/7z/*.c',
        'extlibs/src/PhysFS/lzma/C/Compress/Branch/*.c'
    }

    excludes {
        'extlibs/src/PhysFS/lzma/C/Archive/7z/7zMain.c'
    }

    defines {
        'PHYSFS_NO_CDROM_SUPPORT=1',
        'HAVE_GCC_DESTRUCTOR=1',
        'OPT_GENERIC',
        'REAL_IS_FLOAT',
        'PHYSFS_SUPPORTS_ZIP=1',
        'PHYSFS_SUPPORTS_7Z=1',
        'PHYSFS_SUPPORTS_GRP=0',
        'PHYSFS_SUPPORTS_QPAK=0',
        'PHYSFS_SUPPORTS_HOG=0',
        'PHYSFS_SUPPORTS_MVL=0',
        'PHYSFS_SUPPORTS_WAD=0'
    }

    filter { 'action:gmake', 'system:not windows' }
        targetdir 'extlibs/libs'
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-mingw/x86'
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-mingw/x64'
    filter { 'action:vs*', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-vs2013/x86'
    filter { 'action:vs*', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-vs2013/x64'

-- Soloud
project 'soloud'
    targetname        'soloud'
    kind              'StaticLib'
    language          'C++'
    flags             { 'NoPCH' }
    exceptionhandling 'Off'
    -- rtti           'Off'

    includedirs {
        'extlibs/include/soloud',
        'extlibs/src/soloud/',
        'extlibs/src/soloud/audiosource',
        'extlibs/src/soloud/filter',
        'extlibs/src/soloud/core'
    }

    files {
        'extlibs/src/soloud/backend/null/**.c*',
        'extlibs/src/soloud/audiosource/**.c*',
        'extlibs/src/soloud/filter/**.c*',
        'extlibs/src/soloud/core/**.c*'
    }

    defines { 'WITH_NULL' }
    
    filter { 'system:windows' }
        defines { 'WITH_WINMM' }
        files { 'extlibs/src/soloud/backend/winmm/**.c*' }
    filter { 'system:macosx' }
        defines { 'WITH_COREAUDIO' }
        files { 'extlibs/src/soloud/backend/coreaudio/**.c*' }
    filter { 'system:not windows', 'system:not macosx' }
        defines { 'WITH_ALSA', 'WITH_OSS' }
        files {
            'extlibs/src/soloud/backend/alsa/**.c*',
            'extlibs/src/soloud/backend/oss/**.c*'
        }

    filter { 'action:gmake' }
        buildoptions { '-x c++ -std=c++11' }
        linkoptions  { "-static-libgcc -static-libstdc++" }

    filter { 'action:gmake', 'system:not windows' }
        targetdir 'extlibs/libs'
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-mingw/x86'
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-mingw/x64'
    filter { 'action:vs*', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-vs2013/x86'
    filter { 'action:vs*', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-vs2013/x64'

    filter { 'configurations:Release*' }
        flags { 'Optimize', 'OptimizeSpeed', 'NoEditAndContinue', 'No64BitChecks' }
    filter { 'architecture:x32', 'configurations:Release*' }
        flags { 'EnableSSE2' }

    filter { 'action:vs*', 'system:windows' }
        defines { '_CRT_SECURE_NO_WARNINGS' }

    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        defines { 'DISABLE_SIMD' }

-- The Game
project 'game'
    targetname 'game'
    targetdir  'bin'
    language   'C++'

    files {
        'src/**/*.cpp',
        'src/**/*.c',
        'src/*.cpp',
        'src/*.c'
    }

    -- Debug config
    filter { 'configurations:Debug*' }
        kind         'ConsoleApp'

    -- Release config
    filter { 'configurations:Release*' }
        kind         'WindowedApp'

    -- Linux specific
    filter { 'system:linux' }
        links        { 'GL', 'luajit-5.1', 'asound' }
        linkoptions  { "-export-dynamic" }

    -- Windows specific
    filter 'system:windows'
        files        { 'game.rc' }
        links        { 'SDL2main', 'opengl32', 'lua51', 'winmm' }

    -- GCC specific
    filter { 'action:gmake' }
        buildoptions { '-x c++ -std=c++11' }
        linkoptions  { "-static-libgcc -static-libstdc++" }

    -- All configs, again, because of linking order
    filter {}
        links        { 'SDL2', 'physfs', 'freetype', 'soloud' }

    -- Post build commands
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        postbuildcommands { 'postbuild-win mingw/x86'}
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        postbuildcommands { 'postbuild-win mingw/x64'}
    filter { 'action:vs*', 'system:windows', 'architecture:x32' }
        postbuildcommands { 'postbuild-win vs2013/x86'}
    filter { 'action:vs*', 'system:windows', 'architecture:x64' }
        postbuildcommands { 'postbuild-win vs2013/x64'}
