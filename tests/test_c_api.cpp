#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <psr/c/database.h>
#include <string>

namespace fs = std::filesystem;

class CApiTest : public ::testing::Test {
protected:
    void SetUp() override { test_db_path_ = (fs::temp_directory_path() / "psr_c_test.db").string(); }

    void TearDown() override {
        if (fs::exists(test_db_path_)) {
            fs::remove(test_db_path_);
        }
    }

    std::string test_db_path_;
};

TEST_F(CApiTest, OpenAndClose) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(test_db_path_.c_str(), PSR_LOG_OFF, 0, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(psr_database_is_open(db), 1);

    psr_database_close(db);
}

TEST_F(CApiTest, OpenInMemory) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);

    psr_database_close(db);
}

TEST_F(CApiTest, OpenNullPath) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(nullptr, PSR_LOG_OFF, 0, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);
}

TEST_F(CApiTest, ExecuteQuery) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* result = psr_database_execute(db, "CREATE TABLE test (id INTEGER PRIMARY KEY)", &error);
    EXPECT_EQ(error, PSR_OK);
    EXPECT_NE(result, nullptr);

    psr_result_free(result);
    psr_database_close(db);
}

TEST_F(CApiTest, InsertAndSelect) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* r1 =
        psr_database_execute(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)", &error);
    psr_result_free(r1);

    psr_result_t* r2 = psr_database_execute(db, "INSERT INTO users (name, age) VALUES ('Alice', 30)", &error);
    psr_result_free(r2);

    EXPECT_EQ(psr_database_last_insert_rowid(db), 1);
    EXPECT_EQ(psr_database_changes(db), 1);

    psr_result_t* result = psr_database_execute(db, "SELECT * FROM users", &error);
    ASSERT_NE(result, nullptr);

    EXPECT_EQ(psr_result_row_count(result), 1u);
    EXPECT_EQ(psr_result_column_count(result), 3u);

    EXPECT_STREQ(psr_result_column_name(result, 0), "id");
    EXPECT_STREQ(psr_result_column_name(result, 1), "name");
    EXPECT_STREQ(psr_result_column_name(result, 2), "age");

    int64_t id;
    EXPECT_EQ(psr_result_get_int(result, 0, 0, &id), PSR_OK);
    EXPECT_EQ(id, 1);

    const char* name = psr_result_get_string(result, 0, 1);
    EXPECT_STREQ(name, "Alice");

    int64_t age;
    EXPECT_EQ(psr_result_get_int(result, 0, 2, &age), PSR_OK);
    EXPECT_EQ(age, 30);

    psr_result_free(result);
    psr_database_close(db);
}

TEST_F(CApiTest, ValueTypes) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* r1 = psr_database_execute(db, "CREATE TABLE types (i INTEGER, f REAL, t TEXT, n INTEGER)", &error);
    psr_result_free(r1);

    psr_result_t* r2 = psr_database_execute(db, "INSERT INTO types VALUES (42, 3.14, 'hello', NULL)", &error);
    psr_result_free(r2);

    psr_result_t* result = psr_database_execute(db, "SELECT * FROM types", &error);
    ASSERT_NE(result, nullptr);

    EXPECT_EQ(psr_result_get_type(result, 0, 0), PSR_TYPE_INTEGER);
    EXPECT_EQ(psr_result_get_type(result, 0, 1), PSR_TYPE_FLOAT);
    EXPECT_EQ(psr_result_get_type(result, 0, 2), PSR_TYPE_TEXT);
    EXPECT_EQ(psr_result_get_type(result, 0, 3), PSR_TYPE_NULL);

    EXPECT_EQ(psr_result_is_null(result, 0, 0), 0);
    EXPECT_EQ(psr_result_is_null(result, 0, 3), 1);

    int64_t i;
    EXPECT_EQ(psr_result_get_int(result, 0, 0, &i), PSR_OK);
    EXPECT_EQ(i, 42);

    double f;
    EXPECT_EQ(psr_result_get_double(result, 0, 1, &f), PSR_OK);
    EXPECT_DOUBLE_EQ(f, 3.14);

    psr_result_free(result);
    psr_database_close(db);
}

