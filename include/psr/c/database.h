#ifndef PSR_C_DATABASE_H
#define PSR_C_DATABASE_H

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
    PSR_ERROR_MIGRATION = -3,
} psr_error_t;

// Log levels for console output
typedef enum {
    PSR_LOG_DEBUG = 0,
    PSR_LOG_INFO = 1,
    PSR_LOG_WARN = 2,
    PSR_LOG_ERROR = 3,
    PSR_LOG_OFF = 4,
} psr_log_level_t;

// Database options
typedef struct {
    int read_only;
    psr_log_level_t console_level;
} psr_database_options_t;

// Returns default options
PSR_C_API psr_database_options_t psr_database_options_default(void);

// Opaque handle type
typedef struct psr_database psr_database_t;

// Database functions
PSR_C_API psr_database_t* psr_database_open(const char* path, const psr_database_options_t* options);
PSR_C_API psr_database_t*
psr_database_from_migrations(const char* db_path, const char* migrations_path, const psr_database_options_t* options);
PSR_C_API void psr_database_close(psr_database_t* db);
PSR_C_API int psr_database_is_healthy(psr_database_t* db);
PSR_C_API const char* psr_database_path(psr_database_t* db);

// Version and migration
PSR_C_API int64_t psr_database_current_version(psr_database_t* db);
PSR_C_API psr_error_t psr_database_set_version(psr_database_t* db, int64_t version);
PSR_C_API psr_error_t psr_database_migrate_up(psr_database_t* db, const char* migrations_path);

// Transaction management
PSR_C_API psr_error_t psr_database_begin_transaction(psr_database_t* db);
PSR_C_API psr_error_t psr_database_commit(psr_database_t* db);
PSR_C_API psr_error_t psr_database_rollback(psr_database_t* db);

// Utility functions
PSR_C_API const char* psr_error_string(psr_error_t error);
PSR_C_API const char* psr_version(void);

// Element handle
typedef struct psr_element psr_element_t;

// Element lifecycle
PSR_C_API psr_element_t* psr_element_create(void);
PSR_C_API void psr_element_destroy(psr_element_t* element);
PSR_C_API void psr_element_clear(psr_element_t* element);

// Element scalar setters
PSR_C_API psr_error_t psr_element_set_int(psr_element_t* element, const char* name, int64_t value);
PSR_C_API psr_error_t psr_element_set_double(psr_element_t* element, const char* name, double value);
PSR_C_API psr_error_t psr_element_set_string(psr_element_t* element, const char* name, const char* value);
PSR_C_API psr_error_t psr_element_set_null(psr_element_t* element, const char* name);

// Element vector setters
PSR_C_API psr_error_t psr_element_set_vector_int(psr_element_t* element, const char* name, const int64_t* values, size_t count);
PSR_C_API psr_error_t psr_element_set_vector_double(psr_element_t* element, const char* name, const double* values, size_t count);
PSR_C_API psr_error_t psr_element_set_vector_string(psr_element_t* element, const char* name, const char** values, size_t count);

// Element accessors
PSR_C_API int psr_element_has_scalars(psr_element_t* element);
PSR_C_API int psr_element_has_vectors(psr_element_t* element);
PSR_C_API size_t psr_element_scalar_count(psr_element_t* element);
PSR_C_API size_t psr_element_vector_count(psr_element_t* element);

#ifdef __cplusplus
}
#endif

#endif  // PSR_C_DATABASE_H
