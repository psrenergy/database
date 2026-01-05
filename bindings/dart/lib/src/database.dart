import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

// Error codes
const int PSR_OK = 0;
const int PSR_ERROR_INVALID_ARGUMENT = -1;
const int PSR_ERROR_DATABASE = -2;
const int PSR_ERROR_QUERY = -3;
const int PSR_ERROR_NO_MEMORY = -4;
const int PSR_ERROR_NOT_OPEN = -5;
const int PSR_ERROR_INDEX_OUT_OF_RANGE = -6;
const int PSR_ERROR_MIGRATION = -7;

// Log levels
const int PSR_LOG_DEBUG = 0;
const int PSR_LOG_INFO = 1;
const int PSR_LOG_WARN = 2;
const int PSR_LOG_ERROR = 3;
const int PSR_LOG_OFF = 4;

// Value types
const int PSR_TYPE_NULL = 0;
const int PSR_TYPE_INTEGER = 1;
const int PSR_TYPE_FLOAT = 2;
const int PSR_TYPE_TEXT = 3;
const int PSR_TYPE_BLOB = 4;

// Opaque handles
typedef PsrDatabase = Pointer<Void>;
typedef PsrResult = Pointer<Void>;
typedef PsrElement = Pointer<Void>;
typedef PsrTimeSeries = Pointer<Void>;
typedef PsrStringArray = Pointer<Void>;

// FFI function typedefs
typedef _PsrDatabaseOpenNative = PsrDatabase Function(
    Pointer<Utf8> path, Int32 consoleLevel, Int32 readOnly, Pointer<Int32> error);
typedef _PsrDatabaseOpen = PsrDatabase Function(
    Pointer<Utf8> path, int consoleLevel, int readOnly, Pointer<Int32> error);

typedef _PsrDatabaseFromSchemaNative = PsrDatabase Function(
    Pointer<Utf8> dbPath, Pointer<Utf8> schemaPath, Int32 consoleLevel, Pointer<Int32> error);
typedef _PsrDatabaseFromSchema = PsrDatabase Function(
    Pointer<Utf8> dbPath, Pointer<Utf8> schemaPath, int consoleLevel, Pointer<Int32> error);

typedef _PsrDatabaseFromSqlFileNative = PsrDatabase Function(
    Pointer<Utf8> dbPath, Pointer<Utf8> sqlFilePath, Int32 consoleLevel, Pointer<Int32> error);
typedef _PsrDatabaseFromSqlFile = PsrDatabase Function(
    Pointer<Utf8> dbPath, Pointer<Utf8> sqlFilePath, int consoleLevel, Pointer<Int32> error);

typedef _PsrDatabaseCloseNative = Void Function(PsrDatabase db);
typedef _PsrDatabaseClose = void Function(PsrDatabase db);

typedef _PsrDatabaseIsOpenNative = Int32 Function(PsrDatabase db);
typedef _PsrDatabaseIsOpen = int Function(PsrDatabase db);

typedef _PsrDatabaseExecuteNative = PsrResult Function(PsrDatabase db, Pointer<Utf8> sql, Pointer<Int32> error);
typedef _PsrDatabaseExecute = PsrResult Function(PsrDatabase db, Pointer<Utf8> sql, Pointer<Int32> error);

typedef _PsrDatabaseLastInsertRowidNative = Int64 Function(PsrDatabase db);
typedef _PsrDatabaseLastInsertRowid = int Function(PsrDatabase db);

typedef _PsrDatabaseChangesNative = Int32 Function(PsrDatabase db);
typedef _PsrDatabaseChanges = int Function(PsrDatabase db);

typedef _PsrDatabaseBeginTransactionNative = Int32 Function(PsrDatabase db);
typedef _PsrDatabaseBeginTransaction = int Function(PsrDatabase db);

typedef _PsrDatabaseCommitNative = Int32 Function(PsrDatabase db);
typedef _PsrDatabaseCommit = int Function(PsrDatabase db);

typedef _PsrDatabaseRollbackNative = Int32 Function(PsrDatabase db);
typedef _PsrDatabaseRollback = int Function(PsrDatabase db);

typedef _PsrDatabaseErrorMessageNative = Pointer<Utf8> Function(PsrDatabase db);
typedef _PsrDatabaseErrorMessage = Pointer<Utf8> Function(PsrDatabase db);

typedef _PsrElementCreateNative = PsrElement Function();
typedef _PsrElementCreate = PsrElement Function();

typedef _PsrElementFreeNative = Void Function(PsrElement elem);
typedef _PsrElementFree = void Function(PsrElement elem);

typedef _PsrElementSetIntNative = Int32 Function(PsrElement elem, Pointer<Utf8> column, Int64 value);
typedef _PsrElementSetInt = int Function(PsrElement elem, Pointer<Utf8> column, int value);

typedef _PsrElementSetDoubleNative = Int32 Function(PsrElement elem, Pointer<Utf8> column, Double value);
typedef _PsrElementSetDouble = int Function(PsrElement elem, Pointer<Utf8> column, double value);

typedef _PsrElementSetStringNative = Int32 Function(PsrElement elem, Pointer<Utf8> column, Pointer<Utf8> value);
typedef _PsrElementSetString = int Function(PsrElement elem, Pointer<Utf8> column, Pointer<Utf8> value);

typedef _PsrElementSetNullNative = Int32 Function(PsrElement elem, Pointer<Utf8> column);
typedef _PsrElementSetNull = int Function(PsrElement elem, Pointer<Utf8> column);

typedef _PsrElementSetDoubleArrayNative = Int32 Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Double> values, IntPtr count);
typedef _PsrElementSetDoubleArray = int Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Double> values, int count);

typedef _PsrElementSetIntArrayNative = Int32 Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Int64> values, IntPtr count);
typedef _PsrElementSetIntArray = int Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Int64> values, int count);