TEST_F(CApiTest, Transaction) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* r1 = psr_database_execute(db, "CREATE TABLE counter (value INTEGER)", &error);
    psr_result_free(r1);

    psr_result_t* r2 = psr_database_execute(db, "INSERT INTO counter VALUES (0)", &error);
    psr_result_free(r2);

    EXPECT_EQ(psr_database_begin_transaction(db), PSR_OK);

    psr_result_t* r3 = psr_database_execute(db, "UPDATE counter SET value = 1", &error);
    psr_result_free(r3);

    EXPECT_EQ(psr_database_rollback(db), PSR_OK);

    psr_result_t* result = psr_database_execute(db, "SELECT value FROM counter", &error);
    ASSERT_NE(result, nullptr);

    int64_t value;
    EXPECT_EQ(psr_result_get_int(result, 0, 0, &value), PSR_OK);
    EXPECT_EQ(value, 0);

    psr_result_free(result);
    psr_database_close(db);
}

TEST_F(CApiTest, ErrorHandling) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* result = psr_database_execute(db, "INVALID SQL", &error);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(error, PSR_ERROR_QUERY);

    const char* msg = psr_database_error_message(db);
    EXPECT_NE(msg, nullptr);
    EXPECT_NE(strlen(msg), 0u);

    psr_database_close(db);
}

TEST_F(CApiTest, ErrorStrings) {
    EXPECT_STREQ(psr_error_string(PSR_OK), "Success");
    EXPECT_STREQ(psr_error_string(PSR_ERROR_INVALID_ARGUMENT), "Invalid argument");
    EXPECT_STREQ(psr_error_string(PSR_ERROR_DATABASE), "Database error");
    EXPECT_STREQ(psr_error_string(PSR_ERROR_QUERY), "Query error");
}

TEST_F(CApiTest, Version) {
    const char* version = psr_version();
    EXPECT_NE(version, nullptr);
    EXPECT_STREQ(version, "1.0.0");
}

TEST_F(CApiTest, IndexOutOfRange) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    psr_result_t* r1 = psr_database_execute(db, "CREATE TABLE test (id INTEGER)", &error);
    psr_result_free(r1);

    psr_result_t* r2 = psr_database_execute(db, "INSERT INTO test VALUES (1)", &error);
    psr_result_free(r2);

    psr_result_t* result = psr_database_execute(db, "SELECT * FROM test", &error);
    ASSERT_NE(result, nullptr);

    int64_t value;
    EXPECT_EQ(psr_result_get_int(result, 100, 0, &value), PSR_ERROR_INDEX_OUT_OF_RANGE);
    EXPECT_EQ(psr_result_get_int(result, 0, 100, &value), PSR_ERROR_INDEX_OUT_OF_RANGE);

    EXPECT_EQ(psr_result_column_name(result, 100), nullptr);

    psr_result_free(result);
    psr_database_close(db);
}

// Migration C API tests
class CApiMigrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_db_path_ = (fs::temp_directory_path() / "psr_c_migration_test.db").string();
        test_schema_path_ = fs::temp_directory_path() / "psr_c_test_schema";

        // Clean up from previous runs
        if (fs::exists(test_db_path_)) {
            fs::remove(test_db_path_);
        }
        if (fs::exists(test_schema_path_)) {
            fs::remove_all(test_schema_path_);
        }

        // Create schema directory
        fs::create_directories(test_schema_path_);
    }

    void TearDown() override {
        if (fs::exists(test_db_path_)) {
            fs::remove(test_db_path_);
        }
        if (fs::exists(test_schema_path_)) {
            fs::remove_all(test_schema_path_);
        }
    }

    void create_migration(int version, const std::string& sql) {
        auto dir = test_schema_path_ / std::to_string(version);
        fs::create_directories(dir);
        std::ofstream(dir / "up.sql") << sql;
    }

    std::string test_db_path_;
    fs::path test_schema_path_;
};

TEST_F(CApiMigrationTest, FromSchemaBasic) {
    create_migration(1, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);");
    create_migration(2, "CREATE TABLE posts (id INTEGER PRIMARY KEY, title TEXT);");

    psr_error_t error;
    psr_database_t* db =
        psr_database_from_schema(test_db_path_.c_str(), test_schema_path_.string().c_str(), PSR_LOG_OFF, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(psr_database_current_version(db), 2);

    // Verify tables exist
    psr_result_t* result =
        psr_database_execute(db, "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name", &error);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(psr_result_row_count(result), 2u);

    psr_result_free(result);
    psr_database_close(db);
}

TEST_F(CApiMigrationTest, FromSchemaEmpty) {
    psr_error_t error;
    psr_database_t* db =
        psr_database_from_schema(test_db_path_.c_str(), test_schema_path_.string().c_str(), PSR_LOG_OFF, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(psr_database_current_version(db), 0);

    psr_database_close(db);
}

TEST_F(CApiMigrationTest, FromSchemaNullArgs) {
    psr_error_t error;

    psr_database_t* db1 = psr_database_from_schema(nullptr, test_schema_path_.string().c_str(), PSR_LOG_OFF, &error);
    EXPECT_EQ(db1, nullptr);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);

    psr_database_t* db2 = psr_database_from_schema(test_db_path_.c_str(), nullptr, PSR_LOG_OFF, &error);
    EXPECT_EQ(db2, nullptr);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);
}

