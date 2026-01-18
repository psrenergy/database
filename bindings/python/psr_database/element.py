"""Element builder for creating database elements."""
from ctypes import POINTER, c_char_p, c_double, c_int64, c_void_p
from typing import List, Optional, Sequence, Union

from ._ffi import ensure_bindings, get_library
from ._ffi.types import ErrorCode
from .exceptions import (
    DatabaseException,
    DatabaseOperationException,
    InvalidArgumentException,
)


class Element:
    """A builder for creating database elements.

    Elements are used to insert data into collections.
    After use, call dispose() to free native memory.

    Example:
        element = Element()
        element.set("label", "Item 1")
        element.set("value", 42)
        element.dispose()
    """

    def __init__(self) -> None:
        """Create a new empty element."""
        ensure_bindings()
        lib = get_library()
        self._ptr: c_void_p = lib.psr_element_create()
        self._disposed = False

        if not self._ptr:
            raise DatabaseOperationException("Failed to create element")

    @property
    def ptr(self) -> c_void_p:
        """Internal pointer for FFI calls."""
        self._ensure_not_disposed()
        return self._ptr

    def _ensure_not_disposed(self) -> None:
        """Raise an exception if the element has been disposed."""
        if self._disposed:
            raise DatabaseOperationException("Element has been disposed")

    def set(self, name: str, value: object) -> "Element":
        """Set a value on this element.

        Supported types:
        - None: sets a null value
        - int: 64-bit integer
        - float: 64-bit floating point
        - str: UTF-8 string
        - List[int]: array of integers
        - List[float]: array of floats
        - List[str]: array of strings

        Returns self for method chaining.
        """
        self._ensure_not_disposed()

        if value is None:
            self.set_null(name)
        elif isinstance(value, bool):
            # bool must be checked before int since bool is a subclass of int
            self.set_integer(name, int(value))
        elif isinstance(value, int):
            self.set_integer(name, value)
        elif isinstance(value, float):
            self.set_float(name, value)
        elif isinstance(value, str):
            self.set_string(name, value)
        elif isinstance(value, (list, tuple)):
            if len(value) == 0:
                raise InvalidArgumentException(f"Empty list not allowed for '{name}'")
            first = value[0]
            if isinstance(first, bool):
                self.set_array_integer(name, [int(v) for v in value])
            elif isinstance(first, int):
                self.set_array_integer(name, list(value))
            elif isinstance(first, float):
                self.set_array_float(name, list(value))
            elif isinstance(first, str):
                self.set_array_string(name, list(value))
            else:
                raise InvalidArgumentException(
                    f"Unsupported array element type {type(first).__name__} for '{name}'"
                )
        else:
            raise InvalidArgumentException(
                f"Unsupported type {type(value).__name__} for '{name}'"
            )

        return self

    def set_integer(self, name: str, value: int) -> "Element":
        """Set an integer value."""
        self._ensure_not_disposed()
        lib = get_library()

        error = lib.psr_element_set_integer(
            self._ptr, name.encode("utf-8"), c_int64(value)
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set int '{name}'")

        return self

    def set_float(self, name: str, value: float) -> "Element":
        """Set a float value."""
        self._ensure_not_disposed()
        lib = get_library()

        error = lib.psr_element_set_float(
            self._ptr, name.encode("utf-8"), c_double(value)
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set float '{name}'")

        return self

    def set_string(self, name: str, value: str) -> "Element":
        """Set a string value."""
        self._ensure_not_disposed()
        lib = get_library()

        error = lib.psr_element_set_string(
            self._ptr, name.encode("utf-8"), value.encode("utf-8")
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set string '{name}'")

        return self

    def set_null(self, name: str) -> "Element":
        """Set a null value."""
        self._ensure_not_disposed()
        lib = get_library()

        error = lib.psr_element_set_null(self._ptr, name.encode("utf-8"))
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set null '{name}'")

        return self

    def set_array_integer(self, name: str, values: Sequence[int]) -> "Element":
        """Set an array of integers."""
        self._ensure_not_disposed()
        lib = get_library()

        array_type = c_int64 * len(values)
        array = array_type(*values)

        error = lib.psr_element_set_array_integer(
            self._ptr, name.encode("utf-8"), array, len(values)
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set int array '{name}'")

        return self

    def set_array_float(self, name: str, values: Sequence[float]) -> "Element":
        """Set an array of floats."""
        self._ensure_not_disposed()
        lib = get_library()

        array_type = c_double * len(values)
        array = array_type(*values)

        error = lib.psr_element_set_array_float(
            self._ptr, name.encode("utf-8"), array, len(values)
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set float array '{name}'")

        return self

    def set_array_string(self, name: str, values: Sequence[str]) -> "Element":
        """Set an array of strings."""
        self._ensure_not_disposed()
        lib = get_library()

        # Convert strings to bytes and keep references
        encoded = [v.encode("utf-8") for v in values]
        array_type = c_char_p * len(values)
        array = array_type(*encoded)

        error = lib.psr_element_set_array_string(
            self._ptr, name.encode("utf-8"), array, len(values)
        )
        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(error, f"Failed to set string array '{name}'")

        return self

    def clear(self) -> None:
        """Clear all values from this element."""
        self._ensure_not_disposed()
        lib = get_library()
        lib.psr_element_clear(self._ptr)

    def dispose(self) -> None:
        """Free the native memory associated with this element."""
        if self._disposed:
            return
        lib = get_library()
        lib.psr_element_destroy(self._ptr)
        self._disposed = True

    def __del__(self) -> None:
        """Destructor to clean up native resources."""
        if not self._disposed:
            self.dispose()

    def __enter__(self) -> "Element":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type: object, exc_val: object, exc_tb: object) -> None:
        """Context manager exit - disposes the element."""
        self.dispose()
