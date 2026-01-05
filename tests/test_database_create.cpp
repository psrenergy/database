/**
 * @file test_database_create.cpp
 * @brief Tests for create_element functionality, ported from PSRDatabase.jl
 *
 * These tests are ported from PSRDatabase.jl/test/test_create/test_create.jl
 * Only tests using scalar parameters are included (vector/time series not yet implemented).
 */

#include "psr/database.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

// Helper to get the path to test schema files
fs::path get_test_schema_path(const std::string& schema_name) {
    // Look for schema files relative to the test executable or source directory
    std::vector<fs::path> search_paths = {
        fs::current_path() / "test_create" / schema_name,
        fs::current_path() / ".." / "tests" / "test_create" / schema_name,
        fs::current_path() / ".." / ".." / "tests" / "test_create" / schema_name,
        fs::path(__FILE__).parent_path() / "test_create" / schema_name,
    };

    for (const auto& path : search_paths) {
        if (fs::exists(path)) {
            return path;
        }
    }

    // Return the most likely path even if it doesn't exist
    return fs::current_path() / "test_create" / schema_name;
}

// Helper to split SQL into individual statements
std::vector<std::string> split_sql_statements(const std::string& sql) {
    std::vector<std::string> statements;
    std::string current;
    bool in_string = false;
    char string_char = 0;

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
        }

        if (c == ';' && !in_string) {
            // End of statement
            if (!current.empty()) {
                // Trim whitespace
                size_t start = current.find_first_not_of(" \t\n\r");
                if (start != std::string::npos) {
                    size_t end = current.find_last_not_of(" \t\n\r");
                    statements.push_back(current.substr(start, end - start + 1));
                }
            }
            current.clear();
        } else {
            current += c;
        }
    }

    // Handle last statement without trailing semicolon
    if (!current.empty()) {
        size_t start = current.find_first_not_of(" \t\n\r");
        if (start != std::string::npos) {
            size_t end = current.find_last_not_of(" \t\n\r");
            statements.push_back(current.substr(start, end - start + 1));
        }
    }

    return statements;
}

// Helper to create a temporary database from a schema file
class TestDatabaseHelper {
public:
    TestDatabaseHelper(const std::string& schema_name)
        : schema_path_(get_test_schema_path(schema_name)),
          db_path_(fs::temp_directory_path() / ("test_" + schema_name + ".sqlite")) {
        // Remove old database if exists
        if (fs::exists(db_path_)) {
            fs::remove(db_path_);
        }
    }

    ~TestDatabaseHelper() {
        db_.reset();
        if (fs::exists(db_path_)) {
            fs::remove(db_path_);
        }
    }

    void open() {
        // Read schema SQL
        std::ifstream file(schema_path_);
        if (!file) {
            throw std::runtime_error("Failed to open schema file: " + schema_path_.string());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string schema_sql = buffer.str();

        // Create database and apply schema
        db_ = std::make_unique<psr::Database>(db_path_.string(), psr::LogLevel::off);

        // Execute each statement separately
        auto statements = split_sql_statements(schema_sql);
        for (const auto& stmt : statements) {
            if (!stmt.empty()) {
                db_->execute(stmt);
            }
        }
    }

    psr::Database& db() { return *db_; }

    const fs::path& db_path() const { return db_path_; }

private:
    fs::path schema_path_;
    fs::path db_path_;
    std::unique_ptr<psr::Database> db_;
};

}  // namespace

// =============================================================================
// test_create_parameters - Basic scalar parameter tests
// =============================================================================

class CreateParametersTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: @test_throws PSRDatabase.DatabaseException PSRDatabase.create_element!(
//     db, "Configuration"; label = "Toy Case", value1 = "wrong")
TEST_F(CreateParametersTest, ThrowsOnTypeMismatch) {
    // String for REAL column should throw
    EXPECT_THROW(
        db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", std::string("wrong")}}),
        std::runtime_error);
}

// From Julia: PSRDatabase.create_element!(db, "Configuration"; label = "Toy Case", value1 = 1.0)
//             PSRDatabase.create_element!(db, "Resource"; label = "Resource 2")
//             PSRDatabase.create_element!(db, "Resource"; label = "Resource 1", type = "E")
TEST_F(CreateParametersTest, SucceedsWithValidParameters) {
    EXPECT_NO_THROW({
        int64_t id1 = db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});
        EXPECT_EQ(id1, 1);

        int64_t id2 = db().create_element("Resource", {{"label", std::string("Resource 2")}});
        EXPECT_EQ(id2, 1);

        int64_t id3 =
            db().create_element("Resource", {{"label", std::string("Resource 1")}, {"type", std::string("E")}});
        EXPECT_EQ(id3, 2);
    });

    // Verify data was inserted
    auto result = db().execute("SELECT label, value1 FROM Configuration");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(0).value_or(""), "Toy Case");
    EXPECT_DOUBLE_EQ(result[0].get_double(1).value_or(0.0), 1.0);

    auto resources = db().execute("SELECT label, type FROM Resource ORDER BY label");
    ASSERT_EQ(resources.row_count(), 2);
    EXPECT_EQ(resources[0].get_string(0).value_or(""), "Resource 1");
    EXPECT_EQ(resources[0].get_string(1).value_or(""), "E");
    EXPECT_EQ(resources[1].get_string(0).value_or(""), "Resource 2");
    EXPECT_EQ(resources[1].get_string(1).value_or(""), "D");  // Default value
}

// From Julia: @test_throws PSRDatabase.DatabaseException PSRDatabase.create_element!(
//     db, "Resource"; label = "Resource 4", type3 = "E")
TEST_F(CreateParametersTest, ThrowsOnNonexistentColumn) {
    EXPECT_THROW(db().create_element("Resource", {{"label", std::string("Resource 4")}, {"type3", std::string("E")}}),
                 std::runtime_error);
}

// Additional test: CHECK constraint violation
TEST_F(CreateParametersTest, ThrowsOnCheckConstraintViolation) {
    // enum1 must be 'A', 'B', or 'C'
    EXPECT_THROW(db().create_element("Configuration", {{"label", std::string("Test")}, {"enum1", std::string("X")}}),
                 std::runtime_error);

    // type must be 'D', 'E', or 'F'
    EXPECT_THROW(db().create_element("Resource", {{"label", std::string("Test")}, {"type", std::string("G")}}),
                 std::runtime_error);
}

// Additional test: UNIQUE constraint violation
TEST_F(CreateParametersTest, ThrowsOnUniqueConstraintViolation) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    // Duplicate label should throw
    EXPECT_THROW(db().create_element("Configuration", {{"label", std::string("Toy Case")}}), std::runtime_error);
}

// =============================================================================
// test_create_empty_parameters - Required vs optional field tests
// =============================================================================

class CreateEmptyParametersTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: PSRDatabase.create_element!(db, "Configuration"; label = "Toy Case")
TEST_F(CreateEmptyParametersTest, SucceedsWithOnlyRequiredFields) {
    // Configuration has defaults for value1 and enum1, so only label is required
    EXPECT_NO_THROW({
        int64_t id = db().create_element("Configuration", {{"label", std::string("Toy Case")}});
        EXPECT_EQ(id, 1);
    });

    // Verify defaults were applied
    auto result = db().execute("SELECT label, value1, enum1 FROM Configuration");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(0).value_or(""), "Toy Case");
    EXPECT_DOUBLE_EQ(result[0].get_double(1).value_or(0.0), 100.0);  // Default
    EXPECT_EQ(result[0].get_string(2).value_or(""), "A");            // Default
}

// From Julia: @test_throws PSRDatabase.DatabaseException PSRDatabase.create_element!(db, "Resource")
TEST_F(CreateEmptyParametersTest, ThrowsWhenRequiredFieldMissing) {
    // Resource.label is NOT NULL with no default, so empty create should fail
    EXPECT_THROW(db().create_element("Resource", {}), std::runtime_error);
}

