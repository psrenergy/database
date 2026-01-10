@echo off

SET BASE_PATH=%~dp0..
SET LIB_PATH=%BASE_PATH%\..\..\build\bin

REM Add library path to PATH so Windows can find DLL dependencies
SET PATH=%LIB_PATH%;%PATH%

pushd %BASE_PATH%
dart test %*
popd
