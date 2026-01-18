"""Lua script runner for PSR Database."""
from ctypes import c_void_p
from typing import TYPE_CHECKING

from ._ffi import ensure_bindings, get_library
from ._ffi.types import ErrorCode
from .exceptions import DatabaseOperationException, LuaException

if TYPE_CHECKING:
    from .database import Database


class LuaRunner:
    """A Lua script runner for executing Lua scripts with access to a PSR Database.

    Use LuaRunner to execute Lua scripts that can interact with the database
    via the `db` global object exposed to the script.

    Example:
        db = Database.from_schema(":memory:", "schema.sql")
        with LuaRunner(db) as lua:
            lua.run('''
                db:create_element("Collection", { label = "Item 1" })
            ''')
        db.close()
    """

    def __init__(self, db: "Database") -> None:
        """Create a new LuaRunner for the given database.

        Args:
            db: Database instance to expose to Lua scripts.
        """
        ensure_bindings()
        lib = get_library()

        self._ptr: c_void_p = lib.psr_lua_runner_new(db.ptr)
        self._disposed = False

        if not self._ptr:
            raise DatabaseOperationException("Failed to create LuaRunner")

    def _ensure_not_disposed(self) -> None:
        """Raise an exception if the runner has been disposed."""
        if self._disposed:
            raise DatabaseOperationException("LuaRunner has been disposed")

    def run(self, script: str) -> None:
        """Run a Lua script.

        The script has access to the database via the global `db` object.
        Available methods:
        - db:create_element(collection, values) - Create an element
        - db:read_scalar_strings(collection, attribute) - Read string scalars
        - db:read_scalar_integers(collection, attribute) - Read integer scalars
        - db:read_scalar_floats(collection, attribute) - Read float scalars
        - db:read_vector_strings(collection, attribute) - Read string vectors
        - db:read_vector_integers(collection, attribute) - Read integer vectors
        - db:read_vector_floats(collection, attribute) - Read float vectors

        Args:
            script: Lua script code to execute.

        Raises:
            LuaException: If the script fails to execute.
        """
        self._ensure_not_disposed()
        lib = get_library()

        error = lib.psr_lua_runner_run(self._ptr, script.encode("utf-8"))

        if error != ErrorCode.OK:
            error_ptr = lib.psr_lua_runner_get_error(self._ptr)
            if error_ptr:
                error_msg = error_ptr.decode("utf-8")
                raise LuaException(f"Lua error: {error_msg}")
            else:
                raise LuaException("Lua script execution failed")

    def dispose(self) -> None:
        """Dispose the LuaRunner and free native resources."""
        if self._disposed:
            return
        lib = get_library()
        lib.psr_lua_runner_free(self._ptr)
        self._disposed = True

    def __enter__(self) -> "LuaRunner":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type: object, exc_val: object, exc_tb: object) -> None:
        """Context manager exit - disposes the runner."""
        self.dispose()

    def __del__(self) -> None:
        """Destructor to clean up native resources."""
        if not self._disposed:
            self.dispose()