typedef _PsrElementSetStringArrayNative = Int32 Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Pointer<Utf8>> values, IntPtr count);
typedef _PsrElementSetStringArray = int Function(
    PsrElement elem, Pointer<Utf8> column, Pointer<Pointer<Utf8>> values, int count);

typedef _PsrDatabaseCreateElementNative = Int64 Function(
    PsrDatabase db, Pointer<Utf8> table, PsrElement elem, Pointer<Int32> error);
typedef _PsrDatabaseCreateElement = int Function(
    PsrDatabase db, Pointer<Utf8> table, PsrElement elem, Pointer<Int32> error);

typedef _PsrDatabaseGetElementIdNative = Int64 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Int32> error);
typedef _PsrDatabaseGetElementId = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Int32> error);

typedef _PsrDatabaseDeleteElementNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label);
typedef _PsrDatabaseDeleteElement = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label);

typedef _PsrDatabaseDeleteElementByIdNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Int64 id);
typedef _PsrDatabaseDeleteElementById = int Function(
    PsrDatabase db, Pointer<Utf8> collection, int id);

typedef _PsrResultFreeNative = Void Function(PsrResult result);
typedef _PsrResultFree = void Function(PsrResult result);

typedef _PsrResultRowCountNative = IntPtr Function(PsrResult result);
typedef _PsrResultRowCount = int Function(PsrResult result);

typedef _PsrResultColumnCountNative = IntPtr Function(PsrResult result);
typedef _PsrResultColumnCount = int Function(PsrResult result);

typedef _PsrResultColumnNameNative = Pointer<Utf8> Function(PsrResult result, IntPtr col);
typedef _PsrResultColumnName = Pointer<Utf8> Function(PsrResult result, int col);

typedef _PsrResultIsNullNative = Int32 Function(PsrResult result, IntPtr row, IntPtr col);
typedef _PsrResultIsNull = int Function(PsrResult result, int row, int col);

typedef _PsrResultGetIntNative = Int32 Function(PsrResult result, IntPtr row, IntPtr col, Pointer<Int64> value);
typedef _PsrResultGetInt = int Function(PsrResult result, int row, int col, Pointer<Int64> value);

typedef _PsrResultGetDoubleNative = Int32 Function(PsrResult result, IntPtr row, IntPtr col, Pointer<Double> value);
typedef _PsrResultGetDouble = int Function(PsrResult result, int row, int col, Pointer<Double> value);

typedef _PsrResultGetStringNative = Pointer<Utf8> Function(PsrResult result, IntPtr row, IntPtr col);
typedef _PsrResultGetString = Pointer<Utf8> Function(PsrResult result, int row, int col);

typedef _PsrDatabaseReadElementScalarAttributesNative = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Int32> error);
typedef _PsrDatabaseReadElementScalarAttributes = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Int32> error);

typedef _PsrDatabaseReadElementVectorGroupNative = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Utf8> group, Pointer<Int32> error);
typedef _PsrDatabaseReadElementVectorGroup = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Utf8> group, Pointer<Int32> error);

typedef _PsrDatabaseGetElementIdsNative = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Int32> error);
typedef _PsrDatabaseGetElementIds = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Int32> error);

typedef _PsrDatabaseSetTimeSeriesFileNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> parameter, Pointer<Utf8> filePath);
typedef _PsrDatabaseSetTimeSeriesFile = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> parameter, Pointer<Utf8> filePath);

typedef _PsrDatabaseReadTimeSeriesFileNative = Pointer<Utf8> Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> parameter, Pointer<Int32> error);
typedef _PsrDatabaseReadTimeSeriesFile = Pointer<Utf8> Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> parameter, Pointer<Int32> error);

typedef _PsrDatabaseUpdateScalarParameterDoubleNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Double value);
typedef _PsrDatabaseUpdateScalarParameterDouble = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, double value);

typedef _PsrDatabaseUpdateScalarParameterStringNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Utf8> value);
typedef _PsrDatabaseUpdateScalarParameterString = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Utf8> value);

typedef _PsrDatabaseReadElementSetGroupNative = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Utf8> group, Pointer<Int32> error);
typedef _PsrDatabaseReadElementSetGroup = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> label, Pointer<Utf8> group, Pointer<Int32> error);

typedef _PsrDatabaseUpdateVectorParametersDoubleNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Double> values, IntPtr count);
typedef _PsrDatabaseUpdateVectorParametersDouble = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Double> values, int count);

typedef _PsrDatabaseUpdateSetParametersDoubleNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Double> values, IntPtr count);
typedef _PsrDatabaseUpdateSetParametersDouble = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Double> values, int count);

typedef _PsrDatabaseReadTimeSeriesTableNative = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Int32> error);
typedef _PsrDatabaseReadTimeSeriesTable = PsrResult Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Pointer<Int32> error);

typedef _PsrDatabaseUpdateTimeSeriesRowNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, Double value, Pointer<Utf8> dateTime);
typedef _PsrDatabaseUpdateTimeSeriesRow = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> column, Pointer<Utf8> label, double value, Pointer<Utf8> dateTime);

typedef _PsrDatabaseDeleteTimeSeriesNative = Int32 Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> group, Pointer<Utf8> label);
typedef _PsrDatabaseDeleteTimeSeries = int Function(
    PsrDatabase db, Pointer<Utf8> collection, Pointer<Utf8> group, Pointer<Utf8> label);

class DatabaseException implements Exception {
  final String message;
  DatabaseException(this.message);

  @override
  String toString() => 'DatabaseException: $message';
}

class _NativeBindings {
  late final DynamicLibrary _lib;
  final String _libraryPath;
  Directory? _originalDir;

