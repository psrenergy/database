#include "psr/c/database.h"
#include "psr/database.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <new>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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

// Split SQL into individual statements, respecting string literals
std::vector<std::string> split_sql_statements(const std::string& sql) {
    std::vector<std::string> statements;
    std::string current;
    bool in_string = false;
    char string_char = '\0';

    for (size_t i = 0; i < sql.size(); ++i) {
        char c = sql[i];

        // Handle string literals
        if ((c == '\'' || c == '"') && (i == 0 || sql[i - 1] != '\\')) {
            if (!in_string) {
                in_string = true;
                string_char = c;
            } else if (c == string_char) {
                in_string = false;
            }
            current += c;
        } else if (c == ';' && !in_string) {
            // End of statement
            std::string stmt = current;
            // Trim whitespace
            size_t start = stmt.find_first_not_of(" \t\n\r");
            size_t end = stmt.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                stmt = stmt.substr(start, end - start + 1);
                if (!stmt.empty()) {
                    statements.push_back(stmt);
                }
            }
            current.clear();
        } else {
            current += c;
        }
    }

    // Handle last statement if no trailing semicolon
    std::string stmt = current;
    size_t start = stmt.find_first_not_of(" \t\n\r");
    size_t end = stmt.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
        stmt = stmt.substr(start, end - start + 1);
        if (!stmt.empty()) {
            statements.push_back(stmt);
        }
    }

    return statements;
}

// Helper: Convert string to uppercase
std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}

// Helper: Convert string to lowercase
std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

// Helper: Trim whitespace from both ends
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

// Helper: Normalize whitespace in a string (multiple spaces -> single space)
std::string normalize_whitespace(const std::string& s) {
    std::string result;
    bool in_space = false;
    for (char c : s) {
        if (std::isspace(c)) {
            if (!in_space) {
                result += ' ';
                in_space = true;
            }
        } else {
            result += c;
            in_space = false;
        }
    }
    return trim(result);
}

// Extract column names from a table definition (skip FK, PK, UNIQUE, CHECK, CONSTRAINT)
std::vector<std::string> extract_columns(const std::string& table_def) {
    std::vector<std::string> columns;
    std::string current;
    int paren_depth = 0;

    // Split by comma, respecting parentheses
    for (char c : table_def) {
        if (c == '(') {
            paren_depth++;
            current += c;
        } else if (c == ')') {
            paren_depth--;
            current += c;
        } else if (c == ',' && paren_depth == 0) {
            std::string line = trim(current);
            current.clear();

            // Skip constraints
            std::string upper_line = to_upper(line);
            if (upper_line.find("FOREIGN KEY") == 0 || upper_line.find("PRIMARY KEY") == 0 ||
                upper_line.find("UNIQUE") == 0 || upper_line.find("CHECK") == 0 ||
                upper_line.find("CONSTRAINT") == 0) {
                continue;
            }

            // Extract column name (first word)
            size_t space_pos = line.find_first_of(" \t");
            if (space_pos != std::string::npos) {
                std::string col_name = to_lower(line.substr(0, space_pos));
                // Skip standard columns
                if (col_name != "id" && col_name != "vector_index" && col_name != "label") {
                    columns.push_back(col_name);
                }
            }
        } else {
            current += c;
        }
    }

    // Handle last part
    std::string line = trim(current);
    if (!line.empty()) {
        std::string upper_line = to_upper(line);
        if (upper_line.find("FOREIGN KEY") != 0 && upper_line.find("PRIMARY KEY") != 0 &&
            upper_line.find("UNIQUE") != 0 && upper_line.find("CHECK") != 0 && upper_line.find("CONSTRAINT") != 0) {
            size_t space_pos = line.find_first_of(" \t");
            if (space_pos != std::string::npos) {
                std::string col_name = to_lower(line.substr(0, space_pos));
                if (col_name != "id" && col_name != "vector_index" && col_name != "label") {
                    columns.push_back(col_name);
                }
            }
        }
    }

    return columns;
}

// Validate foreign key actions
// Invalid: ON DELETE CASCADE with non-CASCADE ON UPDATE
void validate_foreign_key_actions(const std::string& sql) {
    std::regex fk_pattern(
        R"(FOREIGN\s+KEY\s*\([^)]+\)\s+REFERENCES\s+\w+\s*\(\s*\w+\s*\)\s+ON\s+DELETE\s+(CASCADE|SET\s+NULL|SET\s+DEFAULT|RESTRICT|NO\s+ACTION)\s+ON\s+UPDATE\s+(CASCADE|SET\s+NULL|SET\s+DEFAULT|RESTRICT|NO\s+ACTION))",
        std::regex::icase);

    std::sregex_iterator begin(sql.begin(), sql.end(), fk_pattern);
    std::sregex_iterator end;

    for (auto it = begin; it != end; ++it) {
        std::string delete_action = to_upper(normalize_whitespace((*it)[1].str()));
        std::string update_action = to_upper(normalize_whitespace((*it)[2].str()));

        if (delete_action == "CASCADE" && update_action != "CASCADE") {
            throw std::runtime_error("Invalid foreign key actions: ON DELETE " + delete_action + " with ON UPDATE " +
                                     update_action + ". When ON DELETE is CASCADE, ON UPDATE must also be CASCADE.");
        }
    }
}

