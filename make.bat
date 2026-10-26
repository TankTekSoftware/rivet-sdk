@echo off
setlocal

set PRESET=desktop-release
if not "%~1"=="" set PRESET=%~1

set TARGET=%~2
if "%TARGET%"=="" set TARGET=build

goto :%TARGET%
if errorlevel 1 goto :unknown

:build
    cmake --preset=%PRESET%
    cmake --build --preset=%PRESET%
    goto :eof

:configure
    cmake --preset=%PRESET%
    goto :eof

:clean
    cmake --build --preset=%PRESET% --target clean
    goto :eof

:rebuild
    call :clean
    call :configure
    call :build
    goto :eof

:unknown
    echo Unknown target: %TARGET%
    echo Usage: make.bat [preset] [build^|configure^|clean^|rebuild]
    exit /b 1