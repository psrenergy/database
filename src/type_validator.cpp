#include "psr/type_validator.h"

#include <stdexcept>

namespace psr {

TypeValidator::TypeValidator(const Schema& schema) : schema_(schema) {}

void TypeValidator::validate_scalar(const std::string& table, const std::string& column, const Value& value) const {
    ColumnType expected = schema_.get_column_type(table, column);
    validate_value("column '" + column + "'", expected, value);
}

void TypeValidator::validate_vector(const std::string& collection,
                                    const std::string& attr_name,
                                    const Value& vector_value) const {
    ColumnType expected = get_vector_element_type(collection, attr_name);
    validate_value("vector '" + attr_name + "'", expected, vector_value);
}

ColumnType TypeValidator::get_vector_element_type(const std::string& collection, const std::string& attr_name) const {
    std::string vector_table = Schema::vector_table_name(collection, attr_name);
    const auto* table = schema_.get_table(vector_table);
    if (!table) {
        throw std::runtime_error("Vector table not found: " + vector_table);
    }

    // Find the value column (not id, not vector_index)
    for (const auto& [name, col] : table->columns) {
        if (name != "id" && name != "vector_index") {
            return col.type;
        }
    }

    throw std::runtime_error("Vector table '" + vector_table + "' has no value column");
}

void TypeValidator::validate_value(const std::string& context, ColumnType expected_type, const Value& value) {
    std::visit(
        [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                // NULL allowed for any type
                return;
            } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                // BLOB allowed for any type
                return;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                if (expected_type != ColumnType::Integer) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got INTEGER");
                }
            } else if constexpr (std::is_same_v<T, double>) {
                // REAL can be stored in INTEGER or REAL columns
                if (expected_type != ColumnType::Real && expected_type != ColumnType::Integer) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got REAL");
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (expected_type != ColumnType::Text) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got TEXT");
                }
            } else if constexpr (std::is_same_v<T, std::vector<int64_t>>) {
                if (expected_type != ColumnType::Integer) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got INTEGER[]");
                }
            } else if constexpr (std::is_same_v<T, std::vector<double>>) {
                if (expected_type != ColumnType::Real && expected_type != ColumnType::Integer) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got REAL[]");
                }
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                if (expected_type != ColumnType::Text) {
                    throw std::runtime_error("Type mismatch for " + context + ": expected " +
                                             column_type_to_string(expected_type) + ", got TEXT[]");
                }
            }
        },
        value);
}

}  // namespace psr
