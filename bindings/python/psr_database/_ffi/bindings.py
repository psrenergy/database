"""ctypes function signatures for PSR Database C API."""
from ctypes import (
    POINTER,
    c_char_p,
    c_double,
    c_int,
    c_int32,
    c_int64,
    c_size_t,
    c_void_p,
)

from .library_loader import get_library
from .types import DatabaseOptions


def _setup_bindings() -> None:
    """Configure all C function signatures."""
    lib = get_library()

    # --- Common functions ---
    lib.psr_error_string.argtypes = [c_int32]
    lib.psr_error_string.restype = c_char_p

    lib.psr_version.argtypes = []
    lib.psr_version.restype = c_char_p

    # --- Database options ---
    lib.psr_database_options_default.argtypes = []
    lib.psr_database_options_default.restype = DatabaseOptions

    # --- Database lifecycle ---
    lib.psr_database_open.argtypes = [c_char_p, POINTER(DatabaseOptions)]
    lib.psr_database_open.restype = c_void_p

    lib.psr_database_from_migrations.argtypes = [c_char_p, c_char_p, POINTER(DatabaseOptions)]
    lib.psr_database_from_migrations.restype = c_void_p

    lib.psr_database_from_schema.argtypes = [c_char_p, c_char_p, POINTER(DatabaseOptions)]
    lib.psr_database_from_schema.restype = c_void_p

    lib.psr_database_close.argtypes = [c_void_p]
    lib.psr_database_close.restype = None

    lib.psr_database_is_healthy.argtypes = [c_void_p]
    lib.psr_database_is_healthy.restype = c_int

    lib.psr_database_path.argtypes = [c_void_p]
    lib.psr_database_path.restype = c_char_p

    lib.psr_database_current_version.argtypes = [c_void_p]
    lib.psr_database_current_version.restype = c_int64

    # --- Element operations ---
    lib.psr_database_create_element.argtypes = [c_void_p, c_char_p, c_void_p]
    lib.psr_database_create_element.restype = c_int64

    lib.psr_database_update_element.argtypes = [c_void_p, c_char_p, c_int64, c_void_p]
    lib.psr_database_update_element.restype = c_int32

    lib.psr_database_delete_element_by_id.argtypes = [c_void_p, c_char_p, c_int64]
    lib.psr_database_delete_element_by_id.restype = c_int32

    # --- Relation operations ---
    lib.psr_database_set_scalar_relation.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p, c_char_p]
    lib.psr_database_set_scalar_relation.restype = c_int32

    lib.psr_database_read_scalar_relation.argtypes = [
        c_void_p, c_char_p, c_char_p, POINTER(POINTER(c_char_p)), POINTER(c_size_t)
    ]
    lib.psr_database_read_scalar_relation.restype = c_int32

    # --- Read scalar attributes ---
    lib.psr_database_read_scalar_integers.argtypes = [
        c_void_p, c_char_p, c_char_p, POINTER(POINTER(c_int64)), POINTER(c_size_t)
    ]
    lib.psr_database_read_scalar_integers.restype = c_int32

    lib.psr_database_read_scalar_floats.argtypes = [
        c_void_p, c_char_p, c_char_p, POINTER(POINTER(c_double)), POINTER(c_size_t)
    ]
    lib.psr_database_read_scalar_floats.restype = c_int32

    lib.psr_database_read_scalar_strings.argtypes = [
        c_void_p, c_char_p, c_char_p, POINTER(POINTER(c_char_p)), POINTER(c_size_t)
    ]
    lib.psr_database_read_scalar_strings.restype = c_int32

    # --- Read vector attributes ---
    lib.psr_database_read_vector_integers.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_int64))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_integers.restype = c_int32

    lib.psr_database_read_vector_floats.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_double))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_floats.restype = c_int32

    lib.psr_database_read_vector_strings.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_char_p))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_strings.restype = c_int32

    # --- Read set attributes ---
    lib.psr_database_read_set_integers.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_int64))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_integers.restype = c_int32

    lib.psr_database_read_set_floats.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_double))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_floats.restype = c_int32

    lib.psr_database_read_set_strings.argtypes = [
        c_void_p, c_char_p, c_char_p,
        POINTER(POINTER(POINTER(c_char_p))), POINTER(POINTER(c_size_t)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_strings.restype = c_int32

    # --- Read scalar by ID ---
    lib.psr_database_read_scalar_integers_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_int64), POINTER(c_int)
    ]
    lib.psr_database_read_scalar_integers_by_id.restype = c_int32

    lib.psr_database_read_scalar_floats_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_double), POINTER(c_int)
    ]
    lib.psr_database_read_scalar_floats_by_id.restype = c_int32

    lib.psr_database_read_scalar_strings_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_char_p), POINTER(c_int)
    ]
    lib.psr_database_read_scalar_strings_by_id.restype = c_int32

    # --- Read vector by ID ---
    lib.psr_database_read_vector_integers_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_int64)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_integers_by_id.restype = c_int32

    lib.psr_database_read_vector_floats_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_double)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_floats_by_id.restype = c_int32

    lib.psr_database_read_vector_strings_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_char_p)), POINTER(c_size_t)
    ]
    lib.psr_database_read_vector_strings_by_id.restype = c_int32

    # --- Read set by ID ---
    lib.psr_database_read_set_integers_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_int64)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_integers_by_id.restype = c_int32

    lib.psr_database_read_set_floats_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_double)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_floats_by_id.restype = c_int32

    lib.psr_database_read_set_strings_by_id.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(POINTER(c_char_p)), POINTER(c_size_t)
    ]
    lib.psr_database_read_set_strings_by_id.restype = c_int32

    # --- Read element IDs ---
    lib.psr_database_read_element_ids.argtypes = [
        c_void_p, c_char_p, POINTER(POINTER(c_int64)), POINTER(c_size_t)
    ]
    lib.psr_database_read_element_ids.restype = c_int32

    # --- Get attribute type ---
    lib.psr_database_get_attribute_type.argtypes = [
        c_void_p, c_char_p, c_char_p, POINTER(c_int32), POINTER(c_int32)
    ]
    lib.psr_database_get_attribute_type.restype = c_int32

    # --- Update scalar attributes ---
    lib.psr_database_update_scalar_integer.argtypes = [c_void_p, c_char_p, c_char_p, c_int64, c_int64]
    lib.psr_database_update_scalar_integer.restype = c_int32

    lib.psr_database_update_scalar_float.argtypes = [c_void_p, c_char_p, c_char_p, c_int64, c_double]
    lib.psr_database_update_scalar_float.restype = c_int32

    lib.psr_database_update_scalar_string.argtypes = [c_void_p, c_char_p, c_char_p, c_int64, c_char_p]
    lib.psr_database_update_scalar_string.restype = c_int32

    # --- Update vector attributes ---
    lib.psr_database_update_vector_integers.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_int64), c_size_t
    ]
    lib.psr_database_update_vector_integers.restype = c_int32

    lib.psr_database_update_vector_floats.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_double), c_size_t
    ]
    lib.psr_database_update_vector_floats.restype = c_int32

    lib.psr_database_update_vector_strings.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_char_p), c_size_t
    ]
    lib.psr_database_update_vector_strings.restype = c_int32

    # --- Update set attributes ---
    lib.psr_database_update_set_integers.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_int64), c_size_t
    ]
    lib.psr_database_update_set_integers.restype = c_int32

    lib.psr_database_update_set_floats.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_double), c_size_t
    ]
    lib.psr_database_update_set_floats.restype = c_int32

    lib.psr_database_update_set_strings.argtypes = [
        c_void_p, c_char_p, c_char_p, c_int64, POINTER(c_char_p), c_size_t
    ]
    lib.psr_database_update_set_strings.restype = c_int32

    # --- Memory cleanup ---
    lib.psr_free_integer_array.argtypes = [POINTER(c_int64)]
    lib.psr_free_integer_array.restype = None

    lib.psr_free_float_array.argtypes = [POINTER(c_double)]
    lib.psr_free_float_array.restype = None

    lib.psr_free_string_array.argtypes = [POINTER(c_char_p), c_size_t]
    lib.psr_free_string_array.restype = None

    lib.psr_free_integer_vectors.argtypes = [POINTER(POINTER(c_int64)), POINTER(c_size_t), c_size_t]
    lib.psr_free_integer_vectors.restype = None

    lib.psr_free_float_vectors.argtypes = [POINTER(POINTER(c_double)), POINTER(c_size_t), c_size_t]
    lib.psr_free_float_vectors.restype = None

    lib.psr_free_string_vectors.argtypes = [POINTER(POINTER(c_char_p)), POINTER(c_size_t), c_size_t]
    lib.psr_free_string_vectors.restype = None

    # --- Element functions ---
    lib.psr_element_create.argtypes = []
    lib.psr_element_create.restype = c_void_p

    lib.psr_element_destroy.argtypes = [c_void_p]
    lib.psr_element_destroy.restype = None

    lib.psr_element_clear.argtypes = [c_void_p]
    lib.psr_element_clear.restype = None

    lib.psr_element_set_integer.argtypes = [c_void_p, c_char_p, c_int64]
    lib.psr_element_set_integer.restype = c_int32

    lib.psr_element_set_float.argtypes = [c_void_p, c_char_p, c_double]
    lib.psr_element_set_float.restype = c_int32

    lib.psr_element_set_string.argtypes = [c_void_p, c_char_p, c_char_p]
    lib.psr_element_set_string.restype = c_int32

    lib.psr_element_set_null.argtypes = [c_void_p, c_char_p]
    lib.psr_element_set_null.restype = c_int32

    lib.psr_element_set_array_integer.argtypes = [c_void_p, c_char_p, POINTER(c_int64), c_int32]
    lib.psr_element_set_array_integer.restype = c_int32

    lib.psr_element_set_array_float.argtypes = [c_void_p, c_char_p, POINTER(c_double), c_int32]
    lib.psr_element_set_array_float.restype = c_int32

    lib.psr_element_set_array_string.argtypes = [c_void_p, c_char_p, POINTER(c_char_p), c_int32]
    lib.psr_element_set_array_string.restype = c_int32

    lib.psr_element_has_scalars.argtypes = [c_void_p]
    lib.psr_element_has_scalars.restype = c_int

    lib.psr_element_has_arrays.argtypes = [c_void_p]
    lib.psr_element_has_arrays.restype = c_int

    lib.psr_element_scalar_count.argtypes = [c_void_p]
    lib.psr_element_scalar_count.restype = c_size_t

    lib.psr_element_array_count.argtypes = [c_void_p]
    lib.psr_element_array_count.restype = c_size_t

    lib.psr_element_to_string.argtypes = [c_void_p]
    lib.psr_element_to_string.restype = c_char_p

    lib.psr_string_free.argtypes = [c_char_p]
    lib.psr_string_free.restype = None

    # --- Lua runner functions ---
    lib.psr_lua_runner_new.argtypes = [c_void_p]
    lib.psr_lua_runner_new.restype = c_void_p

    lib.psr_lua_runner_free.argtypes = [c_void_p]
    lib.psr_lua_runner_free.restype = None

    lib.psr_lua_runner_run.argtypes = [c_void_p, c_char_p]
    lib.psr_lua_runner_run.restype = c_int32

    lib.psr_lua_runner_get_error.argtypes = [c_void_p]
    lib.psr_lua_runner_get_error.restype = c_char_p


# Setup bindings on module import
_bindings_initialized = False


def ensure_bindings() -> None:
    """Ensure bindings are initialized."""
    global _bindings_initialized
    if not _bindings_initialized:
        _setup_bindings()
        _bindings_initialized = True
