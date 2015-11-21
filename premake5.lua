-- The solution
solution 'm2n'
    configurations { 'Debug', 'Release', 'Debug32', 'Release32', 'Debug64', 'Release64' }
    location       '.'
    language       'C++'
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

    -- GCC specific
    filter { 'action:gmake' }
        buildoptions { '-std=c++11' }
        linkoptions  { "-static-libgcc -static-libstdc++" }

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
    filter { 'action:vs2013', 'system:windows', 'architecture:x32' }
        libdirs      { 'extlibs/libs-vs2013/x86' }
    filter { 'action:vs2013', 'system:windows', 'architecture:x64' }
        libdirs      { 'extlibs/libs-vs2013/x64' }

    -- Archituectures
    filter 'configurations:*32'
        architecture 'x32'

    filter 'configurations:*64'
        architecture 'x64'

-- Soloud
project 'soloud'
    targetname        'soloud'
    kind              'StaticLib'
    flags             { 'NoPCH' }
    exceptionhandling "Off"
    rtti              "Off"

    includedirs {
        'extlibs/include/soloud'
    }

    files {
        'extlibs/src/soloud/backend/sdl2_static/**.c*',
        'extlibs/src/soloud/audiosource/**.c*',
        'extlibs/src/soloud/filter/**.c*',
        'extlibs/src/soloud/core/**.c*'
    }

    defines { 'WITH_SDL2_STATIC' }

    filter { 'action:gmake', 'system:not windows' }
        targetdir 'extlibs/libs'
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-mingw/x86'
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-mingw/x64'
    filter { 'action:vs2013', 'system:windows', 'architecture:x32' }
        targetdir 'extlibs/libs-vs2013/x86'
    filter { 'action:vs2013', 'system:windows', 'architecture:x64' }
        targetdir 'extlibs/libs-vs2013/x64'

    filter { 'configurations:Release*' }
        flags { 'Optimize', 'OptimizeSpeed', 'NoEditAndContinue', 'No64BitChecks' }
    filter { 'architecture:x32', 'configurations:Release*' }
        flags { 'EnableSSE2' }

    filter { 'action:vs2013', 'system:windows' }
        defines { '_CRT_SECURE_NO_WARNINGS' }

    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        buildoptions { --[['-fPIC',]] '-msse4.1' }

-- The project
project 'game'
    targetname     'game'
    targetdir      'bin'

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
        links        { 'GL', 'luajit-5.1' }
        linkoptions  { "-export-dynamic" }

    -- Windows specific
    filter 'system:windows'
        files        { 'game.rc' }
        links        { 'SDL2main', 'opengl32', 'lua51', 'winmm' }

    -- All configs, again, because of linking order
    filter {}
        links        { 'SDL2', 'physfs', 'freetype', 'soloud' }

    -- Post build commands
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        postbuildcommands { 'postbuild-win mingw/x86'}
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        postbuildcommands { 'postbuild-win mingw/x64'}
    filter { 'action:vs2013', 'system:windows', 'architecture:x32' }
        postbuildcommands { 'postbuild-win vs2013/x86'}
    filter { 'action:vs2013', 'system:windows', 'architecture:x64' }
        postbuildcommands { 'postbuild-win vs2013/x64'}
