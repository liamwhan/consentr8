@echo off
setlocal
REM if "%WAV_VCVARS_LOCATION%"=="" set WAV_VCVARS_LOCATION="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64

set CommonCompilerFlags= /utf-8 /nologo /fp:fast /fp:except- /Gm- /GR- /EHa- /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4127 /FC

REM -MTd: Defines _DEBUG and _MT
REM -MT: Define _MT (use multithreaded static version of the runtime library)
REM -0d: Disable optimizations
REM -Ox: Enable max optimizations
REM -Zo: Enahnce optimized debugging
REM -Z7: Produce object files with full symbolic information
if NOT "%1"=="Release" (set CommonCompilerFlags=%CommonCompilerFlags% /MTd /Od /Zo /Z7) else (set CommonCompilerFlags=%CommonCompilerFlags% /MT /Ox)

set Defines=/DC8_WIN32=1 /DUNICODE=1 /D_UNICODE=1
if NOT "%1"=="Release" set Defines=%Defines% /DC8_SLOW=1 /DC8_INTERNAL=1

set CommonLinkerFlags= -incremental:no -opt:ref -LIBPATH:"%DXSDK_DIR%/Lib/x86" Gdi32.lib User32.lib OLE32.LIB Shell32.lib Kernel32.lib Pathcch.lib
set Includes=/I..\code /I"%WindowsSdkDir%Include\um" /I"%WindowsSdkDir%Include\shared"
Set Sources=..\code\win32_c8.cpp
set OutputFile=/Fec8.exe
if "%1"=="Release" set OutputFile=/Fe..\publish\c8.exe

IF NOT EXIST build mkdir build
IF NOT EXIST publish mkdir publish

taskkill /im c8.exe /f /t > NUL 2> NUL

pushd build
del *.pdb > NUL 2> NUL
del *.map > NUL 2> NUL
del *.dll > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.ico > NUL 2> NUL

xcopy ..\trayicon.ico .\ /Y /Q

echo "cl %OutputFile% %CommonCompilerFlags% %Defines% %Includes% /Fmc8.map %Sources% /link %CommonLinkerFlags%"
cl %OutputFile% %CommonCompilerFlags% %Defines% %Includes% /Fmc8.map %Sources% /link%CommonLinkerFlags%
popd
endlocal