// =============================================================================
// test_create_with_transaction - Transaction tests
// =============================================================================

class CreateTransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters_and_vectors.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia:
// PSRDatabase.create_element!(db, "Configuration"; label = "Toy Case", value1 = 1.0)
// PSRDatabase.SQLite.transaction(db.sqlite_db) do
//     for i in 1:10
//         PSRDatabase.create_element!(db, "Plant"; label = "Plant $i", capacity = 5.0 * i)
//     end
// end
TEST_F(CreateTransactionTest, WorksWithTransaction) {
    // First create Configuration
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create 10 Plants within a transaction
    db().begin_transaction();
    for (int i = 1; i <= 10; i++) {
        db().create_element("Plant", {{"label", std::string("Plant ") + std::to_string(i)}, {"capacity", 5.0 * i}});
    }
    db().commit();

    // Verify all plants were created
    auto result = db().execute("SELECT COUNT(*) FROM Plant");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_int(0).value_or(0), 10);

    // Verify capacities
    auto plants = db().execute("SELECT label, capacity FROM Plant ORDER BY id");
    ASSERT_EQ(plants.row_count(), 10);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(plants[i].get_string(0).value_or(""), "Plant " + std::to_string(i + 1));
        EXPECT_DOUBLE_EQ(plants[i].get_double(1).value_or(0.0), 5.0 * (i + 1));
    }
}

TEST_F(CreateTransactionTest, RollbackOnFailure) {
    // Create Configuration first
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Start transaction and create some plants
    db().begin_transaction();
    db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 10.0}});
    db().create_element("Plant", {{"label", std::string("Plant 2")}, {"capacity", 20.0}});

    // Rollback the transaction
    db().rollback();

    // Verify no plants were created
    auto result = db().execute("SELECT COUNT(*) FROM Plant");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_int(0).value_or(-1), 0);
}

// =============================================================================
// test_create_scalar_parameter_date - Date string tests
// =============================================================================

class CreateScalarDateTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_scalar_parameter_date.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: PSRDatabase.create_element!(db, "Configuration";
//     label = "Toy Case", date_initial = DateTime(2000), date_final = DateTime(2001, 10, 12, 23, 45, 12))
TEST_F(CreateScalarDateTest, SucceedsWithDateStrings) {
    EXPECT_NO_THROW({
        int64_t id = db().create_element("Configuration", {{"label", std::string("Toy Case")},
                                                           {"date_initial", std::string("2000-01-01 00:00:00")},
                                                           {"date_final", std::string("2001-10-12 23:45:12")}});
        EXPECT_EQ(id, 1);
    });

    // Verify dates were stored correctly
    auto result = db().execute("SELECT date_initial, date_final FROM Configuration");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(0).value_or(""), "2000-01-01 00:00:00");
    EXPECT_EQ(result[0].get_string(1).value_or(""), "2001-10-12 23:45:12");
}

TEST_F(CreateScalarDateTest, UsesDefaultDateValue) {
    // date_initial has a default value of "2019-01-01"
    EXPECT_NO_THROW({
        db().create_element("Configuration",
                            {{"label", std::string("Toy Case")}, {"date_final", std::string("2020-12-31")}});
    });

    auto result = db().execute("SELECT date_initial FROM Configuration");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(0).value_or(""), "2019-01-01");
}

// Note: In SQLite, TEXT columns don't validate date formats.
// The Julia tests that throw on invalid dates rely on Julia-side validation,
// not SQLite constraints. We include this test to show the difference.
TEST_F(CreateScalarDateTest, AcceptsAnyTextForDateColumn) {
    // SQLite TEXT columns accept any string, even invalid dates
    // This differs from Julia which validates date formats
    EXPECT_NO_THROW({
        db().create_element("Resource",
                            {{"label", std::string("Resource 1")}, {"date_initial_1", std::string("not-a-date")}});
    });
}

