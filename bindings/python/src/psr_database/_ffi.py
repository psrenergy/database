"""CFFI definitions and library loading for PSR Database."""

import os
import sys
from pathlib import Path

from psr_database._psr_database_cffi import ffi

__all__ = ["ffi", "lib"]


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
    this_dir = Path(__file__).parent
    project_root = this_dir.parent.parent.parent.parent
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


lib = ffi.dlopen(_find_library())
