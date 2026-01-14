#!/bin/bash

BASEPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

julia --project=$BASEPATH/.. --code-coverage=user -e "import Pkg; Pkg.test()"
julia --project=$BASEPATH/.. $BASEPATH/coverage.jl