// =============================================================================
// Additional tests for foreign key relationships
// =============================================================================

class CreateWithRelationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters_and_vectors.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: PSRDatabase.create_element!(db, "Plant"; label = "Plant 3", resource_id = 1)
TEST_F(CreateWithRelationsTest, SucceedsWithValidForeignKey) {
    // Create Configuration (required for the schema)
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create a Resource first
    int64_t resource_id = db().create_element("Resource", {{"label", std::string("Resource 1")}});
    EXPECT_EQ(resource_id, 1);

    // Create Plant with foreign key to Resource
    EXPECT_NO_THROW({
        int64_t plant_id = db().create_element(
            "Plant", {{"label", std::string("Plant 1")}, {"capacity", 50.0}, {"resource_id", resource_id}});
        EXPECT_EQ(plant_id, 1);
    });

    // Verify the relationship
    auto result = db().execute("SELECT p.label, r.label FROM Plant p JOIN Resource r ON p.resource_id = r.id");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(0).value_or(""), "Plant 1");
    EXPECT_EQ(result[0].get_string(1).value_or(""), "Resource 1");
}

TEST_F(CreateWithRelationsTest, ThrowsOnInvalidForeignKey) {
    // Create Configuration
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Try to create Plant with non-existent Resource ID (foreign key violation)
    EXPECT_THROW(
        db().create_element("Plant",
                            {
                                {"label", std::string("Plant 1")}, {"resource_id", int64_t(999)}  // Non-existent
                            }),
        std::runtime_error);
}

TEST_F(CreateWithRelationsTest, AcceptsNullForeignKey) {
    // Create Configuration
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Plant.resource_id allows NULL
    EXPECT_NO_THROW({
        db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 50.0}, {"resource_id", nullptr}});
    });
}

// =============================================================================
// Vector Tests - From Julia PSRDatabase.jl/test/test_create/test_create.jl
// =============================================================================

class CreateVectorsTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters_and_vectors.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: test_create_parameters_and_vectors
// PSRDatabase.create_element!(db, "Resource"; label = "Resource 1", some_value = [1.0, 2.0])
TEST_F(CreateVectorsTest, SucceedsWithVectorAttributes) {
    // Create Configuration (required for schema)
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create Resource with vector attribute
    EXPECT_NO_THROW({
        int64_t id = db().create_element(
            "Resource", {{"label", std::string("Resource 1")}, {"some_value", std::vector<double>{1.0, 2.0, 3.0}}});
        EXPECT_EQ(id, 1);
    });

    // Verify scalar data
    auto resource = db().execute("SELECT id, label FROM Resource WHERE label = 'Resource 1'");
    ASSERT_EQ(resource.row_count(), 1);
    EXPECT_EQ(resource[0].get_int(0).value_or(0), 1);

    // Verify vector data was inserted
    auto vectors = db().execute("SELECT vector_index, some_value FROM Resource_vector_some_group "
                                "WHERE id = 1 ORDER BY vector_index");
    ASSERT_EQ(vectors.row_count(), 3);
    EXPECT_EQ(vectors[0].get_int(0).value_or(0), 1);
    EXPECT_DOUBLE_EQ(vectors[0].get_double(1).value_or(0.0), 1.0);
    EXPECT_EQ(vectors[1].get_int(0).value_or(0), 2);
    EXPECT_DOUBLE_EQ(vectors[1].get_double(1).value_or(0.0), 2.0);
    EXPECT_EQ(vectors[2].get_int(0).value_or(0), 3);
    EXPECT_DOUBLE_EQ(vectors[2].get_double(1).value_or(0.0), 3.0);
}

