@echo off
REM ==============================================================================
REM windows-media-session-0.8.0 - Build Script
REM ==============================================================================
REM Description: Build script with English compiler output
REM Usage:       build.bat [release|debug|clean]
REM ==============================================================================

:: Force English output BEFORE setlocal to ensure it persists to child processes
set VSLANG=1033
set PreferredUILang=en-US

setlocal enabledelayedexpansion

chcp 437 >nul

REM ==============================================================================
REM Configuration
REM ==============================================================================
set PROJECT_NAME=windows-media-session
set BUILD_TYPE=Release
set BUILD_DIR=build
set INSTALL_PREFIX=%CD%\install
set VS_GENERATOR=Visual Studio 17 2022

REM ==============================================================================
REM Parse Command Line Arguments
REM ==============================================================================
if "%1"=="debug" (
    set BUILD_TYPE=Debug
    echo [%PROJECT_NAME%] Building Debug...
) else if "%1"=="clean" (
    echo [%PROJECT_NAME%] Cleaning...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    if exist %INSTALL_PREFIX% rmdir /s /q %INSTALL_PREFIX%
    echo [OK] Clean complete
    goto :end
) else (
    echo [%PROJECT_NAME%] Building Release...
)

REM ==============================================================================
REM Logging Setup
REM ==============================================================================
if not exist logs mkdir logs
for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do set TS=%%i
set LOG=logs\%TS%_build.log

REM ==============================================================================
REM CMake Configuration Phase
REM ==============================================================================
echo.
echo ==============================================================================
echo CMake Configuration Phase
echo ==============================================================================
echo [LOG] %LOG%
cmake -B %BUILD_DIR% -S . ^
    -G "%VS_GENERATOR%" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% ^
    -DCMAKE_VS_GLOBALS="PreferredUILang=en-US" ^
    -DBUILD_EXAMPLES=ON ^
    -DBUILD_TESTING=OFF > "%LOG%" 2>&1

if errorlevel 1 (
    echo.
    echo [ERROR] CMake configuration failed. Check %LOG%
    goto :error
)

REM ==============================================================================
REM CMake Build Phase
REM ==============================================================================
echo.
echo ==============================================================================
echo CMake Build Phase
echo ==============================================================================
cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j %NUMBER_OF_PROCESSORS% ^
    -- /p:PreferredUILang=en-US /nologo >> "%LOG%" 2>&1

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Check %LOG%
    goto :error
)

REM ==============================================================================
REM CMake Install Phase (Optional)
REM ==============================================================================
REM echo.
REM echo ==============================================================================
REM echo CMake Install Phase
REM echo ==============================================================================
REM cmake --install %BUILD_DIR% --config %BUILD_TYPE%

REM if errorlevel 1 (
REM     echo.
REM     echo [ERROR] Installation failed
REM     goto :error
REM )

REM ==============================================================================
REM Build Summary
REM ==============================================================================
echo.
echo ==============================================================================
echo [OK] Build Complete
echo ==============================================================================
echo   Project:     %PROJECT_NAME%
echo   Build Type:  %BUILD_TYPE%
echo   Library:     %BUILD_DIR%\%BUILD_TYPE%\%PROJECT_NAME%.lib
echo   Examples:    %BUILD_DIR%\examples\%BUILD_TYPE%\
echo   Build Log:   %LOG%
echo ==============================================================================
echo.
goto :end

REM ==============================================================================
REM Error Handler
REM ==============================================================================
:error
echo.
echo ==============================================================================
echo [FAILED] Build process failed
echo ==============================================================================
exit /b 1

REM ==============================================================================
REM End
REM ==============================================================================
:end
endlocal