  late final _PsrDatabaseOpen databaseOpen;
  late final _PsrDatabaseFromSchema databaseFromSchema;
  late final _PsrDatabaseFromSqlFile databaseFromSqlFile;
  late final _PsrDatabaseClose databaseClose;
  late final _PsrDatabaseIsOpen databaseIsOpen;
  late final _PsrDatabaseExecute databaseExecute;
  late final _PsrDatabaseLastInsertRowid databaseLastInsertRowid;
  late final _PsrDatabaseChanges databaseChanges;
  late final _PsrDatabaseBeginTransaction databaseBeginTransaction;
  late final _PsrDatabaseCommit databaseCommit;
  late final _PsrDatabaseRollback databaseRollback;
  late final _PsrDatabaseErrorMessage databaseErrorMessage;
  late final _PsrElementCreate elementCreate;
  late final _PsrElementFree elementFree;
  late final _PsrElementSetInt elementSetInt;
  late final _PsrElementSetDouble elementSetDouble;
  late final _PsrElementSetString elementSetString;
  late final _PsrElementSetNull elementSetNull;
  late final _PsrElementSetDoubleArray elementSetDoubleArray;
  late final _PsrElementSetIntArray elementSetIntArray;
  late final _PsrElementSetStringArray elementSetStringArray;
  late final _PsrDatabaseCreateElement databaseCreateElement;
  late final _PsrDatabaseGetElementId databaseGetElementId;
  late final _PsrDatabaseDeleteElement databaseDeleteElement;
  late final _PsrDatabaseDeleteElementById databaseDeleteElementById;
  late final _PsrResultFree resultFree;
  late final _PsrResultRowCount resultRowCount;
  late final _PsrResultColumnCount resultColumnCount;
  late final _PsrResultColumnName resultColumnName;
  late final _PsrResultIsNull resultIsNull;
  late final _PsrResultGetInt resultGetInt;
  late final _PsrResultGetDouble resultGetDouble;
  late final _PsrResultGetString resultGetString;
  late final _PsrDatabaseReadElementScalarAttributes readElementScalarAttributes;
  late final _PsrDatabaseReadElementVectorGroup readElementVectorGroup;
  late final _PsrDatabaseGetElementIds getElementIds;
  late final _PsrDatabaseSetTimeSeriesFile setTimeSeriesFile;
  late final _PsrDatabaseReadTimeSeriesFile readTimeSeriesFile;
  late final _PsrDatabaseUpdateScalarParameterDouble updateScalarParameterDouble;
  late final _PsrDatabaseUpdateScalarParameterString updateScalarParameterString;
  late final _PsrDatabaseReadElementSetGroup readElementSetGroup;
  late final _PsrDatabaseUpdateVectorParametersDouble updateVectorParametersDouble;
  late final _PsrDatabaseUpdateSetParametersDouble updateSetParametersDouble;
  late final _PsrDatabaseReadTimeSeriesTable readTimeSeriesTable;
  late final _PsrDatabaseUpdateTimeSeriesRow updateTimeSeriesRow;
  late final _PsrDatabaseDeleteTimeSeries deleteTimeSeries;

