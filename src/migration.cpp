#include "psr/migration.h"

#include <filesystem>

namespace psr {

namespace fs = std::filesystem;

struct Migration::Impl {
    const int64_t version;
    const fs::path path;

    ~Impl() {}
};

    Migration::Migration(int64_t version, const std::string& path) :
        impl_(std::make_unique<Impl>(Impl{version, fs::path(path)})) {}

}  // namespace psr