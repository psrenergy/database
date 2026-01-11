import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'ffi/bindings.dart';
import 'ffi/library_loader.dart';
import 'element.dart';
import 'exceptions.dart';

/// Extracts a Dart value from a native psr_value_t pointer.
Object? _extractValue(Pointer<psr_value_t> ptr, {Object? defaultValue}) {
  final value = ptr.ref;
  switch (value.type) {
    case psr_value_type_t.PSR_VALUE_NULL:
      return defaultValue;
    case psr_value_type_t.PSR_VALUE_INT64:
      return value.data.int_value;
    case psr_value_type_t.PSR_VALUE_DOUBLE:
      final d = value.data.double_value;
      return d.isNaN && defaultValue != null ? defaultValue : d;
    case psr_value_type_t.PSR_VALUE_STRING:
      return value.data.string_value.cast<Utf8>().toDartString();
    case psr_value_type_t.PSR_VALUE_ARRAY:
      final array = value.data.array_value;
      final count = array.count;
      final elements = array.elements;
      return List.generate(count, (i) {
        final elemPtr = Pointer<psr_value_t>.fromAddress(
          elements.address + i * sizeOf<psr_value_t>(),
        );
        return _extractValue(elemPtr, defaultValue: defaultValue);
      });
    default:
      return defaultValue;
  }
}

/// Extracts a value at a specific index from a psr_value_t array.
Object? _extractValueAtIndex(
  Pointer<psr_value_t> valuesPtr,
  int index, {
  Object? defaultValue,
}) {
  final ptr = Pointer<psr_value_t>.fromAddress(
    valuesPtr.address + index * sizeOf<psr_value_t>(),
  );
  return _extractValue(ptr, defaultValue: defaultValue);
}

/// A wrapper for the PSR Database.
///
/// Use [Database.fromSchema] to create a new database from a SQL schema file.
/// After use, call [close] to free native resources.
class Database {
  Pointer<psr_database_t> _ptr;
  bool _isClosed = false;

  Database._(this._ptr);

  /// Creates a new database from a SQL schema file.
  ///
  /// - [dbPath] - Path to the SQLite database file.
  /// - [schemaPath] - Path to the SQL schema file.
  ///
  /// Throws [SchemaException] if the schema is invalid.
  /// Throws [DatabaseOperationException] if the database cannot be created.
  factory Database.fromSchema(String dbPath, String schemaPath) {
    final dbPathPtr = dbPath.toNativeUtf8();
    final schemaPathPtr = schemaPath.toNativeUtf8();
    final optionsPtr = calloc<psr_database_options_t>();

    try {
      // Get default options
      optionsPtr.ref = bindings.psr_database_options_default();

      final ptr = bindings.psr_database_from_schema(
        dbPathPtr.cast(),
        schemaPathPtr.cast(),
        optionsPtr,
      );

      if (ptr == nullptr) {
        throw const SchemaException('Failed to create database from schema');
      }

      return Database._(ptr);
    } finally {
      malloc.free(dbPathPtr);
      malloc.free(schemaPathPtr);
      calloc.free(optionsPtr);
    }
  }

  void _ensureNotClosed() {
    if (_isClosed) {
      throw const DatabaseOperationException('Database has been closed');
    }
  }

  /// Creates a new element in the specified collection.
  ///
  /// - [collection] - The name of the collection.
  /// - [values] - A map of attribute names to values.
  ///
  /// Returns the ID of the created element.
  ///
  /// Supported value types:
  /// - `null` - null value
  /// - `int` - 64-bit integer
  /// - `double` - 64-bit floating point
  /// - `String` - UTF-8 string
  /// - `List<int>` - array of integers
  /// - `List<double>` - array of doubles
  /// - `List<String>` - array of strings
  int createElement(String collection, Map<String, Object?> values) {
    _ensureNotClosed();

    final element = Element();
    try {
      for (final entry in values.entries) {
        element.set(entry.key, entry.value);
      }
      return createElementFromBuilder(collection, element);
    } finally {
      element.dispose();
    }
  }

