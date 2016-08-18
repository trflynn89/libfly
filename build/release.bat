@echo off

:: Set msbuild
set msbuild=%ProgramFiles(x86)%"\MSBuild\14.0\Bin\MSBuild.exe"
if not exist "%msbuild%" echo error: "%msbuild%": not found & goto :eof

:: Set the build directory
set root_dir=%~dp0..
if not exist "%root_dir%" echo error: "%root_dir%": not found & goto :eof

:: Perform the build
"%msbuild%" %root_dir%\libfly.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64
if %errorlevel% GEQ 1 echo error: Failed debug x64 build & goto :eof

"%msbuild%" %root_dir%\libfly.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x86
if %errorlevel% GEQ 1 echo error: Failed debug x86 build & goto :eof

"%msbuild%" %root_dir%\libfly.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
if %errorlevel% GEQ 1 echo error: Failed release x64 build & goto :eof

"%msbuild%" %root_dir%\libfly.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
if %errorlevel% GEQ 1 echo error: Failed release x86 build & goto :eof
