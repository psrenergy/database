#ifndef PSR_MIGRATIONS_H
#define PSR_MIGRATIONS_H

#include "export.h"

#include <memory>
#include <string>

namespace psr {

class PSR_API Migrations {
public:
    Migrations();
    Migrations(const std::string& path);
    ~Migrations();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace psr

#endif  // PSR_MIGRATIONS_H
