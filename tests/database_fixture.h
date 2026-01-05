#ifndef PSR_TEST_DATABASE_FIXTURE_H
#define PSR_TEST_DATABASE_FIXTURE_H

#include <gtest/gtest.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class DatabaseFixture : public ::testing::Test {
protected:
    void SetUp() override { path = (fs::temp_directory_path() / "psr_test.db").string(); }

    void TearDown() override {
        if (fs::exists(path)) {
            fs::remove(path);
        }
    }

    std::string path;
};

#endif