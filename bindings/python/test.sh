#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_PATH="$SCRIPT_DIR/../../build/bin"

# Add build directory to library path
if [[ "$OSTYPE" == "darwin"* ]]; then
    export DYLD_LIBRARY_PATH="$BUILD_PATH:$DYLD_LIBRARY_PATH"
else
    export LD_LIBRARY_PATH="$BUILD_PATH:$LD_LIBRARY_PATH"
fi

# Set library path environment variable
export PSR_DATABASE_LIB_PATH="$BUILD_PATH"

# Run pytest with all arguments passed through
python -m pytest "$SCRIPT_DIR/tests" "$@"
