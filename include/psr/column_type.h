#ifndef PSR_COLUMN_TYPE_H
#define PSR_COLUMN_TYPE_H

#include <stdexcept>
#include <string>

namespace psr {

enum class ColumnType { Integer, Real, Text };

inline ColumnType column_type_from_string(const std::string& type_str) {
    if (type_str == "INTEGER")
        return ColumnType::Integer;
    else if (type_str == "REAL")
        return ColumnType::Real;
    else if (type_str == "TEXT")
        return ColumnType::Text;
    throw std::runtime_error("Unknown column type: " + type_str);
}

inline const char* column_type_to_string(ColumnType type) {
    switch (type) {
    case ColumnType::Integer:
        return "INTEGER";
    case ColumnType::Real:
        return "REAL";
    case ColumnType::Text:
        return "TEXT";
    }
    return "UNKNOWN";
}

}  // namespace psr

#endif  // PSR_COLUMN_TYPE_H
