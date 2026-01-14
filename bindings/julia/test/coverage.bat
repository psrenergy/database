@echo off

SET BASE_PATH=%~dp0

CALL julia +1.12.3 --project=%BASE_PATH%\.. --code-coverage=user -e "import Pkg; Pkg.test()"
CALL julia +1.12.3 --project=%BASE_PATH%\.. %BASE_PATH%coverage.jl