// Multiple resources with vectors
TEST_F(CreateVectorsTest, MultipleElementsWithVectors) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create multiple resources
    db().create_element("Resource",
                        {{"label", std::string("Resource 1")}, {"some_value", std::vector<double>{1.0, 2.0}}});

    db().create_element("Resource",
                        {{"label", std::string("Resource 2")}, {"some_value", std::vector<double>{10.0, 20.0, 30.0}}});

    // Verify vector counts
    auto count1 = db().execute("SELECT COUNT(*) FROM Resource_vector_some_group WHERE id = 1");
    EXPECT_EQ(count1[0].get_int(0).value_or(0), 2);

    auto count2 = db().execute("SELECT COUNT(*) FROM Resource_vector_some_group WHERE id = 2");
    EXPECT_EQ(count2[0].get_int(0).value_or(0), 3);
}

// Vector with foreign key relations - resolve labels to IDs
// From Julia: test_create_vectors_with_relations
TEST_F(CreateVectorsTest, VectorWithForeignKeyRelationsById) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create Cost elements
    int64_t cost1 = db().create_element("Cost", {{"label", std::string("Cost 1")}, {"value", 10.0}});
    int64_t cost2 = db().create_element("Cost", {{"label", std::string("Cost 2")}, {"value", 20.0}});
    EXPECT_EQ(cost1, 1);
    EXPECT_EQ(cost2, 2);

    // Create Plant with vector relation using IDs
    EXPECT_NO_THROW({
        db().create_element("Plant", {{"label", std::string("Plant 1")},
                                      {"capacity", 100.0},
                                      {"some_factor", std::vector<double>{1.5, 2.5}},
                                      {"cost_id", std::vector<int64_t>{cost1, cost2}}});
    });

    // Verify vector relations
    auto vectors = db().execute("SELECT vector_index, some_factor, cost_id FROM Plant_vector_cost_relation "
                                "WHERE id = 1 ORDER BY vector_index");
    ASSERT_EQ(vectors.row_count(), 2);
    EXPECT_DOUBLE_EQ(vectors[0].get_double(1).value_or(0.0), 1.5);
    EXPECT_EQ(vectors[0].get_int(2).value_or(0), 1);
    EXPECT_DOUBLE_EQ(vectors[1].get_double(1).value_or(0.0), 2.5);
    EXPECT_EQ(vectors[1].get_int(2).value_or(0), 2);
}

// Vector with foreign key relations using string labels (resolved to IDs)
TEST_F(CreateVectorsTest, VectorWithForeignKeyRelationsByLabel) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create Cost elements
    db().create_element("Cost", {{"label", std::string("Cost A")}, {"value", 10.0}});
    db().create_element("Cost", {{"label", std::string("Cost B")}, {"value", 20.0}});

    // Create Plant with vector relation using labels (should be resolved to IDs)
    EXPECT_NO_THROW({
        db().create_element("Plant", {{"label", std::string("Plant 1")},
                                      {"capacity", 100.0},
                                      {"some_factor", std::vector<double>{1.5, 2.5}},
                                      {"cost_id", std::vector<std::string>{"Cost A", "Cost B"}}});
    });

    // Verify labels were resolved to IDs
    auto vectors = db().execute("SELECT pcr.vector_index, pcr.cost_id, c.label "
                                "FROM Plant_vector_cost_relation pcr "
                                "JOIN Cost c ON pcr.cost_id = c.id "
                                "WHERE pcr.id = 1 ORDER BY pcr.vector_index");
    ASSERT_EQ(vectors.row_count(), 2);
    EXPECT_EQ(vectors[0].get_string(2).value_or(""), "Cost A");
    EXPECT_EQ(vectors[1].get_string(2).value_or(""), "Cost B");
}

// Scalar foreign key resolution by label
TEST_F(CreateVectorsTest, ScalarForeignKeyResolvedByLabel) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create Resource
    db().create_element("Resource", {{"label", std::string("My Resource")}});

    // Create Plant with foreign key using label instead of ID
    EXPECT_NO_THROW({
        db().create_element("Plant", {
                                         {"label", std::string("Plant 1")},
                                         {"capacity", 50.0},
                                         {"resource_id", std::string("My Resource")}  // Label should be resolved to ID
                                     });
    });

    // Verify the relationship
    auto result = db().execute("SELECT p.label, r.label FROM Plant p JOIN Resource r ON p.resource_id = r.id");
    ASSERT_EQ(result.row_count(), 1);
    EXPECT_EQ(result[0].get_string(1).value_or(""), "My Resource");
}

