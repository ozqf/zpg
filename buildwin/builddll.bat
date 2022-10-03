@echo off
cls

@echo --------------------------------------------------------
@echo Build ZPG windows DLL

set BUILD_WIN_DIR=buildwin_dlltemp

cd..
if not exist bin mkdir bin
if not exist %BUILD_WIN_DIR% mkdir %BUILD_WIN_DIR%
cd %BUILD_WIN_DIR%
del *.* /Q
@rem === COMPILER SETTINGS ===
set outputExe=/Fe../bin/zpg.dll
@rem main compile flags, elevating warnings
set compilerFlags=-nologo -Gm -MT -WX -W4 -wd4100 -wd4201 -wd4189 -wd4505 /Zi
@rem No elevated warnings
@rem set compilerFlags=-nologo -Gm -MT -W4 -wd4100 -wd4201 -wd4189 /Zi
set compilerDefines=/DPARANOID=1
@REM set compilerDefines=/DZPG_WINDOWS_DLL
@rem /DVERBOSE=1

@rem === Compile Win32 Window application
@rem set compilerInput=../src/Platform/win32_main.cpp

@rem === Compile Testing Win32 Console application
@rem Ws2_32.lib == winsock2
set compInput_0=
set compInput_1=../src/zpg/zpg_dll.cpp ../src/zpg/zpg_entities/zpg_entities.cpp
set compInput_2=../src/zpg/zpg_paint/zpg_perlin.cpp

@rem === LINK SETTINGS === (disable if running win32 console application test)
@rem set linkStr=/link
@rem set linkInputB=user32.lib opengl32.lib
@rem set linkInputC=Gdi32.lib
set linkStr=/link /SUBSYSTEM:CONSOLE

cl %compilerFlags% %compilerDefines% %outputExe% /LD %compInput_0% %compInput_1% %compInput_2% %linkStr%
@if not %ERRORLEVEL% == 0 goto :FINISHED

@rem Auto run if you like
@rem @call "../buildwin/r.bat"

@echo off

:FINISHED
@cd..
@cd buildwin

set outputExe=
set compilerFlags=
set compilerDefines=
set compInput_0=
set compInput_1=

set linkStr=
set linkInputA=
set linkInputB=
set linkInputC=

@rem @cd..
@rem cd build

@echo on

@rem Project defines
@rem /DPARANOID=1 -> enable all asserts

@rem === COMPILER FLAGS ===
@rem -WX -> warnings as errors
@rem -W4 -> Max warning level that's sane
@rem -Oi -> enable compiler 'intrinsics'
@rem -GR -> turn off runtime type information (C++ feature)
@rem -EHa -> turn off exception handling
@rem -nologo -> disable compile command line header
@rem -wd4100 -> disable warning about unreferenced function parameters
@rem -wd4189 -> disable warning - "'identifier': local variable is initialized but not referenced"
@rem -wd4201 -> disable warning - "nonstandard extension used : nameless struct/union"
@rem -wd4505 -> disable warning about removal of unused internal functions
@rem /Zi -> generate debug information
@rem /Fe -> specify file name and path
@rem -subsystem:windows,5.1 creates xp compatible windows program
@rem -MT package standard library into exe (increase compatibility)
@rem -Gm switch off minimal rebuild stuff (not using any of it)
@rem -Fm Create map file (contains addresses of all variables/functions)
@rem -opt:ref -> make compiler aggressive in removal of unused code
@rem /LD -> compile to DLL
