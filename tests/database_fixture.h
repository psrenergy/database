#ifndef PSR_TEST_DATABASE_FIXTURE_H
#define PSR_TEST_DATABASE_FIXTURE_H

#include <filesystem>
#include <gtest/gtest.h>
#include <string>

namespace fs = std::filesystem;

// TempFileFixture: Provides a temporary file path that is cleaned up after each test.
// Use this fixture for tests that need to create a database file on disk.
// For tests that only need :memory: databases, use TEST() without a fixture.
// For tests that need schema paths, use test_utils.h macros (SCHEMA_PATH, VALID_SCHEMA, etc).
class TempFileFixture : public ::testing::Test {
protected:
    void SetUp() override { path = (fs::temp_directory_path() / "psr_test.db").string(); }

    void TearDown() override {
        if (fs::exists(path)) {
            fs::remove(path);
        }
    }

    std::string path;
};

// Backwards compatibility alias
using DatabaseFixture = TempFileFixture;

#endif  // PSR_TEST_DATABASE_FIXTURE_H