  _NativeBindings(String libraryPath) : _libraryPath = libraryPath {
    // Change to the library directory so dependent DLLs can be found
    final libDir = File(libraryPath).parent;
    if (libDir.existsSync()) {
      _originalDir = Directory.current;
      Directory.current = libDir;
    }

    try {
      _lib = DynamicLibrary.open(libraryPath);
    } finally {
      // Restore original directory
      if (_originalDir != null) {
        Directory.current = _originalDir!;
      }
    }

    databaseOpen = _lib.lookupFunction<_PsrDatabaseOpenNative, _PsrDatabaseOpen>('psr_database_open');
    databaseFromSchema =
        _lib.lookupFunction<_PsrDatabaseFromSchemaNative, _PsrDatabaseFromSchema>('psr_database_from_schema');
    databaseFromSqlFile =
        _lib.lookupFunction<_PsrDatabaseFromSqlFileNative, _PsrDatabaseFromSqlFile>('psr_database_from_sql_file');
    databaseClose = _lib.lookupFunction<_PsrDatabaseCloseNative, _PsrDatabaseClose>('psr_database_close');
    databaseIsOpen = _lib.lookupFunction<_PsrDatabaseIsOpenNative, _PsrDatabaseIsOpen>('psr_database_is_open');
    databaseExecute = _lib.lookupFunction<_PsrDatabaseExecuteNative, _PsrDatabaseExecute>('psr_database_execute');
    databaseLastInsertRowid = _lib.lookupFunction<_PsrDatabaseLastInsertRowidNative, _PsrDatabaseLastInsertRowid>(
        'psr_database_last_insert_rowid');
    databaseChanges = _lib.lookupFunction<_PsrDatabaseChangesNative, _PsrDatabaseChanges>('psr_database_changes');
    databaseBeginTransaction =
        _lib.lookupFunction<_PsrDatabaseBeginTransactionNative, _PsrDatabaseBeginTransaction>(
            'psr_database_begin_transaction');
    databaseCommit = _lib.lookupFunction<_PsrDatabaseCommitNative, _PsrDatabaseCommit>('psr_database_commit');
    databaseRollback = _lib.lookupFunction<_PsrDatabaseRollbackNative, _PsrDatabaseRollback>('psr_database_rollback');
    databaseErrorMessage =
        _lib.lookupFunction<_PsrDatabaseErrorMessageNative, _PsrDatabaseErrorMessage>('psr_database_error_message');
    elementCreate = _lib.lookupFunction<_PsrElementCreateNative, _PsrElementCreate>('psr_element_create');
    elementFree = _lib.lookupFunction<_PsrElementFreeNative, _PsrElementFree>('psr_element_free');
    elementSetInt = _lib.lookupFunction<_PsrElementSetIntNative, _PsrElementSetInt>('psr_element_set_int');
    elementSetDouble = _lib.lookupFunction<_PsrElementSetDoubleNative, _PsrElementSetDouble>('psr_element_set_double');
    elementSetString = _lib.lookupFunction<_PsrElementSetStringNative, _PsrElementSetString>('psr_element_set_string');
    elementSetNull = _lib.lookupFunction<_PsrElementSetNullNative, _PsrElementSetNull>('psr_element_set_null');
    elementSetDoubleArray =
        _lib.lookupFunction<_PsrElementSetDoubleArrayNative, _PsrElementSetDoubleArray>('psr_element_set_double_array');
    elementSetIntArray =
        _lib.lookupFunction<_PsrElementSetIntArrayNative, _PsrElementSetIntArray>('psr_element_set_int_array');
    elementSetStringArray =
        _lib.lookupFunction<_PsrElementSetStringArrayNative, _PsrElementSetStringArray>('psr_element_set_string_array');
    databaseCreateElement =
        _lib.lookupFunction<_PsrDatabaseCreateElementNative, _PsrDatabaseCreateElement>('psr_database_create_element');
    databaseGetElementId =
        _lib.lookupFunction<_PsrDatabaseGetElementIdNative, _PsrDatabaseGetElementId>('psr_database_get_element_id');
    databaseDeleteElement =
        _lib.lookupFunction<_PsrDatabaseDeleteElementNative, _PsrDatabaseDeleteElement>('psr_database_delete_element');
    databaseDeleteElementById = _lib.lookupFunction<_PsrDatabaseDeleteElementByIdNative, _PsrDatabaseDeleteElementById>(
        'psr_database_delete_element_by_id');
    resultFree = _lib.lookupFunction<_PsrResultFreeNative, _PsrResultFree>('psr_result_free');
    resultRowCount = _lib.lookupFunction<_PsrResultRowCountNative, _PsrResultRowCount>('psr_result_row_count');
    resultColumnCount =
        _lib.lookupFunction<_PsrResultColumnCountNative, _PsrResultColumnCount>('psr_result_column_count');
    resultColumnName = _lib.lookupFunction<_PsrResultColumnNameNative, _PsrResultColumnName>('psr_result_column_name');
    resultIsNull = _lib.lookupFunction<_PsrResultIsNullNative, _PsrResultIsNull>('psr_result_is_null');
    resultGetInt = _lib.lookupFunction<_PsrResultGetIntNative, _PsrResultGetInt>('psr_result_get_int');
    resultGetDouble = _lib.lookupFunction<_PsrResultGetDoubleNative, _PsrResultGetDouble>('psr_result_get_double');
    resultGetString = _lib.lookupFunction<_PsrResultGetStringNative, _PsrResultGetString>('psr_result_get_string');
    readElementScalarAttributes =
        _lib.lookupFunction<_PsrDatabaseReadElementScalarAttributesNative, _PsrDatabaseReadElementScalarAttributes>(
            'psr_database_read_element_scalar_attributes');
    readElementVectorGroup =
        _lib.lookupFunction<_PsrDatabaseReadElementVectorGroupNative, _PsrDatabaseReadElementVectorGroup>(
            'psr_database_read_element_vector_group');
    getElementIds =
        _lib.lookupFunction<_PsrDatabaseGetElementIdsNative, _PsrDatabaseGetElementIds>('psr_database_get_element_ids');
    setTimeSeriesFile =
        _lib.lookupFunction<_PsrDatabaseSetTimeSeriesFileNative, _PsrDatabaseSetTimeSeriesFile>(
            'psr_database_set_time_series_file');
    readTimeSeriesFile =
        _lib.lookupFunction<_PsrDatabaseReadTimeSeriesFileNative, _PsrDatabaseReadTimeSeriesFile>(
            'psr_database_read_time_series_file');
    updateScalarParameterDouble =
        _lib.lookupFunction<_PsrDatabaseUpdateScalarParameterDoubleNative, _PsrDatabaseUpdateScalarParameterDouble>(
            'psr_database_update_scalar_parameter_double');
    updateScalarParameterString =
        _lib.lookupFunction<_PsrDatabaseUpdateScalarParameterStringNative, _PsrDatabaseUpdateScalarParameterString>(
            'psr_database_update_scalar_parameter_string');
    readElementSetGroup =
        _lib.lookupFunction<_PsrDatabaseReadElementSetGroupNative, _PsrDatabaseReadElementSetGroup>(
            'psr_database_read_element_set_group');
    updateVectorParametersDouble =
        _lib.lookupFunction<_PsrDatabaseUpdateVectorParametersDoubleNative, _PsrDatabaseUpdateVectorParametersDouble>(
            'psr_database_update_vector_parameters_double');
    updateSetParametersDouble =
        _lib.lookupFunction<_PsrDatabaseUpdateSetParametersDoubleNative, _PsrDatabaseUpdateSetParametersDouble>(
            'psr_database_update_set_parameters_double');
    readTimeSeriesTable =
        _lib.lookupFunction<_PsrDatabaseReadTimeSeriesTableNative, _PsrDatabaseReadTimeSeriesTable>(
            'psr_database_read_time_series_table');
    updateTimeSeriesRow =
        _lib.lookupFunction<_PsrDatabaseUpdateTimeSeriesRowNative, _PsrDatabaseUpdateTimeSeriesRow>(
            'psr_database_update_time_series_row');
    deleteTimeSeries =
        _lib.lookupFunction<_PsrDatabaseDeleteTimeSeriesNative, _PsrDatabaseDeleteTimeSeries>(
            'psr_database_delete_time_series');
  }
}

_NativeBindings? _bindings;

_NativeBindings _getBindings() {
  if (_bindings == null) {
    String libPath;
    if (Platform.isWindows) {
      libPath = 'libpsr_database_c.dll';
    } else if (Platform.isLinux) {
      libPath = 'libpsr_database_c.so';
    } else if (Platform.isMacOS) {
      libPath = 'libpsr_database_c.dylib';
    } else {
      throw UnsupportedError('Unsupported platform');
    }
    _bindings = _NativeBindings(libPath);
  }
  return _bindings!;
}