// Empty vectors should be allowed
TEST_F(CreateVectorsTest, EmptyVectorIsAllowed) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    EXPECT_NO_THROW({
        db().create_element("Resource", {{"label", std::string("Resource 1")}, {"some_value", std::vector<double>{}}});
    });

    // Verify no vector rows were inserted
    auto vectors = db().execute("SELECT COUNT(*) FROM Resource_vector_some_group WHERE id = 1");
    EXPECT_EQ(vectors[0].get_int(0).value_or(-1), 0);
}

// Process with multiple vector groups (inputs and outputs)
TEST_F(CreateVectorsTest, MultipleVectorGroups) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    // Create products
    db().create_element("Product", {{"label", std::string("Coal")}, {"unit", std::string("ton")}});
    db().create_element("Product", {{"label", std::string("Electricity")}, {"unit", std::string("MWh")}});

    // Create process with both input and output vectors
    EXPECT_NO_THROW({
        db().create_element("Process", {{"label", std::string("Coal Plant")},
                                        {"factor_input", std::vector<double>{2.5}},
                                        {"product_input", std::vector<std::string>{"Coal"}},
                                        {"factor_output", std::vector<double>{1.0}},
                                        {"product_output", std::vector<std::string>{"Electricity"}}});
    });

    // Verify inputs
    auto inputs = db().execute("SELECT pi.factor_input, p.label FROM Process_vector_inputs pi "
                               "JOIN Product p ON pi.product_input = p.id WHERE pi.id = 1");
    ASSERT_EQ(inputs.row_count(), 1);
    EXPECT_DOUBLE_EQ(inputs[0].get_double(0).value_or(0.0), 2.5);
    EXPECT_EQ(inputs[0].get_string(1).value_or(""), "Coal");

    // Verify outputs
    auto outputs = db().execute("SELECT po.factor_output, p.label FROM Process_vector_outputs po "
                                "JOIN Product p ON po.product_output = p.id WHERE po.id = 1");
    ASSERT_EQ(outputs.row_count(), 1);
    EXPECT_DOUBLE_EQ(outputs[0].get_double(0).value_or(0.0), 1.0);
    EXPECT_EQ(outputs[0].get_string(1).value_or(""), "Electricity");
}

// Vectors in same group must have same length
TEST_F(CreateVectorsTest, VectorsInSameGroupMustHaveSameLength) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});
    db().create_element("Cost", {{"label", std::string("Cost 1")}});

    // some_factor and cost_id are in same vector group (Plant_vector_cost_relation)
    // They must have the same length
    EXPECT_THROW(
        {
            db().create_element("Plant", {
                                             {"label", std::string("Plant 1")},
                                             {"some_factor", std::vector<double>{1.0, 2.0, 3.0}},  // 3 elements
                                             {"cost_id", std::vector<int64_t>{1, 2}}  // 2 elements - mismatch!
                                         });
        },
        std::runtime_error);
}

// Invalid foreign key label should throw
TEST_F(CreateVectorsTest, ThrowsOnInvalidForeignKeyLabel) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}, {"value1", 1.0}});

    EXPECT_THROW(
        {
            db().create_element(
                "Plant", {{"label", std::string("Plant 1")}, {"resource_id", std::string("NonexistentResource")}});
        },
        std::runtime_error);
}

// =============================================================================
// Time Series Tests
// =============================================================================

class CreateTimeSeriesTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_time_series.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

