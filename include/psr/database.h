#ifndef PSR_DATABASE_H
#define PSR_DATABASE_H

#include "export.h"
#include "psr/log_level.h"
#include "psr/result.h"

#include <memory>
#include <string>
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

    bool is_healthy() const;

    Result execute(const std::string& sql, const std::vector<Value>& params = {});

    int get_user_version() const;

    const std::string& path() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace psr

#endif  // PSR_DATABASE_H
