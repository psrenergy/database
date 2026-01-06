#include "psr/migrations.h"

#include <filesystem>
#include "psr/migration.h"

namespace psr {

struct Migrations::Impl {
    std::vector<Migration> versions;

    ~Impl() {}
};

Migrations::Migrations() : impl_(std::make_unique<Impl>()) {}

Migrations::Migrations(const std::string& path) : impl_(std::make_unique<Impl>()) {
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto& dirname = entry.path().filename().string();
        try {
            std::size_t pos = 0;
            auto migration_version = std::stoll(dirname, &pos);
            if (pos == dirname.size() && migration_version > 0) {
                auto migration_path = entry.path().string();
                auto migration = Migration(migration_version, migration_path);
                impl_->versions.push_back(migration);
            }
        } catch (const std::exception&) {}
    }

    std::sort(impl_->versions.begin(), impl_->versions.end());
}

}  // namespace psr