// From Julia: test_create_time_series
// Basic time series insertion
TEST_F(CreateTimeSeriesTest, SucceedsWithBasicTimeSeries) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    // Create time series data
    psr::TimeSeries generation;
    generation["date_time"] = {std::string("2020-01-01 00:00:00"), std::string("2020-01-01 01:00:00"),
                               std::string("2020-01-01 02:00:00")};
    generation["block"] = {int64_t(1), int64_t(1), int64_t(1)};
    generation["generation"] = {100.0, 150.0, 120.0};

    std::map<std::string, psr::TimeSeries> time_series;
    time_series["generation"] = generation;

    EXPECT_NO_THROW({
        int64_t id =
            db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 200.0}}, time_series);
        EXPECT_EQ(id, 1);
    });

    // Verify time series data
    auto ts_result = db().execute("SELECT date_time, block, generation FROM Plant_time_series_generation "
                                  "WHERE id = 1 ORDER BY date_time");
    ASSERT_EQ(ts_result.row_count(), 3);
    EXPECT_EQ(ts_result[0].get_string(0).value_or(""), "2020-01-01 00:00:00");
    EXPECT_EQ(ts_result[0].get_int(1).value_or(0), 1);
    EXPECT_DOUBLE_EQ(ts_result[0].get_double(2).value_or(0.0), 100.0);
    EXPECT_DOUBLE_EQ(ts_result[2].get_double(2).value_or(0.0), 120.0);
}

// Time series with multiple dimensions (date_time and segment)
TEST_F(CreateTimeSeriesTest, SucceedsWithMultiDimensionalTimeSeries) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    // Create time series with segment dimension
    psr::TimeSeries prices;
    prices["date_time"] = {std::string("2020-01-01"), std::string("2020-01-01"), std::string("2020-01-02"),
                           std::string("2020-01-02")};
    prices["segment"] = {int64_t(1), int64_t(2), int64_t(1), int64_t(2)};
    prices["price"] = {50.0, 60.0, 55.0, 65.0};
    prices["quantity"] = {100.0, 200.0, 150.0, 250.0};

    std::map<std::string, psr::TimeSeries> time_series;
    time_series["prices"] = prices;

    EXPECT_NO_THROW(
        { db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 100.0}}, time_series); });

    // Verify multi-dimensional time series
    auto ts_result = db().execute("SELECT date_time, segment, price, quantity FROM Plant_time_series_prices "
                                  "WHERE id = 1 ORDER BY date_time, segment");
    ASSERT_EQ(ts_result.row_count(), 4);
    EXPECT_EQ(ts_result[0].get_string(0).value_or(""), "2020-01-01");
    EXPECT_EQ(ts_result[0].get_int(1).value_or(0), 1);
    EXPECT_DOUBLE_EQ(ts_result[0].get_double(2).value_or(0.0), 50.0);
    EXPECT_DOUBLE_EQ(ts_result[3].get_double(3).value_or(0.0), 250.0);
}

// Multiple time series groups
TEST_F(CreateTimeSeriesTest, MultipleTimeSeriesGroups) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    // Create generation time series
    psr::TimeSeries generation;
    generation["date_time"] = {std::string("2020-01-01"), std::string("2020-01-02")};
    generation["block"] = {int64_t(1), int64_t(1)};
    generation["generation"] = {100.0, 120.0};

    // Create prices time series
    psr::TimeSeries prices;
    prices["date_time"] = {std::string("2020-01-01"), std::string("2020-01-02")};
    prices["segment"] = {int64_t(1), int64_t(1)};
    prices["price"] = {50.0, 55.0};
    prices["quantity"] = {200.0, 220.0};

    std::map<std::string, psr::TimeSeries> time_series;
    time_series["generation"] = generation;
    time_series["prices"] = prices;

    EXPECT_NO_THROW(
        { db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 200.0}}, time_series); });

    // Verify both time series
    auto gen_result = db().execute("SELECT COUNT(*) FROM Plant_time_series_generation WHERE id = 1");
    EXPECT_EQ(gen_result[0].get_int(0).value_or(0), 2);

    auto price_result = db().execute("SELECT COUNT(*) FROM Plant_time_series_prices WHERE id = 1");
    EXPECT_EQ(price_result[0].get_int(0).value_or(0), 2);
}

