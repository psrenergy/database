#ifndef PSR_COLUMN_TYPE_H
#define PSR_COLUMN_TYPE_H

#include <stdexcept>
#include <string>

namespace psr {

enum class ColumnType { Integer, Real, Text, Blob };

inline ColumnType column_type_from_string(const std::string& type_str) {
    if (type_str == "INTEGER")
        return ColumnType::Integer;
    if (type_str == "REAL")
        return ColumnType::Real;
    if (type_str == "TEXT")
        return ColumnType::Text;
    if (type_str == "BLOB")
        return ColumnType::Blob;
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
    case ColumnType::Blob:
        return "BLOB";
    }
    return "UNKNOWN";
}

}  // namespace psr

#endif  // PSR_COLUMN_TYPE_H
