"""CFFI definitions and library loading for PSR Database."""

import os
import sys
from pathlib import Path

from cffi import FFI

ffi = FFI()

# C declarations from include/psr/c/common.h
ffi.cdef("""
    // Error codes
    typedef enum {
        PSR_OK = 0,
        PSR_ERROR_INVALID_ARGUMENT = -1,
        PSR_ERROR_DATABASE = -2,
        PSR_ERROR_MIGRATION = -3,
        PSR_ERROR_SCHEMA = -4,
        PSR_ERROR_CREATE_ELEMENT = -5,
        PSR_ERROR_NOT_FOUND = -6,
    } psr_error_t;

    // Utility functions
    const char* psr_error_string(psr_error_t error);
    const char* psr_version(void);
""")


def _find_library() -> Path:
    """Find the PSR Database C library."""
    # Platform-specific library name
    if sys.platform == "win32":
        lib_name = "libpsr_database_c.dll"
    elif sys.platform == "darwin":
        lib_name = "libpsr_database_c.dylib"
    else:
        lib_name = "libpsr_database_c.so"

    # Search paths
    search_paths = []

    # 1. Check environment variable
    if "PSR_DATABASE_LIB_PATH" in os.environ:
        search_paths.append(Path(os.environ["PSR_DATABASE_LIB_PATH"]))

    # 2. Check relative to this file (for development: bindings/python -> build/bin)
    this_dir = Path(__file__).parent
    project_root = this_dir.parent.parent.parent.parent
    search_paths.append(project_root / "build" / "bin")

    # 3. Check PATH directories
    for path_dir in os.environ.get("PATH", "").split(os.pathsep):
        if path_dir:
            search_paths.append(Path(path_dir))

    # Search for the library
    for search_path in search_paths:
        lib_path = search_path / lib_name
        if lib_path.exists():
            return lib_path

    # If not found, return just the name and let the system handle it
    return Path(lib_name)


def _load_library():
    """Load the PSR Database C library."""
    lib_path = _find_library()
    return ffi.dlopen(str(lib_path))


# Load library at module import time
lib = _load_library()
