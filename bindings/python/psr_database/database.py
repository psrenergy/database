"""Database wrapper for PSR Database."""
from ctypes import POINTER, byref, c_char_p, c_double, c_int, c_int32, c_int64, c_size_t, c_void_p
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple, Union

from ._ffi import ensure_bindings, get_library
from ._ffi.types import DatabaseOptions, DataStructure, DataType, ErrorCode, LogLevel
from .element import Element
from .exceptions import (
    DatabaseException,
    DatabaseOperationException,
    SchemaException,
)


class Database:
    """A wrapper for the PSR Database.

    Use Database.from_schema() or Database.from_migrations() to create a new database.
    After use, call close() or use as a context manager.

    Example:
        with Database.from_schema(":memory:", "schema.sql") as db:
            db.create_element("Collection", {"label": "Item 1", "value": 42})
            values = db.read_scalar_integers("Collection", "value")
    """

    def __init__(self, ptr: c_void_p) -> None:
        """Internal constructor. Use factory methods instead."""
        self._ptr = ptr
        self._closed = False

    @property
    def ptr(self) -> c_void_p:
        """Internal pointer for FFI calls."""
        return self._ptr

    @classmethod
    def from_schema(
        cls,
        db_path: Union[str, Path],
        schema_path: Union[str, Path],
        read_only: bool = False,
        console_level: LogLevel = LogLevel.OFF,
    ) -> "Database":
        """Create a new database from a SQL schema file.

        Args:
            db_path: Path to database file, or ":memory:" for in-memory database.
            schema_path: Path to SQL schema file.
            read_only: Open database in read-only mode.
            console_level: Console log level.

        Returns:
            A new Database instance.

        Raises:
            SchemaException: If schema loading fails.
        """
        ensure_bindings()
        lib = get_library()

        options = DatabaseOptions(read_only=int(read_only), console_level=console_level)

        ptr = lib.psr_database_from_schema(
            str(db_path).encode("utf-8"),
            str(schema_path).encode("utf-8"),
            byref(options),
        )

        if not ptr:
            raise SchemaException("Failed to create database from schema")

        return cls(ptr)

    @classmethod
    def from_migrations(
        cls,
        db_path: Union[str, Path],
        migrations_path: Union[str, Path],
        read_only: bool = False,
        console_level: LogLevel = LogLevel.OFF,
    ) -> "Database":
        """Create or open a database using migrations.

        Args:
            db_path: Path to database file.
            migrations_path: Path to migrations directory.
            read_only: Open database in read-only mode.
            console_level: Console log level.

        Returns:
            A new Database instance.

        Raises:
            MigrationException: If migration fails.
        """
        ensure_bindings()
        lib = get_library()

        options = DatabaseOptions(read_only=int(read_only), console_level=console_level)

        ptr = lib.psr_database_from_migrations(
            str(db_path).encode("utf-8"),
            str(migrations_path).encode("utf-8"),
            byref(options),
        )

        if not ptr:
            raise DatabaseOperationException("Failed to create database from migrations")

        return cls(ptr)

    def _ensure_not_closed(self) -> None:
        """Raise an exception if the database has been closed."""
        if self._closed:
            raise DatabaseOperationException("Database has been closed")

    # --- Element operations ---

    def create_element(self, collection: str, values: Dict[str, Any]) -> int:
        """Create a new element in the specified collection.

        Args:
            collection: Collection name.
            values: Dictionary of attribute name -> value.

        Returns:
            The ID of the created element.
        """
        self._ensure_not_closed()

        with Element() as element:
            for key, value in values.items():
                element.set(key, value)
            return self.create_element_from_builder(collection, element)

    def create_element_from_builder(self, collection: str, element: Element) -> int:
        """Create a new element using an Element builder.

        Args:
            collection: Collection name.
            element: Element builder with values set.

        Returns:
            The ID of the created element.
        """
        self._ensure_not_closed()
        lib = get_library()

        element_id = lib.psr_database_create_element(
            self._ptr,
            collection.encode("utf-8"),
            element.ptr,
        )

        if element_id < 0:
            raise DatabaseException.from_error_code(
                element_id, f"Failed to create element in '{collection}'"
            )

        return element_id

    def update_element(self, collection: str, element_id: int, values: Dict[str, Any]) -> None:
        """Update an element's scalar attributes by ID.

        Args:
            collection: Collection name.
            element_id: Element ID.
            values: Dictionary of attribute name -> value.
        """
        self._ensure_not_closed()

        with Element() as element:
            for key, value in values.items():
                element.set(key, value)
            self.update_element_from_builder(collection, element_id, element)

    def update_element_from_builder(
        self, collection: str, element_id: int, element: Element
    ) -> None:
        """Update an element using an Element builder.

        Args:
            collection: Collection name.
            element_id: Element ID.
            element: Element builder with values to update.
        """
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_update_element(
            self._ptr,
            collection.encode("utf-8"),
            c_int64(element_id),
            element.ptr,
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update element {element_id} in '{collection}'"
            )

    def delete_element_by_id(self, collection: str, element_id: int) -> None:
        """Delete an element by ID from a collection.

        Args:
            collection: Collection name.
            element_id: Element ID to delete.
        """
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_delete_element_by_id(
            self._ptr,
            collection.encode("utf-8"),
            c_int64(element_id),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to delete element {element_id} from '{collection}'"
            )

    # --- Read scalar attributes ---

    def read_scalar_integers(self, collection: str, attribute: str) -> List[int]:
        """Read all integer values for a scalar attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_int64)()
        out_count = c_size_t()

        error = lib.psr_database_read_scalar_integers(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar integers from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_integer_array(out_values)
        return result

    def read_scalar_floats(self, collection: str, attribute: str) -> List[float]:
        """Read all float values for a scalar attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_double)()
        out_count = c_size_t()

        error = lib.psr_database_read_scalar_floats(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar floats from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_float_array(out_values)
        return result

    def read_scalar_strings(self, collection: str, attribute: str) -> List[str]:
        """Read all string values for a scalar attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_char_p)()
        out_count = c_size_t()

        error = lib.psr_database_read_scalar_strings(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar strings from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i].decode("utf-8") for i in range(count)]
        lib.psr_free_string_array(out_values, count)
        return result

    # --- Read vector attributes ---

    def read_vector_integers(self, collection: str, attribute: str) -> List[List[int]]:
        """Read all integer vectors for a vector attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_vectors = POINTER(POINTER(c_int64))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_integers(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_vectors),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector integers from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_vectors:
            return []

        result: List[List[int]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_vectors[i]:
                result.append([])
            else:
                result.append([out_vectors[i][j] for j in range(size)])

        lib.psr_free_integer_vectors(out_vectors, out_sizes, count)
        return result

    def read_vector_floats(self, collection: str, attribute: str) -> List[List[float]]:
        """Read all float vectors for a vector attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_vectors = POINTER(POINTER(c_double))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_floats(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_vectors),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector floats from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_vectors:
            return []

        result: List[List[float]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_vectors[i]:
                result.append([])
            else:
                result.append([out_vectors[i][j] for j in range(size)])

        lib.psr_free_float_vectors(out_vectors, out_sizes, count)
        return result

    def read_vector_strings(self, collection: str, attribute: str) -> List[List[str]]:
        """Read all string vectors for a vector attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_vectors = POINTER(POINTER(c_char_p))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_strings(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_vectors),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector strings from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_vectors:
            return []

        result: List[List[str]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_vectors[i]:
                result.append([])
            else:
                result.append([out_vectors[i][j].decode("utf-8") for j in range(size)])

        lib.psr_free_string_vectors(out_vectors, out_sizes, count)
        return result

    # --- Read set attributes ---

    def read_set_integers(self, collection: str, attribute: str) -> List[List[int]]:
        """Read all integer sets for a set attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_sets = POINTER(POINTER(c_int64))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_integers(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_sets),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set integers from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_sets:
            return []

        result: List[List[int]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_sets[i]:
                result.append([])
            else:
                result.append([out_sets[i][j] for j in range(size)])

        lib.psr_free_integer_vectors(out_sets, out_sizes, count)
        return result

    def read_set_floats(self, collection: str, attribute: str) -> List[List[float]]:
        """Read all float sets for a set attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_sets = POINTER(POINTER(c_double))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_floats(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_sets),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set floats from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_sets:
            return []

        result: List[List[float]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_sets[i]:
                result.append([])
            else:
                result.append([out_sets[i][j] for j in range(size)])

        lib.psr_free_float_vectors(out_sets, out_sizes, count)
        return result

    def read_set_strings(self, collection: str, attribute: str) -> List[List[str]]:
        """Read all string sets for a set attribute."""
        self._ensure_not_closed()
        lib = get_library()

        out_sets = POINTER(POINTER(c_char_p))()
        out_sizes = POINTER(c_size_t)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_strings(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_sets),
            byref(out_sizes),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set strings from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_sets:
            return []

        result: List[List[str]] = []
        for i in range(count):
            size = out_sizes[i]
            if size == 0 or not out_sets[i]:
                result.append([])
            else:
                result.append([out_sets[i][j].decode("utf-8") for j in range(size)])

        lib.psr_free_string_vectors(out_sets, out_sizes, count)
        return result

    # --- Read scalar by ID ---

    def read_scalar_integer_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> Optional[int]:
        """Read an integer value for a scalar attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_value = c_int64()
        out_has_value = c_int()

        error = lib.psr_database_read_scalar_integers_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_value),
            byref(out_has_value),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar integer by id from '{collection}.{attribute}'"
            )

        if out_has_value.value == 0:
            return None
        return out_value.value

    def read_scalar_float_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> Optional[float]:
        """Read a float value for a scalar attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_value = c_double()
        out_has_value = c_int()

        error = lib.psr_database_read_scalar_floats_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_value),
            byref(out_has_value),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar float by id from '{collection}.{attribute}'"
            )

        if out_has_value.value == 0:
            return None
        return out_value.value

    def read_scalar_string_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> Optional[str]:
        """Read a string value for a scalar attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_value = c_char_p()
        out_has_value = c_int()

        error = lib.psr_database_read_scalar_strings_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_value),
            byref(out_has_value),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar string by id from '{collection}.{attribute}'"
            )

        if out_has_value.value == 0 or not out_value.value:
            return None

        result = out_value.value.decode("utf-8")
        lib.psr_string_free(out_value)
        return result

    # --- Read vector by ID ---

    def read_vector_integers_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[int]:
        """Read integer vector for a vector attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_int64)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_integers_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector integers by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_integer_array(out_values)
        return result

    def read_vector_floats_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[float]:
        """Read float vector for a vector attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_double)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_floats_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector floats by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_float_array(out_values)
        return result

    def read_vector_strings_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[str]:
        """Read string vector for a vector attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_char_p)()
        out_count = c_size_t()

        error = lib.psr_database_read_vector_strings_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read vector strings by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i].decode("utf-8") for i in range(count)]
        lib.psr_free_string_array(out_values, count)
        return result

    # --- Read set by ID ---

    def read_set_integers_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[int]:
        """Read integer set for a set attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_int64)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_integers_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set integers by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_integer_array(out_values)
        return result

    def read_set_floats_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[float]:
        """Read float set for a set attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_double)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_floats_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set floats by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i] for i in range(count)]
        lib.psr_free_float_array(out_values)
        return result

    def read_set_strings_by_id(
        self, collection: str, attribute: str, element_id: int
    ) -> List[str]:
        """Read string set for a set attribute by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_char_p)()
        out_count = c_size_t()

        error = lib.psr_database_read_set_strings_by_id(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read set strings by id from '{collection}.{attribute}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result = [out_values[i].decode("utf-8") for i in range(count)]
        lib.psr_free_string_array(out_values, count)
        return result

    # --- Read element IDs ---

    def read_element_ids(self, collection: str) -> List[int]:
        """Read all element IDs from a collection."""
        self._ensure_not_closed()
        lib = get_library()

        out_ids = POINTER(c_int64)()
        out_count = c_size_t()

        error = lib.psr_database_read_element_ids(
            self._ptr,
            collection.encode("utf-8"),
            byref(out_ids),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read element ids from '{collection}'"
            )

        count = out_count.value
        if count == 0 or not out_ids:
            return []

        result = [out_ids[i] for i in range(count)]
        lib.psr_free_integer_array(out_ids)
        return result

    # --- Attribute type ---

    def get_attribute_type(
        self, collection: str, attribute: str
    ) -> Tuple[str, str]:
        """Get the structure and data type of an attribute.

        Returns:
            Tuple of (data_structure, data_type) as strings.
            data_structure: "scalar", "vector", or "set"
            data_type: "integer", "real", or "text"
        """
        self._ensure_not_closed()
        lib = get_library()

        out_data_structure = c_int32()
        out_data_type = c_int32()

        error = lib.psr_database_get_attribute_type(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_data_structure),
            byref(out_data_type),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to get attribute type for '{collection}.{attribute}'"
            )

        structure_map = {
            DataStructure.SCALAR: "scalar",
            DataStructure.VECTOR: "vector",
            DataStructure.SET: "set",
        }
        type_map = {
            DataType.INTEGER: "integer",
            DataType.FLOAT: "real",
            DataType.STRING: "text",
        }

        data_structure = structure_map.get(out_data_structure.value, "unknown")
        data_type = type_map.get(out_data_type.value, "unknown")

        return (data_structure, data_type)

    # --- Update scalar attributes ---

    def update_scalar_integer(
        self, collection: str, attribute: str, element_id: int, value: int
    ) -> None:
        """Update an integer scalar attribute value by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_update_scalar_integer(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            c_int64(value),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update scalar integer '{collection}.{attribute}' for id {element_id}"
            )

    def update_scalar_float(
        self, collection: str, attribute: str, element_id: int, value: float
    ) -> None:
        """Update a float scalar attribute value by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_update_scalar_float(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            c_double(value),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update scalar float '{collection}.{attribute}' for id {element_id}"
            )

    def update_scalar_string(
        self, collection: str, attribute: str, element_id: int, value: str
    ) -> None:
        """Update a string scalar attribute value by element ID."""
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_update_scalar_string(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            value.encode("utf-8"),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update scalar string '{collection}.{attribute}' for id {element_id}"
            )

    # --- Update vector attributes ---

    def update_vector_integers(
        self, collection: str, attribute: str, element_id: int, values: Sequence[int]
    ) -> None:
        """Update an integer vector attribute by element ID (replaces entire vector)."""
        self._ensure_not_closed()
        lib = get_library()

        array_type = c_int64 * len(values)
        array = array_type(*values)

        error = lib.psr_database_update_vector_integers(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update vector integers '{collection}.{attribute}' for id {element_id}"
            )

    def update_vector_floats(
        self, collection: str, attribute: str, element_id: int, values: Sequence[float]
    ) -> None:
        """Update a float vector attribute by element ID (replaces entire vector)."""
        self._ensure_not_closed()
        lib = get_library()

        array_type = c_double * len(values)
        array = array_type(*values)

        error = lib.psr_database_update_vector_floats(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update vector floats '{collection}.{attribute}' for id {element_id}"
            )

    def update_vector_strings(
        self, collection: str, attribute: str, element_id: int, values: Sequence[str]
    ) -> None:
        """Update a string vector attribute by element ID (replaces entire vector)."""
        self._ensure_not_closed()
        lib = get_library()

        encoded = [v.encode("utf-8") for v in values]
        array_type = c_char_p * len(values)
        array = array_type(*encoded)

        error = lib.psr_database_update_vector_strings(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update vector strings '{collection}.{attribute}' for id {element_id}"
            )

    # --- Update set attributes ---

    def update_set_integers(
        self, collection: str, attribute: str, element_id: int, values: Sequence[int]
    ) -> None:
        """Update an integer set attribute by element ID (replaces entire set)."""
        self._ensure_not_closed()
        lib = get_library()

        array_type = c_int64 * len(values)
        array = array_type(*values)

        error = lib.psr_database_update_set_integers(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update set integers '{collection}.{attribute}' for id {element_id}"
            )

    def update_set_floats(
        self, collection: str, attribute: str, element_id: int, values: Sequence[float]
    ) -> None:
        """Update a float set attribute by element ID (replaces entire set)."""
        self._ensure_not_closed()
        lib = get_library()

        array_type = c_double * len(values)
        array = array_type(*values)

        error = lib.psr_database_update_set_floats(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update set floats '{collection}.{attribute}' for id {element_id}"
            )

    def update_set_strings(
        self, collection: str, attribute: str, element_id: int, values: Sequence[str]
    ) -> None:
        """Update a string set attribute by element ID (replaces entire set)."""
        self._ensure_not_closed()
        lib = get_library()

        encoded = [v.encode("utf-8") for v in values]
        array_type = c_char_p * len(values)
        array = array_type(*encoded)

        error = lib.psr_database_update_set_strings(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            c_int64(element_id),
            array,
            c_size_t(len(values)),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to update set strings '{collection}.{attribute}' for id {element_id}"
            )

    # --- Relation operations ---

    def set_scalar_relation(
        self, collection: str, attribute: str, from_label: str, to_label: str
    ) -> None:
        """Set a scalar relation (foreign key) between two elements by their labels.

        Args:
            collection: Collection name.
            attribute: Relation attribute name (foreign key column).
            from_label: Label of the source element.
            to_label: Label of the target element.
        """
        self._ensure_not_closed()
        lib = get_library()

        error = lib.psr_database_set_scalar_relation(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            from_label.encode("utf-8"),
            to_label.encode("utf-8"),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to set scalar relation '{attribute}' in '{collection}'"
            )

    def read_scalar_relation(
        self, collection: str, attribute: str
    ) -> List[Optional[str]]:
        """Read scalar relation values (target labels) for a FK attribute.

        Returns null for elements with no relation set.

        Args:
            collection: Collection name.
            attribute: Relation attribute name (foreign key column).

        Returns:
            List of target labels (None for elements with no relation).
        """
        self._ensure_not_closed()
        lib = get_library()

        out_values = POINTER(c_char_p)()
        out_count = c_size_t()

        error = lib.psr_database_read_scalar_relation(
            self._ptr,
            collection.encode("utf-8"),
            attribute.encode("utf-8"),
            byref(out_values),
            byref(out_count),
        )

        if error != ErrorCode.OK:
            raise DatabaseException.from_error_code(
                error, f"Failed to read scalar relation '{attribute}' from '{collection}'"
            )

        count = out_count.value
        if count == 0 or not out_values:
            return []

        result: List[Optional[str]] = []
        for i in range(count):
            ptr = out_values[i]
            if not ptr:
                result.append(None)
            else:
                s = ptr.decode("utf-8")
                result.append(s if s else None)

        lib.psr_free_string_array(out_values, count)
        return result

    # --- Lifecycle ---

    def close(self) -> None:
        """Close the database and free native resources."""
        if self._closed:
            return
        lib = get_library()
        lib.psr_database_close(self._ptr)
        self._closed = True

    def __enter__(self) -> "Database":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type: object, exc_val: object, exc_tb: object) -> None:
        """Context manager exit - closes the database."""
        self.close()

    def __del__(self) -> None:
        """Destructor to clean up native resources."""
        if not self._closed:
            self.close()
