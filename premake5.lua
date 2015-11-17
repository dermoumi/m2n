-- Helper function
local source_dirs = {
    'src/**/*.cpp',
    'src/**/*.c',
    'src/*.cpp',
    'src/*.c'
}

-- The global files to be compiled
local include_dirs = {
    'src',
    'extlibs/include'
}

-- The solution
solution 'm2n'
    configurations { 'Debug', 'Release', 'Debug32', 'Release32', 'Debug64', 'Release64' }
    location       '.'
    language       'C++'
    targetdir      'bin'
    includedirs    ( include_dirs )

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

    -- Linking dirs and post build commands
    filter { 'action:gmake', 'system:windows', 'architecture:x32' }
        libdirs      { 'extlibs/libs-mingw/x86' }
        postbuildcommands { 'postbuild-win mingw/x86'}
    filter { 'action:gmake', 'system:windows', 'architecture:x64' }
        libdirs      { 'extlibs/libs-mingw/x64' }
        postbuildcommands { 'postbuild-win mingw/x64'}
    filter { 'action:vs2013', 'system:windows', 'architecture:x32' }
        libdirs      { 'extlibs/libs-vs2013/x86' }
        postbuildcommands { 'postbuild-win vs2013/x86'}
    filter { 'action:vs2013', 'system:windows', 'architecture:x64' }
        libdirs      { 'extlibs/libs-vs2013/x64' }
        postbuildcommands { 'postbuild-win vs2013/x64'}

-- Archituectures
filter 'configurations:*32'
    architecture 'x32'

filter 'configurations:*64'
    architecture 'x64'

-- The project
project 'game'
    targetname     'game'
    files          ( source_dirs )

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
        links        { 'opengl32', 'lua51', 'winmm' }

    -- All configs, again, because of linking order
    filter {}
        links        { 'SDL2', 'physfs', 'freetype' }