// Validate that vector tables have vector_index column
void validate_vector_tables(const std::string& sql) {
    std::regex table_pattern(R"(CREATE\s+TABLE\s+(\w+_vector_\w+)\s*\(([^;]+)\))", std::regex::icase);

    std::sregex_iterator begin(sql.begin(), sql.end(), table_pattern);
    std::sregex_iterator end;

    for (auto it = begin; it != end; ++it) {
        std::string table_name = (*it)[1].str();
        std::string table_def = (*it)[2].str();

        std::regex vector_index_pattern(R"(vector_index\s+INTEGER)", std::regex::icase);
        if (!std::regex_search(table_def, vector_index_pattern)) {
            throw std::runtime_error("Vector table '" + table_name + "' must have a 'vector_index INTEGER' column.");
        }
    }
}

// Validate no duplicate attributes between main tables and their vector/set tables
void validate_no_duplicated_attributes(const std::string& sql) {
    std::regex table_pattern(R"(CREATE\s+TABLE\s+(\w+)\s*\(([^;]+)\))", std::regex::icase);
    std::map<std::string, std::vector<std::string>> tables;

    std::sregex_iterator begin(sql.begin(), sql.end(), table_pattern);
    std::sregex_iterator end;

    for (auto it = begin; it != end; ++it) {
        std::string table_name = (*it)[1].str();
        std::string table_def = (*it)[2].str();
        tables[table_name] = extract_columns(table_def);
    }

    // Check for duplicates between main tables and their vector/set tables
    std::regex aux_table_pattern(R"(^(\w+)_(vector|set)_)", std::regex::icase);
    for (const auto& [table_name, columns] : tables) {
        std::smatch match;
        if (std::regex_search(table_name, match, aux_table_pattern)) {
            std::string parent_name = match[1].str();
            auto parent_it = tables.find(parent_name);
            if (parent_it != tables.end()) {
                std::set<std::string> parent_cols(parent_it->second.begin(), parent_it->second.end());
                for (const auto& col : columns) {
                    if (parent_cols.count(col)) {
                        throw std::runtime_error("Duplicated attribute '" + col + "' found in both '" + parent_name +
                                                 "' and '" + table_name + "'.");
                    }
                }
            }
        }
    }
}

// Validate that collection tables have required label column
void validate_collection_tables(const std::string& sql) {
    std::regex table_pattern(R"(CREATE\s+TABLE\s+(\w+)\s*\(([^;]+)\))", std::regex::icase);

    std::sregex_iterator begin(sql.begin(), sql.end(), table_pattern);
    std::sregex_iterator end;

    for (auto it = begin; it != end; ++it) {
        std::string table_name = (*it)[1].str();
        std::string table_def = (*it)[2].str();

        // Skip auxiliary tables (vector, set, time_series)
        std::regex aux_pattern(R"(_(vector|set|time_series)_)", std::regex::icase);
        if (std::regex_search(table_name, aux_pattern)) {
            continue;
        }

        // Skip Configuration tables
        if (to_lower(table_name) == "configuration") {
            continue;
        }

        // Skip tables that end with "_files" (time series file tables)
        std::regex files_pattern(R"(_files$)", std::regex::icase);
        if (std::regex_search(table_name, files_pattern)) {
            continue;
        }

        // Check for required 'label' column
        std::regex label_pattern(R"(\blabel\b)", std::regex::icase);
        if (!std::regex_search(table_def, label_pattern)) {
            throw std::runtime_error("Collection table '" + table_name + "' must have a 'label' column.");
        }
    }
}

// Main schema validation function
void validate_schema(const std::string& sql) {
    validate_foreign_key_actions(sql);
    validate_vector_tables(sql);
    validate_no_duplicated_attributes(sql);
    validate_collection_tables(sql);
}

}  // anonymous namespace

// Internal struct definitions
struct psr_database {
    psr::Database db;
    std::string last_error;
    psr_database(const std::string& path, psr::LogLevel level, bool read_only = false) : db(path, level, read_only) {}
    psr_database(psr::Database&& database) : db(std::move(database)) {}
};

struct psr_result {
    psr::Result result;
    // For manually constructed results
    std::vector<std::string> columns;
    std::vector<std::vector<psr::Value>> rows;
    bool manual_mode = false;

    explicit psr_result(psr::Result r) : result(std::move(r)), manual_mode(false) {}
    psr_result() : result({}, {}), manual_mode(true) {}
};

struct psr_time_series {
    psr::TimeSeries data;
};

struct psr_element {
    std::vector<std::pair<std::string, psr::Value>> fields;
    std::map<std::string, psr::TimeSeries> time_series;
};

struct psr_string_array {
    std::vector<std::string> strings;
};

extern "C" {

// Database functions

PSR_C_API psr_database_t* psr_database_open(const char* path, psr_log_level_t console_level, int read_only, psr_error_t* error) {
    if (!path) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto* db = new psr_database(path, to_cpp_log_level(console_level), read_only != 0);
        if (error)
            *error = PSR_OK;
        return db;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception&) {
        if (error)
            *error = PSR_ERROR_DATABASE;
        return nullptr;
    }
}

PSR_C_API psr_database_t* psr_database_from_schema(const char* db_path, const char* schema_path,
                                                   psr_log_level_t console_level, psr_error_t* error) {
    if (!db_path || !schema_path) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto database = psr::Database::from_schema(db_path, schema_path, to_cpp_log_level(console_level));
        auto* db = new psr_database(std::move(database));
        if (error)
            *error = PSR_OK;
        return db;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception&) {
        if (error)
            *error = PSR_ERROR_MIGRATION;
        return nullptr;
    }
}

