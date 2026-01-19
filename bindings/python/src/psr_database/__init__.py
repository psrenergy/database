"""Python bindings for PSR Database."""

from psr_database._ffi import ffi, lib
from psr_database.exceptions import PSRDatabaseError

__all__ = [
    "version",
    "error_string",
    "PSRDatabaseError",
    "ErrorCode",
]


class ErrorCode:
    """Error code constants matching psr_error_t."""

    OK = 0
    INVALID_ARGUMENT = -1
    DATABASE = -2
    MIGRATION = -3
    SCHEMA = -4
    CREATE_ELEMENT = -5
    NOT_FOUND = -6


def version() -> str:
    """Get the PSR Database library version."""
    return ffi.string(lib.psr_version()).decode("utf-8")


def error_string(error_code: int) -> str:
    """Get a human-readable error message for an error code."""
    return ffi.string(lib.psr_error_string(error_code)).decode("utf-8")
