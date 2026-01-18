"""Library loader for PSR Database native library."""
import os
import sys
from ctypes import CDLL
from pathlib import Path
from typing import Optional

_cached_library: Optional[CDLL] = None


def _get_library_name() -> str:
    """Get the platform-specific library name."""
    if sys.platform == "win32":
        return "libpsr_database_c.dll"
    elif sys.platform == "darwin":
        return "libpsr_database_c.dylib"
    else:
        return "libpsr_database_c.so"


def _find_library() -> str:
    """Find the library path using various search strategies."""
    lib_name = _get_library_name()

    # Try custom path from environment variable
    custom_path = os.environ.get("PSR_DATABASE_LIB_PATH")
    if custom_path:
        custom_file = Path(custom_path) / lib_name
        if custom_file.exists():
            return str(custom_file)

    # Try current working directory
    cwd_path = Path.cwd() / lib_name
    if cwd_path.exists():
        return str(cwd_path)

    # Try directory of the executable/script
    if hasattr(sys, "executable"):
        exec_dir = Path(sys.executable).parent / lib_name
        if exec_dir.exists():
            return str(exec_dir)

    # Fall back to system PATH (let OS handle it)
    return lib_name


def get_library() -> CDLL:
    """Get the native library handle, loading it if needed."""
    global _cached_library
    if _cached_library is not None:
        return _cached_library

    lib_path = _find_library()
    _cached_library = CDLL(lib_path)
    return _cached_library