// Custom exception for schema validation errors
class schema_validation_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

PSR_C_API psr_database_t* psr_database_from_sql_file(const char* db_path, const char* sql_file_path,
                                                      psr_log_level_t console_level, psr_error_t* error) {
    if (!db_path || !sql_file_path) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    psr_database* db = nullptr;
    try {
        // Check if SQL file exists
        if (!std::filesystem::exists(sql_file_path)) {
            if (error)
                *error = PSR_ERROR_INVALID_ARGUMENT;
            return nullptr;
        }

        // Read SQL file content
        std::ifstream file(sql_file_path);
        if (!file) {
            if (error)
                *error = PSR_ERROR_INVALID_ARGUMENT;
            return nullptr;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sql_content = buffer.str();

        // Validate schema before creating database
        try {
            validate_schema(sql_content);
        } catch (const std::runtime_error& e) {
            throw schema_validation_error(e.what());
        }

        // Create database
        db = new psr_database(db_path, to_cpp_log_level(console_level));

        // Split and execute each statement
        auto statements = split_sql_statements(sql_content);
        for (const auto& stmt : statements) {
            db->db.execute(stmt);
        }

        if (error)
            *error = PSR_OK;
        return db;
    } catch (const std::bad_alloc&) {
        delete db;
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const schema_validation_error&) {
        delete db;
        if (error)
            *error = PSR_ERROR_SCHEMA_VALIDATION;
        return nullptr;
    } catch (const std::exception&) {
        delete db;
        if (error)
            *error = PSR_ERROR_QUERY;
        return nullptr;
    }
}

PSR_C_API void psr_database_close(psr_database_t* db) {
    delete db;
}

PSR_C_API int psr_database_is_open(psr_database_t* db) {
    if (!db)
        return 0;
    return db->db.is_open() ? 1 : 0;
}

PSR_C_API psr_result_t* psr_database_execute(psr_database_t* db, const char* sql, psr_error_t* error) {
    if (!db) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (!sql) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto result = db->db.execute(sql);
        auto* res = new psr_result(std::move(result));
        if (error)
            *error = PSR_OK;
        return res;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        db->last_error = "Out of memory";
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API int64_t psr_database_last_insert_rowid(psr_database_t* db) {
    if (!db)
        return 0;
    return db->db.last_insert_rowid();
}

PSR_C_API int psr_database_changes(psr_database_t* db) {
    if (!db)
        return 0;
    return db->db.changes();
}

PSR_C_API psr_error_t psr_database_begin_transaction(psr_database_t* db) {
    if (!db)
        return PSR_ERROR_INVALID_ARGUMENT;
    try {
        db->db.begin_transaction();
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_commit(psr_database_t* db) {
    if (!db)
        return PSR_ERROR_INVALID_ARGUMENT;
    try {
        db->db.commit();
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_rollback(psr_database_t* db) {
    if (!db)
        return PSR_ERROR_INVALID_ARGUMENT;
    try {
        db->db.rollback();
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API const char* psr_database_error_message(psr_database_t* db) {
    if (!db)
        return "Invalid database handle";
    if (!db->last_error.empty()) {
        return db->last_error.c_str();
    }
    return db->db.error_message().c_str();
}

// Migration functions

PSR_C_API int64_t psr_database_current_version(psr_database_t* db) {
    if (!db)
        return 0;
    try {
        return db->db.current_version();
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return 0;
    }
}

PSR_C_API psr_error_t psr_database_set_version(psr_database_t* db, int64_t version) {
    if (!db)
        return PSR_ERROR_INVALID_ARGUMENT;
    try {
        db->db.set_version(version);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_migrate_up(psr_database_t* db) {
    if (!db)
        return PSR_ERROR_INVALID_ARGUMENT;
    try {
        db->db.migrate_up();
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_MIGRATION;
    }
}

// Element builder functions

PSR_C_API psr_element_t* psr_element_create(void) {
    try {
        return new psr_element();
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

PSR_C_API void psr_element_free(psr_element_t* elem) {
    delete elem;
}

PSR_C_API psr_error_t psr_element_set_null(psr_element_t* elem, const char* column) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    elem->fields.emplace_back(column, nullptr);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_int(psr_element_t* elem, const char* column, int64_t value) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    elem->fields.emplace_back(column, value);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_double(psr_element_t* elem, const char* column, double value) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    elem->fields.emplace_back(column, value);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_string(psr_element_t* elem, const char* column, const char* value) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    if (value) {
        elem->fields.emplace_back(column, std::string(value));
    } else {
        elem->fields.emplace_back(column, nullptr);
    }
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_blob(psr_element_t* elem, const char* column, const uint8_t* data, size_t size) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    if (data && size > 0) {
        elem->fields.emplace_back(column, std::vector<uint8_t>(data, data + size));
    } else {
        elem->fields.emplace_back(column, std::vector<uint8_t>());
    }
    return PSR_OK;
}

// Vector setters

PSR_C_API psr_error_t psr_element_set_int_array(psr_element_t* elem, const char* column, const int64_t* values,
                                                size_t count) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<int64_t> vec;
    if (values && count > 0) {
        vec.assign(values, values + count);
    }
    elem->fields.emplace_back(column, std::move(vec));
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_double_array(psr_element_t* elem, const char* column, const double* values,
                                                   size_t count) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<double> vec;
    if (values && count > 0) {
        vec.assign(values, values + count);
    }
    elem->fields.emplace_back(column, std::move(vec));
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_set_string_array(psr_element_t* elem, const char* column, const char** values,
                                                   size_t count) {
    if (!elem || !column)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<std::string> vec;
    if (values && count > 0) {
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            vec.push_back(values[i] ? values[i] : "");
        }
    }
    elem->fields.emplace_back(column, std::move(vec));
    return PSR_OK;
}

// Time series functions

PSR_C_API psr_time_series_t* psr_time_series_create(void) {
    try {
        return new psr_time_series();
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

PSR_C_API void psr_time_series_free(psr_time_series_t* ts) {
    delete ts;
}

PSR_C_API psr_error_t psr_time_series_add_int_column(psr_time_series_t* ts, const char* name, const int64_t* values,
                                                     size_t count) {
    if (!ts || !name)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<psr::Value> vec;
    if (values && count > 0) {
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            vec.push_back(values[i]);
        }
    }
    ts->data.columns[name] = std::move(vec);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_time_series_add_double_column(psr_time_series_t* ts, const char* name, const double* values,
                                                        size_t count) {
    if (!ts || !name)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<psr::Value> vec;
    if (values && count > 0) {
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            vec.push_back(values[i]);
        }
    }
    ts->data.columns[name] = std::move(vec);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_time_series_add_string_column(psr_time_series_t* ts, const char* name, const char** values,
                                                        size_t count) {
    if (!ts || !name)
        return PSR_ERROR_INVALID_ARGUMENT;
    std::vector<psr::Value> vec;
    if (values && count > 0) {
        vec.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            vec.push_back(std::string(values[i] ? values[i] : ""));
        }
    }
    ts->data.columns[name] = std::move(vec);
    return PSR_OK;
}

PSR_C_API psr_error_t psr_element_add_time_series(psr_element_t* elem, const char* group, psr_time_series_t* ts) {
    if (!elem || !group || !ts)
        return PSR_ERROR_INVALID_ARGUMENT;
    elem->time_series[group] = ts->data;
    return PSR_OK;
}

PSR_C_API int64_t psr_database_create_element(psr_database_t* db, const char* table, psr_element_t* elem,
                                              psr_error_t* error) {
    if (!db || !table || !elem) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return 0;
    }

    try {
        int64_t rowid;
        if (elem->time_series.empty()) {
            rowid = db->db.create_element(table, elem->fields);
        } else {
            rowid = db->db.create_element(table, elem->fields, elem->time_series);
        }
        if (error)
            *error = PSR_OK;
        return rowid;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        if (error)
            *error = PSR_ERROR_QUERY;
        return 0;
    }
}

PSR_C_API int64_t psr_database_get_element_id(psr_database_t* db, const char* collection, const char* label,
                                              psr_error_t* error) {
    if (!db || !collection || !label) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return 0;
    }

    try {
        int64_t id = db->db.get_element_id(collection, label);
        if (error)
            *error = PSR_OK;
        return id;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        if (error)
            *error = PSR_ERROR_QUERY;
        return 0;
    }
}

// Relation update functions

PSR_C_API psr_error_t psr_database_set_scalar_relation(psr_database_t* db, const char* collection,
                                                       const char* target_collection, const char* parent_label,
                                                       const char* child_label, const char* relation_name) {
    if (!db || !collection || !target_collection || !parent_label || !child_label || !relation_name) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        db->db.set_scalar_relation(collection, target_collection, parent_label, child_label, relation_name);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_set_vector_relation(psr_database_t* db, const char* collection,
                                                       const char* target_collection, const char* parent_label,
                                                       const char** child_labels, size_t count,
                                                       const char* relation_name) {
    if (!db || !collection || !target_collection || !parent_label || !relation_name) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        std::vector<std::string> labels;
        if (child_labels && count > 0) {
            labels.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                labels.push_back(child_labels[i] ? child_labels[i] : "");
            }
        }
        db->db.set_vector_relation(collection, target_collection, parent_label, labels, relation_name);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_set_vector_relation_by_id(psr_database_t* db, const char* collection,
                                                             const char* target_collection, int64_t parent_id,
                                                             const int64_t* child_ids, size_t count,
                                                             const char* relation_name) {
    if (!db || !collection || !target_collection || !relation_name) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        std::vector<int64_t> ids;
        if (child_ids && count > 0) {
            ids.assign(child_ids, child_ids + count);
        }
        db->db.set_vector_relation(collection, target_collection, parent_id, ids, relation_name);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_set_set_relation(psr_database_t* db, const char* collection,
                                                    const char* target_collection, const char* parent_label,
                                                    const char** child_labels, size_t count,
                                                    const char* relation_name) {
    if (!db || !collection || !target_collection || !parent_label || !relation_name) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        std::vector<std::string> labels;
        if (child_labels && count > 0) {
            labels.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                labels.push_back(child_labels[i] ? child_labels[i] : "");
            }
        }
        db->db.set_set_relation(collection, target_collection, parent_label, labels, relation_name);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

// Parameter update functions

PSR_C_API psr_error_t psr_database_update_scalar_parameter_int(psr_database_t* db, const char* collection,
                                                               const char* column, const char* label, int64_t value) {
    if (!db || !collection || !column || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        db->db.update_scalar_parameter(collection, column, label, value);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_update_scalar_parameter_double(psr_database_t* db, const char* collection,
                                                                  const char* column, const char* label, double value) {
    if (!db || !collection || !column || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        db->db.update_scalar_parameter(collection, column, label, value);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_update_scalar_parameter_string(psr_database_t* db, const char* collection,
                                                                  const char* column, const char* label,
                                                                  const char* value) {
    if (!db || !collection || !column || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        db->db.update_scalar_parameter(collection, column, label, value ? std::string(value) : std::string());
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_update_vector_parameters_double(psr_database_t* db, const char* collection,
                                                                   const char* column, const char* label,
                                                                   const double* values, size_t count) {
    if (!db || !collection || !column || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        std::vector<double> vec;
        if (values && count > 0) {
            vec.assign(values, values + count);
        }
        db->db.update_vector_parameters(collection, column, label, vec);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_update_set_parameters_double(psr_database_t* db, const char* collection,
                                                                const char* column, const char* label,
                                                                const double* values, size_t count) {
    if (!db || !collection || !column || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        std::vector<double> vec;
        if (values && count > 0) {
            vec.assign(values, values + count);
        }
        db->db.update_set_parameters(collection, column, label, vec);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

// Time series file functions

PSR_C_API psr_error_t psr_database_set_time_series_file(psr_database_t* db, const char* collection,
                                                        const char* parameter, const char* file_path) {
    if (!db || !collection || !parameter) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }
    try {
        db->db.set_time_series_file(collection, parameter, file_path ? file_path : "");
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API const char* psr_database_read_time_series_file(psr_database_t* db, const char* collection,
                                                         const char* parameter, psr_error_t* error) {
    if (!db || !collection || !parameter) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    try {
        db->last_error = db->db.read_time_series_file(collection, parameter);
        if (error)
            *error = PSR_OK;
        return db->last_error.c_str();
    } catch (const std::exception& e) {
        db->last_error = e.what();
        if (error)
            *error = PSR_ERROR_QUERY;
        return nullptr;
    }
}

PSR_C_API const char* psr_error_string(psr_error_t error) {
    switch (error) {
    case PSR_OK:
        return "Success";
    case PSR_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case PSR_ERROR_DATABASE:
        return "Database error";
    case PSR_ERROR_QUERY:
        return "Query error";
    case PSR_ERROR_NO_MEMORY:
        return "Out of memory";
    case PSR_ERROR_NOT_OPEN:
        return "Database not open";
    case PSR_ERROR_INDEX_OUT_OF_RANGE:
        return "Index out of range";
    case PSR_ERROR_MIGRATION:
        return "Migration error";
    case PSR_ERROR_SCHEMA_VALIDATION:
        return "Schema validation error";
    default:
        return "Unknown error";
    }
}

PSR_C_API const char* psr_version(void) {
    return PSR_VERSION;
}

// Result functions

PSR_C_API void psr_result_free(psr_result_t* result) {
    delete result;
}

PSR_C_API size_t psr_result_row_count(psr_result_t* result) {
    if (!result)
        return 0;
    if (result->manual_mode)
        return result->rows.size();
    return result->result.row_count();
}

PSR_C_API size_t psr_result_column_count(psr_result_t* result) {
    if (!result)
        return 0;
    if (result->manual_mode)
        return result->columns.size();
    return result->result.column_count();
}

PSR_C_API const char* psr_result_column_name(psr_result_t* result, size_t col) {
    if (!result)
        return nullptr;
    if (result->manual_mode) {
        if (col >= result->columns.size())
            return nullptr;
        return result->columns[col].c_str();
    }
    if (col >= result->result.column_count())
        return nullptr;
    return result->result.columns()[col].c_str();
}

PSR_C_API psr_value_type_t psr_result_get_type(psr_result_t* result, size_t row, size_t col) {
    if (!result)
        return PSR_TYPE_NULL;

    const psr::Value* value_ptr = nullptr;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size())
            return PSR_TYPE_NULL;
        value_ptr = &result->rows[row][col];
    } else {
        if (row >= result->result.row_count() || col >= result->result.column_count())
            return PSR_TYPE_NULL;
        value_ptr = &result->result[row][col];
    }

    const auto& value = *value_ptr;
    if (std::holds_alternative<std::nullptr_t>(value)) {
        return PSR_TYPE_NULL;
    } else if (std::holds_alternative<int64_t>(value)) {
        return PSR_TYPE_INTEGER;
    } else if (std::holds_alternative<double>(value)) {
        return PSR_TYPE_FLOAT;
    } else if (std::holds_alternative<std::string>(value)) {
        return PSR_TYPE_TEXT;
    } else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
        return PSR_TYPE_BLOB;
    }
    return PSR_TYPE_NULL;
}

PSR_C_API int psr_result_is_null(psr_result_t* result, size_t row, size_t col) {
    if (!result)
        return 1;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size())
            return 1;
        return std::holds_alternative<std::nullptr_t>(result->rows[row][col]) ? 1 : 0;
    }
    if (row >= result->result.row_count() || col >= result->result.column_count())
        return 1;
    return result->result[row].is_null(col) ? 1 : 0;
}

PSR_C_API psr_error_t psr_result_get_int(psr_result_t* result, size_t row, size_t col, int64_t* value) {
    if (!result || !value)
        return PSR_ERROR_INVALID_ARGUMENT;

    const psr::Value* value_ptr = nullptr;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size())
            return PSR_ERROR_INDEX_OUT_OF_RANGE;
        value_ptr = &result->rows[row][col];
    } else {
        if (row >= result->result.row_count() || col >= result->result.column_count())
            return PSR_ERROR_INDEX_OUT_OF_RANGE;
        value_ptr = &result->result[row][col];
    }

    if (const auto* v = std::get_if<int64_t>(value_ptr)) {
        *value = *v;
        return PSR_OK;
    }
    return PSR_ERROR_INVALID_ARGUMENT;
}

PSR_C_API psr_error_t psr_result_get_double(psr_result_t* result, size_t row, size_t col, double* value) {
    if (!result || !value)
        return PSR_ERROR_INVALID_ARGUMENT;

    const psr::Value* value_ptr = nullptr;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size())
            return PSR_ERROR_INDEX_OUT_OF_RANGE;
        value_ptr = &result->rows[row][col];
    } else {
        if (row >= result->result.row_count() || col >= result->result.column_count())
            return PSR_ERROR_INDEX_OUT_OF_RANGE;
        value_ptr = &result->result[row][col];
    }

    if (const auto* v = std::get_if<double>(value_ptr)) {
        *value = *v;
        return PSR_OK;
    }
    return PSR_ERROR_INVALID_ARGUMENT;
}

PSR_C_API const char* psr_result_get_string(psr_result_t* result, size_t row, size_t col) {
    if (!result)
        return nullptr;

    const psr::Value* value_ptr = nullptr;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size())
            return nullptr;
        value_ptr = &result->rows[row][col];
    } else {
        if (row >= result->result.row_count() || col >= result->result.column_count())
            return nullptr;
        value_ptr = &result->result[row][col];
    }

    if (const auto* str = std::get_if<std::string>(value_ptr)) {
        return str->c_str();
    }
    return nullptr;
}

PSR_C_API const uint8_t* psr_result_get_blob(psr_result_t* result, size_t row, size_t col, size_t* size) {
    if (!result) {
        if (size)
            *size = 0;
        return nullptr;
    }

    const psr::Value* value_ptr = nullptr;
    if (result->manual_mode) {
        if (row >= result->rows.size() || col >= result->columns.size()) {
            if (size)
                *size = 0;
            return nullptr;
        }
        value_ptr = &result->rows[row][col];
    } else {
        if (row >= result->result.row_count() || col >= result->result.column_count()) {
            if (size)
                *size = 0;
            return nullptr;
        }
        value_ptr = &result->result[row][col];
    }

    if (const auto* blob = std::get_if<std::vector<uint8_t>>(value_ptr)) {
        if (size)
            *size = blob->size();
        return blob->data();
    }
    if (size)
        *size = 0;
    return nullptr;
}

// String array functions

PSR_C_API size_t psr_string_array_count(psr_string_array_t* arr) {
    if (!arr)
        return 0;
    return arr->strings.size();
}

PSR_C_API const char* psr_string_array_get(psr_string_array_t* arr, size_t index) {
    if (!arr || index >= arr->strings.size())
        return nullptr;
    return arr->strings[index].c_str();
}

PSR_C_API void psr_string_array_free(psr_string_array_t* arr) {
    delete arr;
}

// Database comparison functions

PSR_C_API psr_string_array_t* psr_database_compare_scalar_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_scalar_parameters(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_vector_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_vector_parameters(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_set_parameters(psr_database_t* db1, psr_database_t* db2,
                                                                  const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_set_parameters(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_scalar_relations(psr_database_t* db1, psr_database_t* db2,
                                                                    const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_scalar_relations(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_vector_relations(psr_database_t* db1, psr_database_t* db2,
                                                                    const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_vector_relations(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_set_relations(psr_database_t* db1, psr_database_t* db2,
                                                                 const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_set_relations(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_time_series(psr_database_t* db1, psr_database_t* db2,
                                                               const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_time_series(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_time_series_files(psr_database_t* db1, psr_database_t* db2,
                                                                     const char* collection, psr_error_t* error) {
    if (!db1 || !db2 || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_time_series_files(db2->db, collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_compare_databases(psr_database_t* db1, psr_database_t* db2,
                                                             psr_error_t* error) {
    if (!db1 || !db2) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto diffs = db1->db.compare_databases(db2->db);
        auto* arr = new psr_string_array();
        arr->strings = std::move(diffs);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db1->last_error = e.what();
        return nullptr;
    }
}

// ============================================================================
// Read operations
// ============================================================================

PSR_C_API psr_result_t* psr_database_read_scalar_parameters(psr_database_t* db, const char* collection,
                                                             const char* column, psr_error_t* error) {
    if (!db || !collection || !column) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto values = db->db.read_scalar_parameters(collection, column);
        auto* result = new psr_result();

        // Store column name
        result->columns.push_back(column);

        // Convert values to rows
        for (auto& val : values) {
            std::vector<psr::Value> row_values;
            row_values.push_back(std::move(val));
            result->rows.push_back(std::move(row_values));
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_vector_parameters(psr_database_t* db, const char* collection,
                                                             const char* column, psr_error_t* error) {
    if (!db || !collection || !column) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto vectors = db->db.read_vector_parameters(collection, column);
        auto* result = new psr_result();

        // Store columns: element_index, vector_index, column value
        result->columns.push_back("element_index");
        result->columns.push_back("vector_index");
        result->columns.push_back(column);

        // Flatten vectors into rows
        int64_t element_idx = 0;
        for (const auto& vec : vectors) {
            int64_t vec_idx = 0;
            for (const auto& val : vec) {
                std::vector<psr::Value> row_values;
                row_values.push_back(element_idx);
                row_values.push_back(vec_idx);
                row_values.push_back(val);
                result->rows.push_back(std::move(row_values));
                vec_idx++;
            }
            element_idx++;
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_set_parameters(psr_database_t* db, const char* collection,
                                                          const char* column, psr_error_t* error) {
    if (!db || !collection || !column) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto sets = db->db.read_set_parameters(collection, column);
        auto* result = new psr_result();

        // Store columns: element_index, set_index, column value
        result->columns.push_back("element_index");
        result->columns.push_back("set_index");
        result->columns.push_back(column);

        // Flatten sets into rows
        int64_t element_idx = 0;
        for (const auto& set : sets) {
            int64_t set_idx = 0;
            for (const auto& val : set) {
                std::vector<psr::Value> row_values;
                row_values.push_back(element_idx);
                row_values.push_back(set_idx);
                row_values.push_back(val);
                result->rows.push_back(std::move(row_values));
                set_idx++;
            }
            element_idx++;
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_scalar_relations(psr_database_t* db, const char* collection,
                                                            const char* target_collection, const char* relation_name,
                                                            psr_error_t* error) {
    if (!db || !collection || !target_collection || !relation_name) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto labels = db->db.read_scalar_relations(collection, target_collection, relation_name);
        auto* result = new psr_result();

        // Store column name
        result->columns.push_back(relation_name);

        // Convert labels to rows
        for (const auto& label : labels) {
            std::vector<psr::Value> row_values;
            row_values.push_back(label);
            result->rows.push_back(std::move(row_values));
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_vector_relations(psr_database_t* db, const char* collection,
                                                            const char* target_collection, const char* relation_name,
                                                            psr_error_t* error) {
    if (!db || !collection || !target_collection || !relation_name) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto vectors = db->db.read_vector_relations(collection, target_collection, relation_name);
        auto* result = new psr_result();

        // Store columns: element_index, vector_index, relation_name
        result->columns.push_back("element_index");
        result->columns.push_back("vector_index");
        result->columns.push_back(relation_name);

        // Flatten vectors into rows
        int64_t element_idx = 0;
        for (const auto& vec : vectors) {
            int64_t vec_idx = 0;
            for (const auto& label : vec) {
                std::vector<psr::Value> row_values;
                row_values.push_back(element_idx);
                row_values.push_back(vec_idx);
                row_values.push_back(label);
                result->rows.push_back(std::move(row_values));
                vec_idx++;
            }
            element_idx++;
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_set_relations(psr_database_t* db, const char* collection,
                                                         const char* target_collection, const char* relation_name,
                                                         psr_error_t* error) {
    if (!db || !collection || !target_collection || !relation_name) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto sets = db->db.read_set_relations(collection, target_collection, relation_name);
        auto* result = new psr_result();

        // Store columns: element_index, set_index, relation_name
        result->columns.push_back("element_index");
        result->columns.push_back("set_index");
        result->columns.push_back(relation_name);

        // Flatten sets into rows
        int64_t element_idx = 0;
        for (const auto& set : sets) {
            int64_t set_idx = 0;
            for (const auto& label : set) {
                std::vector<psr::Value> row_values;
                row_values.push_back(element_idx);
                row_values.push_back(set_idx);
                row_values.push_back(label);
                result->rows.push_back(std::move(row_values));
                set_idx++;
            }
            element_idx++;
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_element_scalar_attributes(psr_database_t* db, const char* collection,
                                                                     const char* label, psr_error_t* error) {
    if (!db || !collection || !label) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto attrs = db->db.read_element_scalar_attributes(collection, label);
        auto* result = new psr_result();

        // Build columns and single row from map
        std::vector<psr::Value> row_values;
        for (const auto& [key, val] : attrs) {
            result->columns.push_back(key);
            row_values.push_back(val);
        }
        result->rows.push_back(std::move(row_values));

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_element_vector_group(psr_database_t* db, const char* collection,
                                                                const char* label, const char* group,
                                                                psr_error_t* error) {
    if (!db || !collection || !label || !group) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto rows = db->db.read_element_vector_group(collection, label, group);
        auto* result = new psr_result();

        // Extract columns from first row
        if (!rows.empty()) {
            for (const auto& [key, _] : rows[0]) {
                result->columns.push_back(key);
            }

            // Convert rows
            for (const auto& row_map : rows) {
                std::vector<psr::Value> row_values;
                for (const auto& col : result->columns) {
                    auto it = row_map.find(col);
                    if (it != row_map.end()) {
                        row_values.push_back(it->second);
                    } else {
                        row_values.push_back(nullptr);
                    }
                }
                result->rows.push_back(std::move(row_values));
            }
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_element_set_group(psr_database_t* db, const char* collection,
                                                             const char* label, const char* group,
                                                             psr_error_t* error) {
    if (!db || !collection || !label || !group) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto rows = db->db.read_element_set_group(collection, label, group);
        auto* result = new psr_result();

        // Extract columns from first row
        if (!rows.empty()) {
            for (const auto& [key, _] : rows[0]) {
                result->columns.push_back(key);
            }

            // Convert rows
            for (const auto& row_map : rows) {
                std::vector<psr::Value> row_values;
                for (const auto& col : result->columns) {
                    auto it = row_map.find(col);
                    if (it != row_map.end()) {
                        row_values.push_back(it->second);
                    } else {
                        row_values.push_back(nullptr);
                    }
                }
                result->rows.push_back(std::move(row_values));
            }
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_element_time_series_group(psr_database_t* db, const char* collection,
                                                                     const char* label, const char* group,
                                                                     psr_error_t* error) {
    if (!db || !collection || !label || !group) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto rows = db->db.read_element_time_series_group(collection, label, group);
        auto* result = new psr_result();

        // Extract columns from first row
        if (!rows.empty()) {
            for (const auto& [key, _] : rows[0]) {
                result->columns.push_back(key);
            }

            // Convert rows
            for (const auto& row_map : rows) {
                std::vector<psr::Value> row_values;
                for (const auto& col : result->columns) {
                    auto it = row_map.find(col);
                    if (it != row_map.end()) {
                        row_values.push_back(it->second);
                    } else {
                        row_values.push_back(nullptr);
                    }
                }
                result->rows.push_back(std::move(row_values));
            }
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_result_t* psr_database_read_time_series_table(psr_database_t* db, const char* collection,
                                                             const char* column, const char* label,
                                                             psr_error_t* error) {
    if (!db || !collection || !column || !label) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto cpp_result = db->db.read_time_series_table(collection, column, label);
        auto* result = new psr_result(std::move(cpp_result));

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

// ============================================================================
// Update operations
// ============================================================================

PSR_C_API psr_error_t psr_database_update_time_series_row(psr_database_t* db, const char* collection,
                                                          const char* column, const char* label, double value,
                                                          const char* date_time) {
    if (!db || !collection || !column || !label || !date_time) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }

    try {
        db->db.update_time_series_row(collection, column, label, value, date_time);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

// ============================================================================
// Delete operations
// ============================================================================

PSR_C_API psr_error_t psr_database_delete_time_series(psr_database_t* db, const char* collection, const char* group,
                                                       const char* label) {
    if (!db || !collection || !group || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }

    try {
        db->db.delete_time_series(collection, group, label);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_delete_element(psr_database_t* db, const char* collection, const char* label) {
    if (!db || !collection || !label) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }

    try {
        db->db.delete_element(collection, label);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

PSR_C_API psr_error_t psr_database_delete_element_by_id(psr_database_t* db, const char* collection, int64_t id) {
    if (!db || !collection) {
        return PSR_ERROR_INVALID_ARGUMENT;
    }

    try {
        db->db.delete_element(collection, id);
        return PSR_OK;
    } catch (const std::exception& e) {
        db->last_error = e.what();
        return PSR_ERROR_QUERY;
    }
}

// ============================================================================
// Introspection operations
// ============================================================================

PSR_C_API psr_result_t* psr_database_get_element_ids(psr_database_t* db, const char* collection, psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto ids = db->db.get_element_ids(collection);
        auto* result = new psr_result();

        result->columns.push_back("id");

        for (int64_t id : ids) {
            std::vector<psr::Value> row_values;
            row_values.push_back(id);
            result->rows.push_back(std::move(row_values));
        }

        if (error)
            *error = PSR_OK;
        return result;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_collections(psr_database_t* db, psr_error_t* error) {
    if (!db) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto collections = db->db.get_collections();
        auto* arr = new psr_string_array();
        arr->strings = std::move(collections);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_vector_groups(psr_database_t* db, const char* collection,
                                                              psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto groups = db->db.get_vector_groups(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(groups);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_set_groups(psr_database_t* db, const char* collection,
                                                           psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto groups = db->db.get_set_groups(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(groups);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_time_series_groups(psr_database_t* db, const char* collection,
                                                                   psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto groups = db->db.get_time_series_groups(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(groups);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

// Column type detection

PSR_C_API int psr_database_is_scalar_column(psr_database_t* db, const char* collection, const char* column) {
    if (!db || !collection || !column) {
        return 0;
    }
    return db->db.is_scalar_column(collection, column) ? 1 : 0;
}

PSR_C_API int psr_database_is_vector_column(psr_database_t* db, const char* collection, const char* column) {
    if (!db || !collection || !column) {
        return 0;
    }
    return db->db.is_vector_column(collection, column) ? 1 : 0;
}

PSR_C_API int psr_database_is_set_column(psr_database_t* db, const char* collection, const char* column) {
    if (!db || !collection || !column) {
        return 0;
    }
    return db->db.is_set_column(collection, column) ? 1 : 0;
}

// Table introspection

PSR_C_API psr_string_array_t* psr_database_get_table_columns(psr_database_t* db, const char* table,
                                                              psr_error_t* error) {
    if (!db || !table) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto columns = db->db.get_table_columns_public(table);
        auto* arr = new psr_string_array();
        arr->strings = std::move(columns);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_vector_tables(psr_database_t* db, const char* collection,
                                                              psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto tables = db->db.get_vector_tables_public(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(tables);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_set_tables(psr_database_t* db, const char* collection,
                                                           psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto tables = db->db.get_set_tables_public(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(tables);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

PSR_C_API psr_string_array_t* psr_database_get_time_series_tables(psr_database_t* db, const char* collection,
                                                                   psr_error_t* error) {
    if (!db || !collection) {
        if (error)
            *error = PSR_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    try {
        auto tables = db->db.get_time_series_tables_public(collection);
        auto* arr = new psr_string_array();
        arr->strings = std::move(tables);
        if (error)
            *error = PSR_OK;
        return arr;
    } catch (const std::bad_alloc&) {
        if (error)
            *error = PSR_ERROR_NO_MEMORY;
        return nullptr;
    } catch (const std::exception& e) {
        if (error)
            *error = PSR_ERROR_QUERY;
        db->last_error = e.what();
        return nullptr;
    }
}

}  // extern "C"