TEST_F(CApiMigrationTest, CurrentVersionAndSetVersion) {
    psr_error_t error;
    psr_database_t* db = psr_database_open(test_db_path_.c_str(), PSR_LOG_OFF, 0, &error);
    ASSERT_NE(db, nullptr);

    EXPECT_EQ(psr_database_current_version(db), 0);

    EXPECT_EQ(psr_database_set_version(db, 5), PSR_OK);
    EXPECT_EQ(psr_database_current_version(db), 5);

    EXPECT_EQ(psr_database_set_version(db, 10), PSR_OK);
    EXPECT_EQ(psr_database_current_version(db), 10);

    psr_database_close(db);
}

TEST_F(CApiMigrationTest, MigrationErrorString) {
    EXPECT_STREQ(psr_error_string(PSR_ERROR_MIGRATION), "Migration error");
}

TEST_F(CApiMigrationTest, FromSchemaInvalidPath) {
    psr_error_t error;
    psr_database_t* db = psr_database_from_schema(test_db_path_.c_str(), "/nonexistent/path", PSR_LOG_OFF, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_MIGRATION);
}

// Element builder C API tests
class CApiElementTest : public ::testing::Test {
protected:
    void SetUp() override {
        psr_error_t error;
        db_ = psr_database_open(":memory:", PSR_LOG_OFF, 0, &error);
        ASSERT_NE(db_, nullptr);

        // Create test schema
        psr_result_t* r1 = psr_database_execute(
            db_,
            "CREATE TABLE Resource (id INTEGER PRIMARY KEY AUTOINCREMENT, label TEXT UNIQUE NOT NULL, "
            "type TEXT NOT NULL DEFAULT 'D' CHECK(type IN ('D', 'E', 'F'))) STRICT",
            &error);
        psr_result_free(r1);

        psr_result_t* r2 = psr_database_execute(
            db_,
            "CREATE TABLE Configuration (id INTEGER PRIMARY KEY AUTOINCREMENT, label TEXT NOT NULL, "
            "value1 REAL NOT NULL DEFAULT 100, date_time_value2 TEXT) STRICT",
            &error);
        psr_result_free(r2);
    }

    void TearDown() override { psr_database_close(db_); }

    psr_database_t* db_;
};

TEST_F(CApiElementTest, ElementCreateAndFree) {
    psr_element_t* elem = psr_element_create();
    ASSERT_NE(elem, nullptr);
    psr_element_free(elem);
}

