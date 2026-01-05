#ifndef PSR_DATABASE_H
#define PSR_DATABASE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Platform-specific export macros
#ifdef _WIN32
#ifdef PSR_DATABASE_C_EXPORTS
#define PSR_C_API __declspec(dllexport)
#else
#define PSR_C_API __declspec(dllimport)
#endif
#else
#define PSR_C_API __attribute__((visibility("default")))
#endif

// Error codes
typedef enum {
    PSR_OK = 0,
    PSR_ERROR_INVALID_ARGUMENT = -1,
    PSR_ERROR_DATABASE = -2,
    PSR_ERROR_QUERY = -3,
    PSR_ERROR_NO_MEMORY = -4,
    PSR_ERROR_NOT_OPEN = -5,
    PSR_ERROR_INDEX_OUT_OF_RANGE = -6,
    PSR_ERROR_MIGRATION = -7,
    PSR_ERROR_SCHEMA_VALIDATION = -8,
} psr_error_t;

// Log levels for console output
typedef enum {
    PSR_LOG_DEBUG = 0,
    PSR_LOG_INFO = 1,
    PSR_LOG_WARN = 2,
    PSR_LOG_ERROR = 3,
    PSR_LOG_OFF = 4,
} psr_log_level_t;

// Value types for result columns
typedef enum {
    PSR_TYPE_NULL = 0,
    PSR_TYPE_INTEGER = 1,
    PSR_TYPE_FLOAT = 2,
    PSR_TYPE_TEXT = 3,
    PSR_TYPE_BLOB = 4,
} psr_value_type_t;

// Opaque handle types
typedef struct psr_database psr_database_t;
typedef struct psr_result psr_result_t;
typedef struct psr_element psr_element_t;
typedef struct psr_time_series psr_time_series_t;
typedef struct psr_string_array psr_string_array_t;

// Database functions
PSR_C_API psr_database_t* psr_database_open(const char* path, psr_log_level_t console_level, int read_only, psr_error_t* error);
PSR_C_API psr_database_t* psr_database_from_schema(const char* db_path, const char* schema_path,
                                                   psr_log_level_t console_level, psr_error_t* error);
PSR_C_API psr_database_t* psr_database_from_sql_file(const char* db_path, const char* sql_file_path,
                                                      psr_log_level_t console_level, psr_error_t* error);
PSR_C_API void psr_database_close(psr_database_t* db);
PSR_C_API int psr_database_is_open(psr_database_t* db);
PSR_C_API psr_result_t* psr_database_execute(psr_database_t* db, const char* sql, psr_error_t* error);

// Result accessor functions
PSR_C_API void psr_result_free(psr_result_t* result);
PSR_C_API size_t psr_result_row_count(psr_result_t* result);
PSR_C_API size_t psr_result_column_count(psr_result_t* result);
PSR_C_API const char* psr_result_column_name(psr_result_t* result, size_t col);
PSR_C_API psr_value_type_t psr_result_get_type(psr_result_t* result, size_t row, size_t col);
PSR_C_API int psr_result_is_null(psr_result_t* result, size_t row, size_t col);
PSR_C_API psr_error_t psr_result_get_int(psr_result_t* result, size_t row, size_t col, int64_t* value);
PSR_C_API psr_error_t psr_result_get_double(psr_result_t* result, size_t row, size_t col, double* value);
PSR_C_API const char* psr_result_get_string(psr_result_t* result, size_t row, size_t col);
PSR_C_API const uint8_t* psr_result_get_blob(psr_result_t* result, size_t row, size_t col, size_t* size);

PSR_C_API int64_t psr_database_last_insert_rowid(psr_database_t* db);
PSR_C_API int psr_database_changes(psr_database_t* db);
PSR_C_API psr_error_t psr_database_begin_transaction(psr_database_t* db);
PSR_C_API psr_error_t psr_database_commit(psr_database_t* db);
PSR_C_API psr_error_t psr_database_rollback(psr_database_t* db);
PSR_C_API const char* psr_database_error_message(psr_database_t* db);

// Migration functions
PSR_C_API int64_t psr_database_current_version(psr_database_t* db);
PSR_C_API psr_error_t psr_database_set_version(psr_database_t* db, int64_t version);
PSR_C_API psr_error_t psr_database_migrate_up(psr_database_t* db);

// Element builder functions (for dynamic row creation)
PSR_C_API psr_element_t* psr_element_create(void);
PSR_C_API void psr_element_free(psr_element_t* elem);
PSR_C_API psr_error_t psr_element_set_null(psr_element_t* elem, const char* column);
PSR_C_API psr_error_t psr_element_set_int(psr_element_t* elem, const char* column, int64_t value);
PSR_C_API psr_error_t psr_element_set_double(psr_element_t* elem, const char* column, double value);
PSR_C_API psr_error_t psr_element_set_string(psr_element_t* elem, const char* column, const char* value);
PSR_C_API psr_error_t psr_element_set_blob(psr_element_t* elem, const char* column, const uint8_t* data, size_t size);

