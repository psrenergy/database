#ifndef PSR_DATABASE_H
#define PSR_DATABASE_H

#include "export.h"
#include "psr/element.h"
#include "psr/log_level.h"
#include "psr/result.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace psr {

struct PSR_API DatabaseOptions {
    bool read_only = false;
    LogLevel console_level = LogLevel::info;
};

class PSR_API Database {
public:
    explicit Database(const std::string& path, const DatabaseOptions& options = DatabaseOptions());
    ~Database();

    // Non-copyable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Movable
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    static Database from_migrations(const std::string& db_path,
                                    const std::string& migrations_path,
                                    const DatabaseOptions& options = DatabaseOptions());

    static Database from_schema(const std::string& db_path,
                                const std::string& schema_path,
                                const DatabaseOptions& options = DatabaseOptions());
    bool is_healthy() const;

    Result execute(const std::string& sql, const std::vector<Value>& params = {});

    int64_t current_version() const;
    void set_version(int64_t version);

    void migrate_up(const std::string& migration_path);
    void apply_schema(const std::string& schema_path);

    // Element operations
    int64_t create_element(const std::string& collection, const Element& element);

    // Transaction management
    void begin_transaction();
    void commit();
    void rollback();

    const std::string& path() const;

    // Schema access for introspection
    const class Schema* schema() const;

    // Scalar reading
    std::vector<Value> read_scalar(const std::string& collection, const std::string& attribute) const;
    Value read_scalar_by_label(const std::string& collection, const std::string& attribute, const std::string& label) const;

    // Vector reading (from vector tables - ordered by vector_index)
    std::vector<std::vector<Value>> read_vector(const std::string& collection, const std::string& attribute) const;
    std::vector<Value>
    read_vector_by_label(const std::string& collection, const std::string& attribute, const std::string& label) const;

    // Set reading (from set tables - unordered)
    std::vector<std::vector<Value>> read_set(const std::string& collection, const std::string& attribute) const;
    std::vector<Value>
    read_set_by_label(const std::string& collection, const std::string& attribute, const std::string& label) const;

    // Element reading by ID
    std::vector<int64_t> get_element_ids(const std::string& collection) const;
    std::vector<std::pair<std::string, Value>> read_element_scalar_attributes(const std::string& collection,
                                                                               int64_t element_id) const;
    std::vector<std::pair<std::string, std::vector<Value>>>
    read_element_vector_group(const std::string& collection, int64_t element_id, const std::string& group) const;
    std::vector<std::pair<std::string, std::vector<Value>>>
    read_element_set_group(const std::string& collection, int64_t element_id, const std::string& group) const;
    std::vector<std::map<std::string, Value>>
    read_element_time_series_group(const std::string& collection,
                                   int64_t element_id,
                                   const std::string& group,
                                   const std::vector<std::string>& dimension_keys) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // Internal helper for executing raw SQL (for migrations)
    void execute_raw(const std::string& sql);
};

}  // namespace psr

#endif  // PSR_DATABASE_H
