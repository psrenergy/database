"""Basic tests for PSR Database Python bindings."""

import psr_database
from psr_database import ErrorCode


def test_library_loads():
    """Test that the library loads successfully."""
    # If we get here, the library loaded at import time
    assert psr_database.version is not None


def test_version_returns_string():
    """Test that psr_version() returns a version string."""
    v = psr_database.version()
    assert isinstance(v, str)
    assert len(v) > 0


def test_error_string_ok():
    """Test error_string for PSR_OK."""
    msg = psr_database.error_string(ErrorCode.OK)
    assert isinstance(msg, str)
    assert "ok" in msg.lower() or "success" in msg.lower()


def test_error_string_invalid_argument():
    """Test error_string for PSR_ERROR_INVALID_ARGUMENT."""
    msg = psr_database.error_string(ErrorCode.INVALID_ARGUMENT)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_string_database():
    """Test error_string for PSR_ERROR_DATABASE."""
    msg = psr_database.error_string(ErrorCode.DATABASE)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_string_migration():
    """Test error_string for PSR_ERROR_MIGRATION."""
    msg = psr_database.error_string(ErrorCode.MIGRATION)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_string_schema():
    """Test error_string for PSR_ERROR_SCHEMA."""
    msg = psr_database.error_string(ErrorCode.SCHEMA)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_string_create_element():
    """Test error_string for PSR_ERROR_CREATE_ELEMENT."""
    msg = psr_database.error_string(ErrorCode.CREATE_ELEMENT)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_string_not_found():
    """Test error_string for PSR_ERROR_NOT_FOUND."""
    msg = psr_database.error_string(ErrorCode.NOT_FOUND)
    assert isinstance(msg, str)
    assert len(msg) > 0


def test_error_codes_match_c_api():
    """Test that ErrorCode values match the C API values."""
    assert ErrorCode.OK == 0
    assert ErrorCode.INVALID_ARGUMENT == -1
    assert ErrorCode.DATABASE == -2
    assert ErrorCode.MIGRATION == -3
    assert ErrorCode.SCHEMA == -4
    assert ErrorCode.CREATE_ELEMENT == -5
    assert ErrorCode.NOT_FOUND == -6
