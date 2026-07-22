@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "ROOT_DIR=%%~fI"

set "BUILD_DIR=%ROOT_DIR%\build"
set "FORCE_CLEAN=0"
set "USAGE_EXIT_CODE=0"

if /I "%~1"=="/?" goto usage
if /I "%~1"=="-?" goto usage
if /I "%~1"=="--help" goto usage
if /I "%~1"=="/y" set "FORCE_CLEAN=1"
if /I "%~1"=="-y" set "FORCE_CLEAN=1"
if /I "%~1"=="--yes" set "FORCE_CLEAN=1"

if not "%~2"=="" goto usage_error
if not "%~1"=="" if "%FORCE_CLEAN%"=="0" goto usage_error

for %%I in ("%BUILD_DIR%") do set "BUILD_DIR_NAME=%%~nxI"
if /I not "%BUILD_DIR_NAME%"=="build" (
    echo Refusing to clean unexpected directory:
    echo   %BUILD_DIR%
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    echo Build directory does not exist:
    echo   %BUILD_DIR%
    exit /b 0
)

if "%FORCE_CLEAN%"=="0" (
    echo This will delete:
    echo   %BUILD_DIR%
    choice /C YN /N /M "Continue? [Y/N]: "
    if errorlevel 2 (
        echo Clean cancelled.
        exit /b 0
    )
)

echo Cleaning build directory...
rmdir /S /Q "%BUILD_DIR%"
if errorlevel 1 exit /b %errorlevel%

echo Build directory cleaned:
echo   %BUILD_DIR%
exit /b 0

:usage_error
echo Unknown argument: %*
echo.
set "USAGE_EXIT_CODE=1"

:usage
echo Usage: %~nx0 [/y]
echo.
echo   /y    Delete the build directory without confirmation.
exit /b %USAGE_EXIT_CODE%
