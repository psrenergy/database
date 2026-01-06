#ifndef PSR_MIGRATION_H
#define PSR_MIGRATION_H

#include "export.h"

#include <memory>
#include <string>

namespace psr {

class PSR_API Migration {
public:
    Migration(int64_t version, const std::string& path);
    ~Migration();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace psr

#endif  // PSR_MIGRATION_H