// Vector setters for element builder
PSR_C_API psr_error_t psr_element_set_int_array(psr_element_t* elem, const char* column, const int64_t* values,
                                                size_t count);
PSR_C_API psr_error_t psr_element_set_double_array(psr_element_t* elem, const char* column, const double* values,
                                                   size_t count);
PSR_C_API psr_error_t psr_element_set_string_array(psr_element_t* elem, const char* column, const char** values,
                                                   size_t count);

// Time series functions
PSR_C_API psr_time_series_t* psr_time_series_create(void);
PSR_C_API void psr_time_series_free(psr_time_series_t* ts);
PSR_C_API psr_error_t psr_time_series_add_int_column(psr_time_series_t* ts, const char* name, const int64_t* values,
                                                     size_t count);
PSR_C_API psr_error_t psr_time_series_add_double_column(psr_time_series_t* ts, const char* name, const double* values,
                                                        size_t count);
PSR_C_API psr_error_t psr_time_series_add_string_column(psr_time_series_t* ts, const char* name, const char** values,
                                                        size_t count);
PSR_C_API psr_error_t psr_element_add_time_series(psr_element_t* elem, const char* group, psr_time_series_t* ts);

// Element creation
PSR_C_API int64_t psr_database_create_element(psr_database_t* db, const char* table, psr_element_t* elem,
                                              psr_error_t* error);

// Element lookup
PSR_C_API int64_t psr_database_get_element_id(psr_database_t* db, const char* collection, const char* label,
                                              psr_error_t* error);

// Relation updates
PSR_C_API psr_error_t psr_database_set_scalar_relation(psr_database_t* db, const char* collection,
                                                       const char* target_collection, const char* parent_label,
                                                       const char* child_label, const char* relation_name);

PSR_C_API psr_error_t psr_database_set_vector_relation(psr_database_t* db, const char* collection,
                                                       const char* target_collection, const char* parent_label,
                                                       const char** child_labels, size_t count,
                                                       const char* relation_name);

PSR_C_API psr_error_t psr_database_set_vector_relation_by_id(psr_database_t* db, const char* collection,
                                                             const char* target_collection, int64_t parent_id,
                                                             const int64_t* child_ids, size_t count,
                                                             const char* relation_name);

PSR_C_API psr_error_t psr_database_set_set_relation(psr_database_t* db, const char* collection,
                                                    const char* target_collection, const char* parent_label,
                                                    const char** child_labels, size_t count, const char* relation_name);

// Parameter updates
PSR_C_API psr_error_t psr_database_update_scalar_parameter_int(psr_database_t* db, const char* collection,
                                                               const char* column, const char* label, int64_t value);

PSR_C_API psr_error_t psr_database_update_scalar_parameter_double(psr_database_t* db, const char* collection,
                                                                  const char* column, const char* label, double value);

PSR_C_API psr_error_t psr_database_update_scalar_parameter_string(psr_database_t* db, const char* collection,
                                                                  const char* column, const char* label,
                                                                  const char* value);

PSR_C_API psr_error_t psr_database_update_vector_parameters_double(psr_database_t* db, const char* collection,
                                                                   const char* column, const char* label,
                                                                   const double* values, size_t count);

PSR_C_API psr_error_t psr_database_update_set_parameters_double(psr_database_t* db, const char* collection,
                                                                const char* column, const char* label,
                                                                const double* values, size_t count);

// Time series files
PSR_C_API psr_error_t psr_database_set_time_series_file(psr_database_t* db, const char* collection,
                                                        const char* parameter, const char* file_path);

PSR_C_API const char* psr_database_read_time_series_file(psr_database_t* db, const char* collection,
                                                         const char* parameter, psr_error_t* error);

// String array functions (for comparison results)
PSR_C_API size_t psr_string_array_count(psr_string_array_t* arr);
PSR_C_API const char* psr_string_array_get(psr_string_array_t* arr, size_t index);
PSR_C_API void psr_string_array_free(psr_string_array_t* arr);

