"""Exception hierarchy for PSR Database."""
from typing import Optional

from ._ffi.types import ErrorCode


class DatabaseException(Exception):
    """Base exception for PSR Database errors."""

    def __init__(self, message: str) -> None:
        self.message = message
        super().__init__(message)

    @classmethod
    def from_error_code(cls, error_code: int, context: Optional[str] = None) -> "DatabaseException":
        """Create the appropriate exception from an error code."""
        message = context or "Operation failed"

        if error_code == ErrorCode.INVALID_ARGUMENT:
            return InvalidArgumentException(message)
        elif error_code == ErrorCode.DATABASE:
            return DatabaseOperationException(message)
        elif error_code == ErrorCode.MIGRATION:
            return MigrationException(message)
        elif error_code == ErrorCode.SCHEMA:
            return SchemaException(message)
        elif error_code == ErrorCode.CREATE_ELEMENT:
            return CreateElementException(message)
        elif error_code == ErrorCode.NOT_FOUND:
            return NotFoundException(message)
        else:
            return UnknownDatabaseException(f"Unknown error ({error_code}): {message}")


class InvalidArgumentException(DatabaseException):
    """Invalid argument provided to a function."""
    pass


class DatabaseOperationException(DatabaseException):
    """Database operation failed."""
    pass


class MigrationException(DatabaseException):
    """Database migration failed."""
    pass


class SchemaException(DatabaseException):
    """Schema validation or creation failed."""
    pass


class CreateElementException(DatabaseException):
    """Element creation failed."""
    pass


class NotFoundException(DatabaseException):
    """Requested resource not found."""
    pass


class UnknownDatabaseException(DatabaseException):
    """Unknown error occurred."""
    pass


class LuaException(DatabaseException):
    """Lua script execution failed."""
    pass
