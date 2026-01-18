"""C type definitions for PSR Database FFI bindings."""
from ctypes import Structure, c_int, c_int32
from enum import IntEnum


class LogLevel(IntEnum):
    """Log levels for console output."""
    DEBUG = 0
    INFO = 1
    WARN = 2
    ERROR = 3
    OFF = 4


class DataStructure(IntEnum):
    """Attribute data structure types."""
    SCALAR = 0
    VECTOR = 1
    SET = 2


class DataType(IntEnum):
    """Attribute data types."""
    INTEGER = 0
    FLOAT = 1
    STRING = 2


class ErrorCode(IntEnum):
    """Error codes returned by C API functions."""
    OK = 0
    INVALID_ARGUMENT = -1
    DATABASE = -2
    MIGRATION = -3
    SCHEMA = -4
    CREATE_ELEMENT = -5
    NOT_FOUND = -6


class DatabaseOptions(Structure):
    """Database options structure matching psr_database_options_t."""
    _fields_ = [
        ("read_only", c_int),
        ("console_level", c_int32),
    ]