TEST_F(CApiElementTest, ElementSetString) {
    psr_element_t* elem = psr_element_create();
    ASSERT_NE(elem, nullptr);

    EXPECT_EQ(psr_element_set_string(elem, "label", "Test"), PSR_OK);

    psr_error_t error;
    int64_t id = psr_database_create_element(db_, "Resource", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(id, 1);

    psr_result_t* result = psr_database_execute(db_, "SELECT label FROM Resource WHERE id = 1", &error);
    EXPECT_STREQ(psr_result_get_string(result, 0, 0), "Test");
    psr_result_free(result);
}

TEST_F(CApiElementTest, ElementSetMultipleFields) {
    psr_element_t* elem = psr_element_create();

    psr_element_set_string(elem, "label", "Config 1");
    psr_element_set_double(elem, "value1", 42.5);

    psr_error_t error;
    int64_t id = psr_database_create_element(db_, "Configuration", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(id, 1);

    psr_result_t* result = psr_database_execute(db_, "SELECT label, value1 FROM Configuration WHERE id = 1", &error);
    EXPECT_STREQ(psr_result_get_string(result, 0, 0), "Config 1");

    double value;
    psr_result_get_double(result, 0, 1, &value);
    EXPECT_DOUBLE_EQ(value, 42.5);
    psr_result_free(result);
}

TEST_F(CApiElementTest, ElementSetNull) {
    psr_element_t* elem = psr_element_create();

    psr_element_set_string(elem, "label", "Null Test");
    psr_element_set_double(elem, "value1", 10.0);
    psr_element_set_null(elem, "date_time_value2");

    psr_error_t error;
    int64_t id = psr_database_create_element(db_, "Configuration", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(id, 1);

    psr_result_t* result = psr_database_execute(db_, "SELECT date_time_value2 FROM Configuration WHERE id = 1", &error);
    EXPECT_EQ(psr_result_is_null(result, 0, 0), 1);
    psr_result_free(result);
}

TEST_F(CApiElementTest, ElementSetInt) {
    // Create a table with integer column for this test
    psr_error_t error;
    psr_result_t* r = psr_database_execute(db_, "CREATE TABLE IntTest (id INTEGER PRIMARY KEY, count INTEGER)", &error);
    psr_result_free(r);

    psr_element_t* elem = psr_element_create();
    psr_element_set_int(elem, "count", 42);

    int64_t id = psr_database_create_element(db_, "IntTest", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_OK);
    EXPECT_EQ(id, 1);

    psr_result_t* result = psr_database_execute(db_, "SELECT count FROM IntTest WHERE id = 1", &error);
    int64_t count;
    psr_result_get_int(result, 0, 0, &count);
    EXPECT_EQ(count, 42);
    psr_result_free(result);
}

TEST_F(CApiElementTest, CreateElementReturnsCorrectRowId) {
    psr_error_t error;

    for (int i = 1; i <= 3; ++i) {
        psr_element_t* elem = psr_element_create();
        std::string label = "Resource " + std::to_string(i);
        psr_element_set_string(elem, "label", label.c_str());

        int64_t id = psr_database_create_element(db_, "Resource", elem, &error);
        psr_element_free(elem);

        EXPECT_EQ(error, PSR_OK);
        EXPECT_EQ(id, i);
    }
}

TEST_F(CApiElementTest, CreateElementErrorOnInvalidTable) {
    psr_element_t* elem = psr_element_create();
    psr_element_set_string(elem, "label", "Test");

    psr_error_t error;
    int64_t id = psr_database_create_element(db_, "NonexistentTable", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_ERROR_QUERY);
    EXPECT_EQ(id, 0);
}

TEST_F(CApiElementTest, CreateElementErrorOnConstraintViolation) {
    psr_element_t* elem1 = psr_element_create();
    psr_element_set_string(elem1, "label", "Duplicate");

    psr_error_t error;
    psr_database_create_element(db_, "Resource", elem1, &error);
    psr_element_free(elem1);

    // Try to insert duplicate
    psr_element_t* elem2 = psr_element_create();
    psr_element_set_string(elem2, "label", "Duplicate");

    int64_t id = psr_database_create_element(db_, "Resource", elem2, &error);
    psr_element_free(elem2);

    EXPECT_EQ(error, PSR_ERROR_QUERY);
    EXPECT_EQ(id, 0);
}

TEST_F(CApiElementTest, ElementSetNullArguments) {
    psr_element_t* elem = psr_element_create();

    EXPECT_EQ(psr_element_set_string(nullptr, "col", "val"), PSR_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(psr_element_set_string(elem, nullptr, "val"), PSR_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(psr_element_set_int(nullptr, "col", 1), PSR_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(psr_element_set_double(nullptr, "col", 1.0), PSR_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(psr_element_set_null(nullptr, "col"), PSR_ERROR_INVALID_ARGUMENT);

    psr_element_free(elem);
}

TEST_F(CApiElementTest, CreateElementNullArguments) {
    psr_element_t* elem = psr_element_create();
    psr_element_set_string(elem, "label", "Test");

    psr_error_t error;

    EXPECT_EQ(psr_database_create_element(nullptr, "Resource", elem, &error), 0);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(psr_database_create_element(db_, nullptr, elem, &error), 0);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(psr_database_create_element(db_, "Resource", nullptr, &error), 0);
    EXPECT_EQ(error, PSR_ERROR_INVALID_ARGUMENT);

    psr_element_free(elem);
}

TEST_F(CApiElementTest, CreateElementErrorOnNonexistentColumn) {
    // "type3" doesn't exist, should be "type"
    psr_element_t* elem = psr_element_create();
    psr_element_set_string(elem, "label", "Resource 4");
    psr_element_set_string(elem, "type3", "E");  // Wrong column name

    psr_error_t error;
    int64_t id = psr_database_create_element(db_, "Resource", elem, &error);
    psr_element_free(elem);

    EXPECT_EQ(error, PSR_ERROR_QUERY);
    EXPECT_EQ(id, 0);
}

// ============================================================================
// Schema Validation Tests
// ============================================================================

class CApiSchemaValidationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_db_path_ = (fs::temp_directory_path() / "psr_schema_test.db").string();
        test_sql_path_ = (fs::temp_directory_path() / "psr_schema_test.sql").string();
    }

    void TearDown() override {
        if (fs::exists(test_db_path_)) {
            fs::remove(test_db_path_);
        }
        if (fs::exists(test_sql_path_)) {
            fs::remove(test_sql_path_);
        }
    }

    void write_sql_file(const std::string& content) {
        std::ofstream file(test_sql_path_);
        file << content;
        file.close();
    }

    std::string test_db_path_;
    std::string test_sql_path_;
};

TEST_F(CApiSchemaValidationTest, ValidSchemaSucceeds) {
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE,
            value REAL
        );
        CREATE TABLE Resource_vector_costs (
            id INTEGER PRIMARY KEY,
            resource_id INTEGER NOT NULL,
            vector_index INTEGER NOT NULL,
            cost REAL,
            FOREIGN KEY (resource_id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);

    psr_database_close(db);
}

TEST_F(CApiSchemaValidationTest, InvalidForeignKeyActionsFailsValidation) {
    // CASCADE delete with SET NULL update is invalid
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE
        );
        CREATE TABLE Child (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE,
            resource_id INTEGER,
            FOREIGN KEY (resource_id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE SET NULL
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_SCHEMA_VALIDATION);
}

TEST_F(CApiSchemaValidationTest, MissingVectorIndexFailsValidation) {
    // Vector table without vector_index column
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE
        );
        CREATE TABLE Resource_vector_costs (
            id INTEGER PRIMARY KEY,
            resource_id INTEGER NOT NULL,
            cost REAL,
            FOREIGN KEY (resource_id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_SCHEMA_VALIDATION);
}

TEST_F(CApiSchemaValidationTest, DuplicateAttributesFailsValidation) {
    // 'cost' appears in both Resource and Resource_vector_costs
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE,
            cost REAL
        );
        CREATE TABLE Resource_vector_costs (
            id INTEGER PRIMARY KEY,
            resource_id INTEGER NOT NULL,
            vector_index INTEGER NOT NULL,
            cost REAL,
            FOREIGN KEY (resource_id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_SCHEMA_VALIDATION);
}

TEST_F(CApiSchemaValidationTest, MissingLabelColumnFailsValidation) {
    // Collection table without label column
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    EXPECT_EQ(db, nullptr);
    EXPECT_EQ(error, PSR_ERROR_SCHEMA_VALIDATION);
}

TEST_F(CApiSchemaValidationTest, ConfigurationTableDoesNotNeedLabel) {
    // Configuration table is exempt from label requirement
    write_sql_file(R"(
        CREATE TABLE Configuration (
            id INTEGER PRIMARY KEY,
            key TEXT NOT NULL UNIQUE,
            value TEXT
        );
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);

    psr_database_close(db);
}

TEST_F(CApiSchemaValidationTest, FilesTableDoesNotNeedLabel) {
    // Tables ending with _files are exempt from label requirement
    write_sql_file(R"(
        CREATE TABLE Resource (
            id INTEGER PRIMARY KEY,
            label TEXT NOT NULL UNIQUE
        );
        CREATE TABLE Resource_files (
            id INTEGER PRIMARY KEY,
            resource_id INTEGER NOT NULL,
            file_path TEXT,
            FOREIGN KEY (resource_id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE
        );
    )");

    psr_error_t error;
    psr_database_t* db = psr_database_from_sql_file(test_db_path_.c_str(), test_sql_path_.c_str(), PSR_LOG_OFF, &error);

    ASSERT_NE(db, nullptr);
    EXPECT_EQ(error, PSR_OK);

    psr_database_close(db);
}

TEST_F(CApiSchemaValidationTest, SchemaValidationErrorString) {
    EXPECT_STREQ(psr_error_string(PSR_ERROR_SCHEMA_VALIDATION), "Schema validation error");
}
