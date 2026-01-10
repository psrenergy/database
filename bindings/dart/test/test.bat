@echo off

SET BASE_PATH=%~dp0..
SET PSR_DATABASE_LIB_PATH=%BASE_PATH%\..\..\build\bin

pushd %BASE_PATH%
dart test %*
popd
