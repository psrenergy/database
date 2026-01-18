"""PSR Database Python bindings.

A Python wrapper for the PSR Database SQLite library.

Example:
    from psr_database import Database

    with Database.from_schema(":memory:", "schema.sql") as db:
        db.create_element("Collection", {"label": "Item 1", "value": 42})
        values = db.read_scalar_integers("Collection", "value")
        print(values)  # [42]
"""

from .database import Database
from .element import Element
from .lua_runner import LuaRunner
from .exceptions import (
    DatabaseException,
    InvalidArgumentException,
    DatabaseOperationException,
    MigrationException,
    SchemaException,
    CreateElementException,
    NotFoundException,
    UnknownDatabaseException,
    LuaException,
)
from ._ffi.types import LogLevel, DataStructure, DataType

__all__ = [
    # Main classes
    "Database",
    "Element",
    "LuaRunner",
    # Exceptions
    "DatabaseException",
    "InvalidArgumentException",
    "DatabaseOperationException",
    "MigrationException",
    "SchemaException",
    "CreateElementException",
    "NotFoundException",
    "UnknownDatabaseException",
    "LuaException",
    # Enums
    "LogLevel",
    "DataStructure",
    "DataType",
]