// Database comparison functions
PSR_C_API psr_string_array_t* psr_database_compare_scalar_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_vector_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_set_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                  const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_scalar_relations(psr_database_t* db1, psr_database_t* db2,
                                                                    const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_vector_relations(psr_database_t* db1, psr_database_t* db2,
                                                                    const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_set_relations(psr_database_t* db1, psr_database_t* db2,
                                                                 const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_time_series(psr_database_t* db1, psr_database_t* db2,
                                                               const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_time_series_files(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_compare_databases(psr_database_t* db1, psr_database_t* db2,
                                                             psr_error_t* error);

// Read scalar parameters (returns result with one row per element)
PSR_C_API psr_result_t* psr_database_read_scalar_parameters(psr_database_t* db, const char* collection,
                                                             const char* column, psr_error_t* error);

// Read vector parameters (returns result with all values, grouped by element)
PSR_C_API psr_result_t* psr_database_read_vector_parameters(psr_database_t* db, const char* collection,
                                                             const char* column, psr_error_t* error);

// Read set parameters (returns result with all values, grouped by element)
PSR_C_API psr_result_t* psr_database_read_set_parameters(psr_database_t* db, const char* collection,
                                                          const char* column, psr_error_t* error);

// Read scalar relations (returns target labels for all elements)
PSR_C_API psr_result_t* psr_database_read_scalar_relations(psr_database_t* db, const char* collection,
                                                            const char* target_collection, const char* relation_name,
                                                            psr_error_t* error);

// Read vector relations (returns target labels grouped by element)
PSR_C_API psr_result_t* psr_database_read_vector_relations(psr_database_t* db, const char* collection,
                                                            const char* target_collection, const char* relation_name,
                                                            psr_error_t* error);

// Read set relations (returns target labels grouped by element)
PSR_C_API psr_result_t* psr_database_read_set_relations(psr_database_t* db, const char* collection,
                                                         const char* target_collection, const char* relation_name,
                                                         psr_error_t* error);

// Read element scalar attributes (returns all scalar columns for an element)
PSR_C_API psr_result_t* psr_database_read_element_scalar_attributes(psr_database_t* db, const char* collection,
                                                                     const char* label, psr_error_t* error);

// Read vector group for an element
PSR_C_API psr_result_t* psr_database_read_element_vector_group(psr_database_t* db, const char* collection,
                                                                const char* label, const char* group,
                                                                psr_error_t* error);

// Read set group for an element
PSR_C_API psr_result_t* psr_database_read_element_set_group(psr_database_t* db, const char* collection,
                                                             const char* label, const char* group, psr_error_t* error);

// Read time series group for an element
PSR_C_API psr_result_t* psr_database_read_element_time_series_group(psr_database_t* db, const char* collection,
                                                                     const char* label, const char* group,
                                                                     psr_error_t* error);

// Read time series table for a specific column/label
PSR_C_API psr_result_t* psr_database_read_time_series_table(psr_database_t* db, const char* collection,
                                                             const char* column, const char* label,
                                                             psr_error_t* error);

// Update time series row
PSR_C_API psr_error_t psr_database_update_time_series_row(psr_database_t* db, const char* collection,
                                                          const char* column, const char* label, double value,
                                                          const char* date_time);

// Delete time series data for an element
PSR_C_API psr_error_t psr_database_delete_time_series(psr_database_t* db, const char* collection, const char* group,
                                                       const char* label);

// Delete element by label
PSR_C_API psr_error_t psr_database_delete_element(psr_database_t* db, const char* collection, const char* label);

// Delete element by ID
PSR_C_API psr_error_t psr_database_delete_element_by_id(psr_database_t* db, const char* collection, int64_t id);

// Get element IDs in a collection
PSR_C_API psr_result_t* psr_database_get_element_ids(psr_database_t* db, const char* collection, psr_error_t* error);

// Get collections in the database
PSR_C_API psr_string_array_t* psr_database_get_collections(psr_database_t* db, psr_error_t* error);

// Get vector groups for a collection
PSR_C_API psr_string_array_t* psr_database_get_vector_groups(psr_database_t* db, const char* collection,
                                                              psr_error_t* error);

// Get set groups for a collection
PSR_C_API psr_string_array_t* psr_database_get_set_groups(psr_database_t* db, const char* collection,
                                                           psr_error_t* error);

// Get time series groups for a collection
PSR_C_API psr_string_array_t* psr_database_get_time_series_groups(psr_database_t* db, const char* collection,
                                                                   psr_error_t* error);

// Column type detection (returns 1 for true, 0 for false)
PSR_C_API int psr_database_is_scalar_column(psr_database_t* db, const char* collection, const char* column);
PSR_C_API int psr_database_is_vector_column(psr_database_t* db, const char* collection, const char* column);
PSR_C_API int psr_database_is_set_column(psr_database_t* db, const char* collection, const char* column);

// Table introspection
PSR_C_API psr_string_array_t* psr_database_get_table_columns(psr_database_t* db, const char* table,
                                                              psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_get_vector_tables(psr_database_t* db, const char* collection,
                                                              psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_get_set_tables(psr_database_t* db, const char* collection,
                                                           psr_error_t* error);
PSR_C_API psr_string_array_t* psr_database_get_time_series_tables(psr_database_t* db, const char* collection,
                                                                   psr_error_t* error);

// Utility functions
PSR_C_API const char* psr_error_string(psr_error_t error);
PSR_C_API const char* psr_version(void);

#ifdef __cplusplus
}
#endif

#endif  // PSR_DATABASE_H
