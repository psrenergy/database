@echo off
setlocal

SET BASE_PATH=%~dp0
SET BUILD_PATH=%BASE_PATH%..\..\build\bin

REM Add build directory to PATH for DLL loading
SET PATH=%BUILD_PATH%;%PATH%

REM Set library path environment variable
SET PSR_DATABASE_LIB_PATH=%BUILD_PATH%

REM Run pytest with all arguments passed through
python -m pytest %BASE_PATH%tests %*

endlocal
