@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "ROOT_DIR=%%~fI"

set "BUILD_DIR=%ROOT_DIR%\build"
set "CMAKE_GENERATOR_NAME=Visual Studio 17 2022"
set "CONFIG=%~1"
set "TARGET_CHOICE=%~2"
set "ASK_TARGET=0"

if "%CONFIG%"=="" (
    set "ASK_TARGET=1"
    goto choose_config
)
if /I "%CONFIG%"=="Debug" (
    set "CONFIG=Debug"
    goto resolve_target
)
if /I "%CONFIG%"=="Release" (
    set "CONFIG=Release"
    goto resolve_target
)

echo Unknown configuration: %CONFIG%
echo Usage: %~nx0 [Debug^|Release] [App^|Editor^|All]
exit /b 1

:choose_config
echo Select build configuration:
echo   1. Debug
echo   2. Release
choice /C 12 /N /M "Enter 1 or 2: "
if errorlevel 2 set "CONFIG=Release"
if errorlevel 1 if not defined CONFIG set "CONFIG=Debug"

:resolve_target
if "%TARGET_CHOICE%"=="" (
    if "%ASK_TARGET%"=="1" goto choose_target
    set "TARGET_CHOICE=App"
)

if /I "%TARGET_CHOICE%"=="App" (
    set "TARGET_NAME=RaycastEngineApp"
    set "TARGET_LABEL=App"
    goto build
)
if /I "%TARGET_CHOICE%"=="Game" (
    set "TARGET_NAME=RaycastEngineApp"
    set "TARGET_LABEL=App"
    goto build
)
if /I "%TARGET_CHOICE%"=="Editor" (
    set "TARGET_NAME=RaycastMapEditor"
    set "TARGET_LABEL=Editor"
    goto build
)
if /I "%TARGET_CHOICE%"=="All" (
    set "TARGET_NAME="
    set "TARGET_LABEL=All"
    goto build
)

echo Unknown target: %TARGET_CHOICE%
echo Usage: %~nx0 [Debug^|Release] [App^|Editor^|All]
exit /b 1

:choose_target
echo.
echo Select build target:
echo   1. App
echo   2. Editor
echo   3. All
choice /C 123 /N /M "Enter 1, 2 or 3: "
if errorlevel 3 set "TARGET_CHOICE=All"
if errorlevel 2 if not defined TARGET_CHOICE set "TARGET_CHOICE=Editor"
if errorlevel 1 if not defined TARGET_CHOICE set "TARGET_CHOICE=App"
goto resolve_target

:build
echo.
echo Configuring %TARGET_LABEL% [%CONFIG%]...
set "CMAKE_FRESH_ARG="
set "NEEDS_FRESH_CONFIG=0"
if exist "%BUILD_DIR%\CMakeCache.txt" (
    set "EXISTING_GENERATOR="
    for /f "tokens=2 delims==" %%G in ('findstr /B /C:"CMAKE_GENERATOR:INTERNAL=" "%BUILD_DIR%\CMakeCache.txt" 2^>nul') do set "EXISTING_GENERATOR=%%G"

    if defined EXISTING_GENERATOR if /I not "!EXISTING_GENERATOR!"=="%CMAKE_GENERATOR_NAME%" (
        echo Existing build directory was configured with "!EXISTING_GENERATOR!".
        echo This script now uses "%CMAKE_GENERATOR_NAME%" so CMake generates a .sln file.
        echo Reconfiguring from a fresh CMake cache...
        set "NEEDS_FRESH_CONFIG=1"
    )
)
if exist "%BUILD_DIR%\_deps" (
    for /d %%D in ("%BUILD_DIR%\_deps\*-subbuild") do (
        if exist "%%~fD\CMakeCache.txt" (
            set "EXISTING_SUBBUILD_GENERATOR="
            for /f "tokens=2 delims==" %%G in ('findstr /B /C:"CMAKE_GENERATOR:INTERNAL=" "%%~fD\CMakeCache.txt" 2^>nul') do set "EXISTING_SUBBUILD_GENERATOR=%%G"

            if defined EXISTING_SUBBUILD_GENERATOR if /I not "!EXISTING_SUBBUILD_GENERATOR!"=="%CMAKE_GENERATOR_NAME%" (
                echo Existing FetchContent subbuild "%%~nxD" was configured with "!EXISTING_SUBBUILD_GENERATOR!".
                set "NEEDS_FRESH_CONFIG=1"
            )
        )
    )
)
if "!NEEDS_FRESH_CONFIG!"=="1" (
    set "CMAKE_FRESH_ARG=--fresh"
    if exist "%BUILD_DIR%\CMakeCache.txt" del /Q "%BUILD_DIR%\CMakeCache.txt" >nul 2>nul
    if exist "%BUILD_DIR%\CMakeFiles" rmdir /S /Q "%BUILD_DIR%\CMakeFiles" >nul 2>nul
    if exist "%BUILD_DIR%\_deps" (
        for /d %%D in ("%BUILD_DIR%\_deps\*-subbuild") do (
            if exist "%%~fD\CMakeCache.txt" del /Q "%%~fD\CMakeCache.txt" >nul 2>nul
            if exist "%%~fD\CMakeFiles" rmdir /S /Q "%%~fD\CMakeFiles" >nul 2>nul
        )
    )
)

cmake !CMAKE_FRESH_ARG! -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%CMAKE_GENERATOR_NAME%"
if errorlevel 1 exit /b %errorlevel%
del /Q "%BUILD_DIR%\*.slnx" >nul 2>nul
if exist "%BUILD_DIR%\_deps" (
    for /d %%D in ("%BUILD_DIR%\_deps\*-build" "%BUILD_DIR%\_deps\*-subbuild") do (
        del /Q "%%~fD\*.slnx" >nul 2>nul
    )
)

echo.
echo Building %TARGET_LABEL% [%CONFIG%]...
if "%TARGET_NAME%"=="" (
    cmake --build "%BUILD_DIR%" --config "%CONFIG%" --parallel
) else (
    cmake --build "%BUILD_DIR%" --config "%CONFIG%" --target "%TARGET_NAME%" --parallel
)
if errorlevel 1 exit /b %errorlevel%

echo.
echo Build finished:
if "%TARGET_NAME%"=="" (
    echo   %BUILD_DIR%\%CONFIG%\RaycastEngineApp.exe
    echo   %BUILD_DIR%\%CONFIG%\RaycastMapEditor.exe
) else (
    echo   %BUILD_DIR%\%CONFIG%\%TARGET_NAME%.exe
)
pause 
exit /b 0
