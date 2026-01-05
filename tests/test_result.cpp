#include <gtest/gtest.h>
#include <psr/database.h>
#include <vector>

TEST(ResultTest, EmptyResult) {
    psr::Result result;
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.row_count(), 0u);
    EXPECT_EQ(result.column_count(), 0u);
}

TEST(ResultTest, WithData) {
    std::vector<std::string> columns = {"id", "name", "value"};

    std::vector<psr::Row> rows;
    rows.emplace_back(std::vector<psr::Value>{int64_t{1}, std::string{"test"}, 3.14});
    rows.emplace_back(std::vector<psr::Value>{int64_t{2}, std::string{"example"}, 2.71});

    psr::Result result(columns, std::move(rows));

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.row_count(), 2u);
    EXPECT_EQ(result.column_count(), 3u);

    EXPECT_EQ(result.columns()[0], "id");
    EXPECT_EQ(result.columns()[1], "name");
    EXPECT_EQ(result.columns()[2], "value");
}

TEST(RowTest, GetValues) {
    std::vector<psr::Value> values = {int64_t{42}, std::string{"hello"}, 3.14, nullptr};
    psr::Row row(std::move(values));

    EXPECT_EQ(row.column_count(), 4u);

    EXPECT_TRUE(row.get_int(0).has_value());
    EXPECT_EQ(row.get_int(0).value(), 42);
    EXPECT_TRUE(row.get_string(1).has_value());
    EXPECT_EQ(row.get_string(1).value(), "hello");
    EXPECT_TRUE(row.get_double(2).has_value());
    EXPECT_DOUBLE_EQ(row.get_double(2).value(), 3.14);
    EXPECT_TRUE(row.is_null(3));
}

TEST(RowTest, WrongType) {
    std::vector<psr::Value> values = {std::string{"text"}};
    psr::Row row(std::move(values));

    EXPECT_FALSE(row.get_int(0).has_value());
    EXPECT_TRUE(row.get_string(0).has_value());
}

TEST(RowTest, OutOfRange) {
    std::vector<psr::Value> values = {int64_t{1}};
    psr::Row row(std::move(values));

    EXPECT_THROW(row.at(10), std::out_of_range);
    EXPECT_TRUE(row.is_null(10));  // Out of range returns true for is_null
}

TEST(RowTest, BlobValue) {
    std::vector<uint8_t> blob = {0xDE, 0xAD, 0xBE, 0xEF};
    std::vector<psr::Value> values = {blob};
    psr::Row row(std::move(values));

    auto result = row.get_blob(0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4u);
    EXPECT_EQ((*result)[0], 0xDE);
    EXPECT_EQ((*result)[3], 0xEF);
}
