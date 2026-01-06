#include "psr/c/database.h"
#include "psr/database.h"

#include <new>
#include <string>

namespace {

psr::LogLevel to_cpp_log_level(psr_log_level_t level) {
    switch (level) {
    case PSR_LOG_DEBUG:
        return psr::LogLevel::debug;
    case PSR_LOG_INFO:
        return psr::LogLevel::info;
    case PSR_LOG_WARN:
        return psr::LogLevel::warn;
    case PSR_LOG_ERROR:
        return psr::LogLevel::error;
    case PSR_LOG_OFF:
        return psr::LogLevel::off;
    default:
        return psr::LogLevel::info;
    }
}

psr::DatabaseOptions to_cpp_options(const psr_database_options_t* options) {
    psr::DatabaseOptions cpp_options;
    if (options) {
        cpp_options.read_only = options->read_only != 0;
        cpp_options.console_level = to_cpp_log_level(options->console_level);
    }
    return cpp_options;
}

}  // anonymous namespace

// Internal struct definition
struct psr_database {
    psr::Database db;
    psr_database(const std::string& path, const psr::DatabaseOptions& options) : db(path, options) {}
};

extern "C" {

PSR_C_API psr_database_options_t psr_database_options_default(void) {
    psr_database_options_t options;
    options.read_only = 0;
    options.console_level = PSR_LOG_INFO;
    return options;
}

PSR_C_API psr_database_t* psr_database_open(const char* path, const psr_database_options_t* options) {
    if (!path) {
        return nullptr;
    }

    try {
        auto cpp_options = to_cpp_options(options);
        return new psr_database(path, cpp_options);
    } catch (const std::bad_alloc&) {
        return nullptr;
    } catch (const std::exception&) {
        return nullptr;
    }
}

PSR_C_API void psr_database_close(psr_database_t* db) {
    if (db) {
        delete db;
    }
}

PSR_C_API int psr_database_is_healthy(psr_database_t* db) {
    if (!db)
        return 0;
    return db->db.is_healthy() ? 1 : 0;
}

PSR_C_API const char* psr_database_path(psr_database_t* db) {
    if (!db)
        return nullptr;
    return db->db.path().c_str();
}

PSR_C_API const char* psr_error_string(psr_error_t error) {
    switch (error) {
    case PSR_OK:
        return "Success";
    case PSR_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case PSR_ERROR_DATABASE:
        return "Database error";
    default:
        return "Unknown error";
    }
}

PSR_C_API const char* psr_version(void) {
    return PSR_VERSION;
}

}  // extern "C"