void _setLibraryPath(String path) {
  _bindings = _NativeBindings(path);
}

/// Set the path to the psr_database_c library
void setLibraryPath(String path) {
  _setLibraryPath(path);
}

class DatabaseSQLite {
  final PsrDatabase _db;
  final _NativeBindings _bindings;

  DatabaseSQLite._(this._db, this._bindings);

  static void setLibraryPath(String path) {
    _setLibraryPath(path);
  }

  factory DatabaseSQLite.fromSchema({
    required String databasePath,
    required String schemaPath,
    int logLevel = PSR_LOG_OFF,
  }) {
    final bindings = _getBindings();
    final dbPathPtr = databasePath.toNativeUtf8();
    final schemaPathPtr = schemaPath.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      // Use psr_database_from_sql_file for single SQL files
      final db = bindings.databaseFromSqlFile(dbPathPtr, schemaPathPtr, logLevel, errorPtr);
      if (db == nullptr) {
        final error = errorPtr.value;
        throw DatabaseException('Failed to create database from schema: error code $error');
      }
      return DatabaseSQLite._(db, bindings);
    } finally {
      calloc.free(dbPathPtr);
      calloc.free(schemaPathPtr);
      calloc.free(errorPtr);
    }
  }

  factory DatabaseSQLite.open({
    required String databasePath,
    bool readOnly = false,
    int logLevel = PSR_LOG_OFF,
  }) {
    final bindings = _getBindings();
    final pathPtr = databasePath.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final db = bindings.databaseOpen(pathPtr, logLevel, readOnly ? 1 : 0, errorPtr);
      if (db == nullptr) {
        final error = errorPtr.value;
        throw DatabaseException('Failed to open database: error code $error');
      }
      return DatabaseSQLite._(db, bindings);
    } finally {
      calloc.free(pathPtr);
      calloc.free(errorPtr);
    }
  }

  void close() {
    _bindings.databaseClose(_db);
  }

  bool get isOpen => _bindings.databaseIsOpen(_db) != 0;

  int createElement(String collection, Map<String, dynamic> attributes) {
    final elem = _bindings.elementCreate();
    if (elem == nullptr) {
      throw DatabaseException('Failed to create element builder');
    }

    final tablePtr = collection.toNativeUtf8();

    try {
      for (final entry in attributes.entries) {
        final colPtr = entry.key.toNativeUtf8();
        try {
          final value = entry.value;
          int result;

          if (value == null) {
            result = _bindings.elementSetNull(elem, colPtr);
          } else if (value is int) {
            result = _bindings.elementSetInt(elem, colPtr, value);
          } else if (value is double) {
            result = _bindings.elementSetDouble(elem, colPtr, value);
          } else if (value is String) {
            // Check if it's a foreign key reference (label) by looking at column name
            final valPtr = value.toNativeUtf8();
            result = _bindings.elementSetString(elem, colPtr, valPtr);
            calloc.free(valPtr);
          } else if (value is DateTime) {
            final isoString = _formatDateTime(value);
            final valPtr = isoString.toNativeUtf8();
            result = _bindings.elementSetString(elem, colPtr, valPtr);
            calloc.free(valPtr);
          } else if (value is List<double>) {
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i];
            }
            result = _bindings.elementSetDoubleArray(elem, colPtr, arr, value.length);
            calloc.free(arr);
          } else if (value is List<int>) {
            final arr = calloc<Int64>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i];
            }
            result = _bindings.elementSetIntArray(elem, colPtr, arr, value.length);
            calloc.free(arr);
          } else if (value is List<String>) {
            final arr = calloc<Pointer<Utf8>>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i].toNativeUtf8();
            }
            result = _bindings.elementSetStringArray(elem, colPtr, arr, value.length);
            for (int i = 0; i < value.length; i++) {
              calloc.free(arr[i]);
            }
            calloc.free(arr);
          } else if (value is List<DateTime>) {
            final arr = calloc<Pointer<Utf8>>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = _formatDateTime(value[i]).toNativeUtf8();
            }
            result = _bindings.elementSetStringArray(elem, colPtr, arr, value.length);
            for (int i = 0; i < value.length; i++) {
              calloc.free(arr[i]);
            }
            calloc.free(arr);
          } else {
            throw DatabaseException('Unsupported value type: ${value.runtimeType}');
          }

          if (result != PSR_OK) {
            throw DatabaseException('Failed to set attribute ${entry.key}');
          }
        } finally {
          calloc.free(colPtr);
        }
      }

      final errorPtr = calloc<Int32>();
      try {
        final id = _bindings.databaseCreateElement(_db, tablePtr, elem, errorPtr);
        if (id <= 0 && errorPtr.value != PSR_OK) {
          throw DatabaseException('Failed to create element in $collection');
        }
        return id;
      } finally {
        calloc.free(errorPtr);
      }
    } finally {
      _bindings.elementFree(elem);
      calloc.free(tablePtr);
    }
  }

  void deleteElementFromLabel(String collection, String label) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      final result = _bindings.databaseDeleteElement(_db, collPtr, labelPtr);
      if (result != PSR_OK) {
        throw DatabaseException('Failed to delete element $label from $collection');
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
    }
  }

  void deleteElementFromId(String collection, int id) {
    final collPtr = collection.toNativeUtf8();

    try {
      final result = _bindings.databaseDeleteElementById(_db, collPtr, id);
      if (result != PSR_OK) {
        throw DatabaseException('Failed to delete element with id $id from $collection');
      }
    } finally {
      calloc.free(collPtr);
    }
  }

  Map<String, dynamic> readElementScalarAttributesFromLabel(String collection, String label) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.readElementScalarAttributes(_db, collPtr, labelPtr, errorPtr);
      if (result == nullptr) {
        throw DatabaseException('Failed to read scalar attributes for $label in $collection');
      }

      try {
        return _resultToMap(result);
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
      calloc.free(errorPtr);
    }
  }

  List<Map<String, dynamic>> readElementGroupOfVectorAttributesFromLabel(
      String collection, String label, String group) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();
    final groupPtr = group.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.readElementVectorGroup(_db, collPtr, labelPtr, groupPtr, errorPtr);
      if (result == nullptr) {
        throw DatabaseException('Failed to read vector group $group for $label in $collection');
      }

      try {
        return _resultToListOfMaps(result);
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
      calloc.free(groupPtr);
      calloc.free(errorPtr);
    }
  }

  List<int> getElementsIdsFromCollectionId(String collection) {
    final collPtr = collection.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.getElementIds(_db, collPtr, errorPtr);
      if (result == nullptr) {
        throw DatabaseException('Failed to get element IDs for $collection');
      }

      try {
        final ids = <int>[];
        final rowCount = _bindings.resultRowCount(result);
        final valuePtr = calloc<Int64>();
        for (int i = 0; i < rowCount; i++) {
          if (_bindings.resultGetInt(result, i, 0, valuePtr) == PSR_OK) {
            ids.add(valuePtr.value);
          }
        }
        calloc.free(valuePtr);
        return ids;
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(errorPtr);
    }
  }

  void setTimeSeriesFile(String collection, Map<String, String> files) {
    final collPtr = collection.toNativeUtf8();

    try {
      for (final entry in files.entries) {
        final paramPtr = entry.key.toNativeUtf8();
        final pathPtr = entry.value.toNativeUtf8();

        try {
          final result = _bindings.setTimeSeriesFile(_db, collPtr, paramPtr, pathPtr);
          if (result != PSR_OK) {
            throw DatabaseException('Failed to set time series file for ${entry.key}');
          }
        } finally {
          calloc.free(paramPtr);
          calloc.free(pathPtr);
        }
      }
    } finally {
      calloc.free(collPtr);
    }
  }

  Map<String, String> readCollectionTimeSeriesFiles(String collection) {
    final files = <String, String>{};
    final table = '${collection}_time_series_files';

    // Query the time series files table
    final sqlPtr = 'SELECT * FROM "$table"'.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.databaseExecute(_db, sqlPtr, errorPtr);
      if (result == nullptr) {
        return files;
      }

      try {
        final colCount = _bindings.resultColumnCount(result);
        final rowCount = _bindings.resultRowCount(result);

        if (rowCount == 0) return files;

        // Each column is a parameter, its value is the file path
        for (int col = 0; col < colCount; col++) {
          final colNamePtr = _bindings.resultColumnName(result, col);
          if (colNamePtr == nullptr) continue;
          final colName = colNamePtr.toDartString();

          if (_bindings.resultIsNull(result, 0, col) == 0) {
            final strPtr = _bindings.resultGetString(result, 0, col);
            if (strPtr != nullptr) {
              files[colName] = strPtr.toDartString();
            }
          }
        }
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(sqlPtr);
      calloc.free(errorPtr);
    }

    return files;
  }

  void updateElementScalarAttributesFromLabel(String collection, String label, Map<String, dynamic> attributes) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      for (final entry in attributes.entries) {
        final colPtr = entry.key.toNativeUtf8();
        try {
          final value = entry.value;
          int result;

          if (value == null) {
            // Use string update with empty string for null - or we need a separate null update function
            // For now, use SQL UPDATE directly
            final sql = 'UPDATE "$collection" SET "${entry.key}" = NULL WHERE label = ?';
            final sqlPtr = sql.toNativeUtf8();
            final errorPtr = calloc<Int32>();
            try {
              // This is a workaround - the C API doesn't have a null update function
              final updateResult = _bindings.databaseExecute(_db, 'UPDATE "$collection" SET "${entry.key}" = NULL WHERE label = \'$label\''.toNativeUtf8(), errorPtr);
              if (updateResult != nullptr) {
                _bindings.resultFree(updateResult);
              }
            } finally {
              calloc.free(errorPtr);
            }
            result = PSR_OK;
          } else if (value is double) {
            result = _bindings.updateScalarParameterDouble(_db, collPtr, colPtr, labelPtr, value);
          } else if (value is int) {
            result = _bindings.updateScalarParameterDouble(_db, collPtr, colPtr, labelPtr, value.toDouble());
          } else if (value is String) {
            final valPtr = value.toNativeUtf8();
            result = _bindings.updateScalarParameterString(_db, collPtr, colPtr, labelPtr, valPtr);
            calloc.free(valPtr);
          } else if (value is DateTime) {
            final isoString = _formatDateTime(value);
            final valPtr = isoString.toNativeUtf8();
            result = _bindings.updateScalarParameterString(_db, collPtr, colPtr, labelPtr, valPtr);
            calloc.free(valPtr);
          } else {
            throw DatabaseException('Unsupported value type: ${value.runtimeType}');
          }

          if (result != PSR_OK) {
            throw DatabaseException('Failed to update attribute ${entry.key}');
          }
        } finally {
          calloc.free(colPtr);
        }
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
    }
  }

  List<Map<String, dynamic>> readElementGroupOfSetAttributesFromLabel(
      String collection, String label, String group) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();
    final groupPtr = group.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.readElementSetGroup(_db, collPtr, labelPtr, groupPtr, errorPtr);
      if (result == nullptr) {
        throw DatabaseException('Failed to read set group $group for $label in $collection');
      }

      try {
        return _resultToListOfMaps(result);
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
      calloc.free(groupPtr);
      calloc.free(errorPtr);
    }
  }

  void updateElementGroupOfVectorAttributesFromLabel(
      String collection, String label, String group, Map<String, dynamic> values) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      for (final entry in values.entries) {
        final colPtr = entry.key.toNativeUtf8();
        try {
          final value = entry.value;

          if (value is List<double>) {
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i];
            }
            final result = _bindings.updateVectorParametersDouble(_db, collPtr, colPtr, labelPtr, arr, value.length);
            calloc.free(arr);
            if (result != PSR_OK) {
              throw DatabaseException('Failed to update vector attribute ${entry.key}');
            }
          } else if (value is List<int>) {
            // Convert to double array
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i].toDouble();
            }
            final result = _bindings.updateVectorParametersDouble(_db, collPtr, colPtr, labelPtr, arr, value.length);
            calloc.free(arr);
            if (result != PSR_OK) {
              throw DatabaseException('Failed to update vector attribute ${entry.key}');
            }
          } else if (value is List<dynamic>) {
            // Handle mixed list with possible nulls
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              if (value[i] == null) {
                arr[i] = double.nan;
              } else if (value[i] is num) {
                arr[i] = (value[i] as num).toDouble();
              } else {
                throw DatabaseException('Unsupported value in list: ${value[i].runtimeType}');
              }
            }
            final result = _bindings.updateVectorParametersDouble(_db, collPtr, colPtr, labelPtr, arr, value.length);
            calloc.free(arr);
            if (result != PSR_OK) {
              throw DatabaseException('Failed to update vector attribute ${entry.key}');
            }
          } else {
            throw DatabaseException('Unsupported value type: ${value.runtimeType}');
          }
        } finally {
          calloc.free(colPtr);
        }
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
    }
  }

  void updateElementGroupOfSetAttributesFromLabel(
      String collection, String label, String group, Map<String, dynamic> values) {
    final collPtr = collection.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      for (final entry in values.entries) {
        final colPtr = entry.key.toNativeUtf8();
        try {
          final value = entry.value;

          if (value is List<double>) {
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              arr[i] = value[i];
            }
            final result = _bindings.updateSetParametersDouble(_db, collPtr, colPtr, labelPtr, arr, value.length);
            calloc.free(arr);
            if (result != PSR_OK) {
              throw DatabaseException('Failed to update set attribute ${entry.key}');
            }
          } else if (value is List<int>) {
            throw DatabaseException('Integer arrays not supported for set parameters - expected List<double>');
          } else if (value is List<dynamic>) {
            // Handle mixed list with possible nulls
            final arr = calloc<Double>(value.length);
            for (int i = 0; i < value.length; i++) {
              if (value[i] == null) {
                arr[i] = double.nan;
              } else if (value[i] is num) {
                arr[i] = (value[i] as num).toDouble();
              } else {
                throw DatabaseException('Unsupported value in list: ${value[i].runtimeType}');
              }
            }
            final result = _bindings.updateSetParametersDouble(_db, collPtr, colPtr, labelPtr, arr, value.length);
            calloc.free(arr);
            if (result != PSR_OK) {
              throw DatabaseException('Failed to update set attribute ${entry.key}');
            }
          } else {
            throw DatabaseException('Unsupported value type: ${value.runtimeType}');
          }
        } finally {
          calloc.free(colPtr);
        }
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(labelPtr);
    }
  }

  List<Map<String, dynamic>> readTimeSeriesTable(String collection, String column, String label) {
    final collPtr = collection.toNativeUtf8();
    final colPtr = column.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.readTimeSeriesTable(_db, collPtr, colPtr, labelPtr, errorPtr);
      if (result == nullptr) {
        return [];
      }

      try {
        return _resultToListOfMaps(result);
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(colPtr);
      calloc.free(labelPtr);
      calloc.free(errorPtr);
    }
  }

  void updateTimeSeries(String collection, String column, String label, double value, Map<String, dynamic> dimensions) {
    final collPtr = collection.toNativeUtf8();
    final colPtr = column.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    // Extract date_time from dimensions
    final dateTime = dimensions['date_time'];
    if (dateTime == null) {
      throw DatabaseException('date_time is required in dimensions');
    }

    final dateTimeStr = dateTime is DateTime ? _formatDateTime(dateTime) : dateTime.toString();
    final dateTimePtr = dateTimeStr.toNativeUtf8();

    try {
      final result = _bindings.updateTimeSeriesRow(_db, collPtr, colPtr, labelPtr, value, dateTimePtr);
      if (result != PSR_OK) {
        throw DatabaseException('Failed to update time series: time step not found');
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(colPtr);
      calloc.free(labelPtr);
      calloc.free(dateTimePtr);
    }
  }

  void deleteTimeSeries(String collection, String group, String label) {
    final collPtr = collection.toNativeUtf8();
    final groupPtr = group.toNativeUtf8();
    final labelPtr = label.toNativeUtf8();

    try {
      final result = _bindings.deleteTimeSeries(_db, collPtr, groupPtr, labelPtr);
      if (result != PSR_OK) {
        throw DatabaseException('Failed to delete time series for $label in $collection');
      }
    } finally {
      calloc.free(collPtr);
      calloc.free(groupPtr);
      calloc.free(labelPtr);
    }
  }

  List<Map<String, dynamic>> readElementGroupOfTimeSeriesAttributesFromId(
      String collection, int id, String group, List<String> dimensions) {
    // Query the time series table directly
    final table = '${collection}_time_series_$group';
    final sql = 'SELECT * FROM "$table" WHERE id = $id';
    final sqlPtr = sql.toNativeUtf8();
    final errorPtr = calloc<Int32>();

    try {
      final result = _bindings.databaseExecute(_db, sqlPtr, errorPtr);
      if (result == nullptr) {
        return [];
      }

      try {
        return _resultToListOfMaps(result);
      } finally {
        _bindings.resultFree(result);
      }
    } finally {
      calloc.free(sqlPtr);
      calloc.free(errorPtr);
    }
  }

  void updateElementGroupOfTimeSeriesFromId(String collection, int id, String group, Map<String, dynamic> data) {
    // This is a complex operation - insert new rows into the time series table
    final table = '${collection}_time_series_$group';

    // Get the length of the data arrays
    int? length;
    for (final entry in data.entries) {
      if (entry.value is List) {
        final listLength = (entry.value as List).length;
        if (length == null) {
          length = listLength;
        } else if (length != listLength) {
          throw DatabaseException('All arrays must have the same length');
        }
      }
    }

    if (length == null || length == 0) {
      throw DatabaseException('No data to insert');
    }

    // Insert each row
    for (int i = 0; i < length; i++) {
      final columns = <String>['id'];
      final values = <String>['$id'];

      for (final entry in data.entries) {
        final value = (entry.value as List)[i];
        columns.add('"${entry.key}"');

        if (value == null) {
          values.add('NULL');
        } else if (value is num) {
          values.add(value.toString());
        } else if (value is DateTime) {
          values.add("'${_formatDateTime(value)}'");
        } else {
          values.add("'$value'");
        }
      }

      final sql = 'INSERT OR REPLACE INTO "$table" (${columns.join(', ')}) VALUES (${values.join(', ')})';
      final sqlPtr = sql.toNativeUtf8();
      final errorPtr = calloc<Int32>();

      try {
        final result = _bindings.databaseExecute(_db, sqlPtr, errorPtr);
        if (result == nullptr && errorPtr.value != PSR_OK) {
          final errorMsg = errorPtr.value == PSR_ERROR_QUERY
              ? 'One of the provided dimension entries already exists in the database. Please provide a new entry or update the existing one.'
              : 'Failed to insert time series row';
          throw DatabaseException(errorMsg);
        }
        if (result != nullptr) {
          _bindings.resultFree(result);
        }
      } finally {
        calloc.free(sqlPtr);
        calloc.free(errorPtr);
      }
    }
  }

  Map<String, dynamic> _resultToMap(PsrResult result) {
    final map = <String, dynamic>{};
    final colCount = _bindings.resultColumnCount(result);
    final rowCount = _bindings.resultRowCount(result);

    if (rowCount == 0) return map;

    for (int col = 0; col < colCount; col++) {
      final colNamePtr = _bindings.resultColumnName(result, col);
      if (colNamePtr == nullptr) continue;
      final colName = colNamePtr.toDartString();

      if (_bindings.resultIsNull(result, 0, col) != 0) {
        map[colName] = null;
      } else {
        // Try to get as different types
        final intPtr = calloc<Int64>();
        if (_bindings.resultGetInt(result, 0, col, intPtr) == PSR_OK) {
          map[colName] = intPtr.value;
        } else {
          final doublePtr = calloc<Double>();
          if (_bindings.resultGetDouble(result, 0, col, doublePtr) == PSR_OK) {
            map[colName] = doublePtr.value;
          } else {
            final strPtr = _bindings.resultGetString(result, 0, col);
            if (strPtr != nullptr) {
              final str = strPtr.toDartString();
              // Check if it's a date
              if (_isDateColumn(colName) && str.isNotEmpty) {
                map[colName] = _parseDateTime(str);
              } else {
                map[colName] = str;
              }
            }
          }
          calloc.free(doublePtr);
        }
        calloc.free(intPtr);
      }
    }
    return map;
  }

  List<Map<String, dynamic>> _resultToListOfMaps(PsrResult result) {
    final list = <Map<String, dynamic>>[];
    final colCount = _bindings.resultColumnCount(result);
    final rowCount = _bindings.resultRowCount(result);

    final colNames = <String>[];
    for (int col = 0; col < colCount; col++) {
      final colNamePtr = _bindings.resultColumnName(result, col);
      colNames.add(colNamePtr != nullptr ? colNamePtr.toDartString() : '');
    }

    for (int row = 0; row < rowCount; row++) {
      final map = <String, dynamic>{};
      for (int col = 0; col < colCount; col++) {
        final colName = colNames[col];

        if (_bindings.resultIsNull(result, row, col) != 0) {
          map[colName] = null;
        } else {
          final intPtr = calloc<Int64>();
          if (_bindings.resultGetInt(result, row, col, intPtr) == PSR_OK) {
            map[colName] = intPtr.value;
          } else {
            final doublePtr = calloc<Double>();
            if (_bindings.resultGetDouble(result, row, col, doublePtr) == PSR_OK) {
              map[colName] = doublePtr.value;
            } else {
              final strPtr = _bindings.resultGetString(result, row, col);
              if (strPtr != nullptr) {
                final str = strPtr.toDartString();
                if (_isDateColumn(colName) && str.isNotEmpty) {
                  map[colName] = _parseDateTime(str);
                } else {
                  map[colName] = str;
                }
              }
            }
            calloc.free(doublePtr);
          }
          calloc.free(intPtr);
        }
      }
      list.add(map);
    }
    return list;
  }

  bool _isDateColumn(String colName) {
    final lower = colName.toLowerCase();
    return lower.contains('date') || lower.contains('time');
  }

  DateTime? _parseDateTime(String str) {
    try {
      return DateTime.parse(str);
    } catch (_) {
      return null;
    }
  }

  String _formatDateTime(DateTime dt) {
    return '${dt.year.toString().padLeft(4, '0')}-'
        '${dt.month.toString().padLeft(2, '0')}-'
        '${dt.day.toString().padLeft(2, '0')} '
        '${dt.hour.toString().padLeft(2, '0')}:'
        '${dt.minute.toString().padLeft(2, '0')}:'
        '${dt.second.toString().padLeft(2, '0')}';
  }
}