  /// Creates a new element in the specified collection using an Element builder.
  ///
  /// - [collection] - The name of the collection.
  /// - [element] - The Element builder containing the values.
  ///
  /// Returns the ID of the created element.
  ///
  /// Note: The caller is responsible for disposing the Element after use.
  int createElementFromBuilder(String collection, Element element) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    try {
      final id = bindings.psr_database_create_element(
        _ptr,
        collectionPtr.cast(),
        element.ptr.cast(),
      );

      if (id < 0) {
        throw DatabaseException.fromError(
          id,
          "Failed to create element in '$collection'",
        );
      }

      return id;
    } finally {
      malloc.free(collectionPtr);
    }
  }

  /// Closes the database and frees native resources.
  ///
  /// After calling this method, the database cannot be used.
  void close() {
    if (_isClosed) return;
    bindings.psr_database_close(_ptr);
    _isClosed = true;
  }

  // ============================================================
  // Read Operations
  // ============================================================

  /// Reads all values of a scalar attribute from a collection.
  ///
  /// Returns a list of values, one for each element in the collection.
  /// Use [defaultValue] to substitute for null values.
  ///
  /// Throws [NotFoundException] if the attribute is not found.
  List<Object?> readScalarParameters(
    String collection,
    String attribute, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_scalar_parameters(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read scalar parameters '$attribute' from '$collection'",
        );
      }

      try {
        return List.generate(
          result.count,
          (i) => _extractValueAtIndex(
            result.values,
            i,
            defaultValue: defaultValue,
          ),
        );
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
    }
  }

  /// Reads a scalar attribute value for a specific element by label.
  ///
  /// Use [defaultValue] to substitute for null values.
  ///
  /// Throws [NotFoundException] if the element or attribute is not found.
  Object? readScalarParameter(
    String collection,
    String attribute,
    String label, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_scalar_parameter(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
        labelPtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read scalar parameter '$attribute' for '$label' from '$collection'",
        );
      }

      try {
        return _extractValue(result.values, defaultValue: defaultValue);
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
      malloc.free(labelPtr);
    }
  }

  /// Reads all values of a vector attribute from a collection.
  ///
  /// Returns a list of lists, one vector for each element in the collection.
  /// Use [defaultValue] to substitute for null values within vectors.
  ///
  /// Throws [NotFoundException] if the attribute is not found.
  List<List<Object?>> readVectorParameters(
    String collection,
    String attribute, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_vector_parameters(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read vector parameters '$attribute' from '$collection'",
        );
      }

      try {
        return List.generate(result.count, (i) {
          final value = _extractValueAtIndex(
            result.values,
            i,
            defaultValue: defaultValue,
          );
          return (value as List<Object?>?) ?? [];
        });
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
    }
  }

  /// Reads a vector attribute value for a specific element by label.
  ///
  /// Use [defaultValue] to substitute for null values within the vector.
  ///
  /// Throws [NotFoundException] if the element or attribute is not found.
  List<Object?> readVectorParameter(
    String collection,
    String attribute,
    String label, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_vector_parameter(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
        labelPtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read vector parameter '$attribute' for '$label' from '$collection'",
        );
      }

      try {
        final value = _extractValue(result.values, defaultValue: defaultValue);
        return (value as List<Object?>?) ?? [];
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
      malloc.free(labelPtr);
    }
  }

  /// Reads all values of a set attribute from a collection.
  ///
  /// Returns a list of lists, one set for each element in the collection.
  /// Use [defaultValue] to substitute for null values within sets.
  ///
  /// Throws [NotFoundException] if the attribute is not found.
  List<List<Object?>> readSetParameters(
    String collection,
    String attribute, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_set_parameters(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read set parameters '$attribute' from '$collection'",
        );
      }

      try {
        return List.generate(result.count, (i) {
          final value = _extractValueAtIndex(
            result.values,
            i,
            defaultValue: defaultValue,
          );
          return (value as List<Object?>?) ?? [];
        });
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
    }
  }

  /// Reads a set attribute value for a specific element by label.
  ///
  /// Use [defaultValue] to substitute for null values within the set.
  ///
  /// Throws [NotFoundException] if the element or attribute is not found.
  List<Object?> readSetParameter(
    String collection,
    String attribute,
    String label, {
    Object? defaultValue,
  }) {
    _ensureNotClosed();

    final collectionPtr = collection.toNativeUtf8();
    final attributePtr = attribute.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      final result = bindings.psr_database_read_set_parameter(
        _ptr,
        collectionPtr.cast(),
        attributePtr.cast(),
        labelPtr.cast(),
      );

      if (result.error != 0) {
        throw DatabaseException.fromError(
          result.error,
          "Failed to read set parameter '$attribute' for '$label' from '$collection'",
        );
      }

      try {
        final value = _extractValue(result.values, defaultValue: defaultValue);
        return (value as List<Object?>?) ?? [];
      } finally {
        final resultPtr = calloc<psr_read_result_t>();
        resultPtr.ref = result;
        bindings.psr_read_result_free(resultPtr);
        calloc.free(resultPtr);
      }
    } finally {
      malloc.free(collectionPtr);
      malloc.free(attributePtr);
      malloc.free(labelPtr);
    }
  }

}
