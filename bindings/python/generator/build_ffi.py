"""FFI builder for PSR Database Python bindings (ABI out-of-line mode)."""

import os
import sys
from pathlib import Path

from cffi import FFI

ffibuilder = FFI()

# C declarations from include/psr/c/common.h
ffibuilder.cdef("""
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

# ABI out-of-line mode: set_source with None generates a pure-Python module
# that uses dlopen at runtime (no C compiler needed at build time)
ffibuilder.set_source("psr_database._psr_database_cffi", None)


def _find_library() -> str:
    """Find the PSR Database C library path."""
    if sys.platform == "win32":
        lib_name = "libpsr_database_c.dll"
    elif sys.platform == "darwin":
        lib_name = "libpsr_database_c.dylib"
    else:
        lib_name = "libpsr_database_c.so"

    search_paths = []

    # Environment variable override
    if "PSR_DATABASE_LIB_PATH" in os.environ:
        search_paths.append(Path(os.environ["PSR_DATABASE_LIB_PATH"]))

    # Relative to package (for development)
    pkg_dir = Path(__file__).parent.parent / "src" / "psr_database"
    project_root = Path(__file__).parent.parent.parent.parent
    search_paths.append(project_root / "build" / "bin")

    # PATH directories
    for path_dir in os.environ.get("PATH", "").split(os.pathsep):
        if path_dir:
            search_paths.append(Path(path_dir))

    for search_path in search_paths:
        lib_path = search_path / lib_name
        if lib_path.exists():
            return str(lib_path)

    return lib_name


if __name__ == "__main__":
    # Generate the Python module
    generator_dir = Path(__file__).parent
    output_dir = generator_dir.parent / "src" / "psr_database"
    ffibuilder.emit_python_code(output_dir / "_psr_database_cffi.py")
    print(f"Generated {output_dir / '_psr_database_cffi.py'}")
