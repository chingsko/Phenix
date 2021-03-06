@echo off

REM @call "D:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsMSBuildCmd.bat"
@call "D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsMSBuildCmd.bat"

if "%1"=="debug" (
    if "%2"==""    (goto debug_x64)
    if "%2"=="x64" (goto debug_x64)
    if "%2"=="x86" (goto debug_x86)
)

if "%1"=="release" (
    if "%2"==""    (goto release_x64)
    if "%2"=="x64" (goto release_x64)
    if "%2"=="x86" (goto release_x86)
)

if "%1"=="debug-llvm" (
    if "%2"==""    (goto debug_x64_llvm)
    if "%2"=="x64" (goto debug_x64_llvm)
    if "%2"=="x86" (goto debug_x86_llvm)
)

if "%1"=="release-llvm" (
    if "%2"==""    (goto release_x64_llvm)
    if "%2"=="x64" (goto release_x64_llvm)
    if "%2"=="x86" (goto release_x86_llvm)
)

if "%1"=="all" (goto all)

:debug_x64
    @msbuild ./Kpp.sln /p:Configuration="Debug";Platform="x64"
    goto eof

:debug_x86
    @msbuild ./Kpp.sln /p:Configuration="Debug";Platform="x86"
    goto eof

:release_x64
    @msbuild ./Kpp.sln /p:Configuration="Release";Platform="x64"
    goto eof

:release_x86
    @msbuild ./Kpp.sln /p:Configuration="Release";Platform="x86"
    goto eof

:debug_x64_llvm
    @msbuild ./Kpp.sln /p:Configuration="Debug-llvm";Platform="x64"
    goto eof

:debug_x86_llvm
    @msbuild ./Kpp.sln /p:Configuration="Debug-llvm";Platform="x86"
    goto eof

:release_x64_llvm
    @msbuild ./Kpp.sln /p:Configuration="Release-llvm";Platform="x64"
    goto eof

:release_x86_llvm
    @msbuild ./Kpp.sln /p:Configuration="Release-llvm";Platform="x86"
    goto eof

:all
    @msbuild ./Kpp.sln /m /p:Configuration="Debug";Platform="x64"
    @msbuild ./Kpp.sln /m /p:Configuration="Debug";Platform="x86"
    @msbuild ./Kpp.sln /m /p:Configuration="Release";Platform="x64"
    @msbuild ./Kpp.sln /m /p:Configuration="Release";Platform="x86"
    @msbuild ./Kpp.sln /m /p:Configuration="Debug-llvm";Platform="x64"
    @msbuild ./Kpp.sln /m /p:Configuration="Debug-llvm";Platform="x86"
    @msbuild ./Kpp.sln /m /p:Configuration="Release-llvm";Platform="x64"
    @msbuild ./Kpp.sln /m /p:Configuration="Release-llvm";Platform="x86"
    goto eof

:eof

REM @call Boot.bat