// Simple time series without extra dimensions
TEST_F(CreateTimeSeriesTest, SimpleTimeSeriesWithoutDimensions) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    // Create simple time series (only date_time and value)
    psr::TimeSeries availability;
    availability["date_time"] = {std::string("2020-01-01"), std::string("2020-01-02"), std::string("2020-01-03")};
    availability["value"] = {0.95, 0.90, 1.00};

    std::map<std::string, psr::TimeSeries> time_series;
    time_series["availability"] = availability;

    EXPECT_NO_THROW({ db().create_element("Resource", {{"label", std::string("Resource 1")}}, time_series); });

    auto ts_result = db().execute("SELECT date_time, value FROM Resource_time_series_availability "
                                  "WHERE id = 1 ORDER BY date_time");
    ASSERT_EQ(ts_result.row_count(), 3);
    EXPECT_DOUBLE_EQ(ts_result[0].get_double(1).value_or(0.0), 0.95);
    EXPECT_DOUBLE_EQ(ts_result[2].get_double(1).value_or(0.0), 1.00);
}

// Empty time series should be allowed
TEST_F(CreateTimeSeriesTest, EmptyTimeSeriesIsAllowed) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    std::map<std::string, psr::TimeSeries> time_series;
    // Empty time_series map

    EXPECT_NO_THROW(
        { db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 100.0}}, time_series); });

    // Verify no time series rows
    auto ts_result = db().execute("SELECT COUNT(*) FROM Plant_time_series_generation WHERE id = 1");
    EXPECT_EQ(ts_result[0].get_int(0).value_or(-1), 0);
}

// Time series columns must have same length
TEST_F(CreateTimeSeriesTest, TimeSeriesColumnsMustHaveSameLength) {
    db().create_element("Configuration", {{"label", std::string("Toy Case")}});

    psr::TimeSeries generation;
    generation["date_time"] = {std::string("2020-01-01"), std::string("2020-01-02")};
    generation["block"] = {int64_t(1)};  // Mismatched length!
    generation["generation"] = {100.0, 120.0};

    std::map<std::string, psr::TimeSeries> time_series;
    time_series["generation"] = generation;

    EXPECT_THROW(
        { db().create_element("Plant", {{"label", std::string("Plant 1")}, {"capacity", 200.0}}, time_series); },
        std::runtime_error);
}

// =============================================================================
// get_element_id Tests
// =============================================================================

class GetElementIdTest : public ::testing::Test {
protected:
    void SetUp() override {
        helper_ = std::make_unique<TestDatabaseHelper>("test_create_parameters.sql");
        helper_->open();
    }

    void TearDown() override { helper_.reset(); }

    psr::Database& db() { return helper_->db(); }

private:
    std::unique_ptr<TestDatabaseHelper> helper_;
};

TEST_F(GetElementIdTest, ReturnsCorrectId) {
    db().create_element("Configuration", {{"label", std::string("Config 1")}});
    db().create_element("Resource", {{"label", std::string("Resource A")}});
    db().create_element("Resource", {{"label", std::string("Resource B")}});

    EXPECT_EQ(db().get_element_id("Configuration", "Config 1"), 1);
    EXPECT_EQ(db().get_element_id("Resource", "Resource A"), 1);
    EXPECT_EQ(db().get_element_id("Resource", "Resource B"), 2);
}

TEST_F(GetElementIdTest, ThrowsOnNotFound) {
    EXPECT_THROW(db().get_element_id("Configuration", "Nonexistent"), std::runtime_error);
}

TEST_F(GetElementIdTest, ThrowsOnInvalidTable) {
    EXPECT_THROW(db().get_element_id("NonexistentTable", "Label"), std::runtime_error);
}
