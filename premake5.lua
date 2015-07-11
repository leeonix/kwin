
newoption {
    trigger     = 'to',
    value       = 'path',
    description = 'Set the output location for the generated files'
}

solution 'kwin'
    configurations { 'Debug', 'Release' }
    language 'C++'
    defines { 'WIN32', '_WINDOWS' }
    flags { 'StaticRuntime' }

    location (_OPTIONS['to'] or 'build')
    targetdir 'bin'
    targetname 'kwin'
    objdir 'obj'

    filter 'action:vs*'
        disablewarnings {
            '4996', -- 4996 - same as define _CRT_SECURE_NO_WARNINGS
        }

    filter 'configurations:Debug'
        defines { '_DEBUG' }
        flags { 'Symbols' }
        optimize 'Debug'
        targetsuffix '_d'

    filter 'configurations:Release'
        defines { 'NDEBUG' }
        optimize 'Full'

project 'kwin'
    uuid '5B16DFBF-B564-49F2-9BD9-8E315D5E85A6'
    kind 'WindowedApp'
    flags { 'WinMain' }

    files {
        'src/*.h',
        'src/*.cpp',
        'src/res_vc.rc',
    }
