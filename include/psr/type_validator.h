#ifndef PSR_TYPE_VALIDATOR_H
#define PSR_TYPE_VALIDATOR_H

#include "column_type.h"
#include "export.h"
#include "schema.h"
#include "value.h"

#include <string>

namespace psr {

class PSR_API TypeValidator {
public:
    explicit TypeValidator(const Schema& schema);

    // Validate a scalar value against a column in a table
    // Throws std::runtime_error on mismatch
    void validate_scalar(const std::string& table, const std::string& column, const Value& value) const;

    // Validate a vector value against the expected element type
    // Automatically determines vector table name from collection + attr
    void validate_vector(const std::string& collection, const std::string& attr_name, const Value& vector_value) const;

    // Low-level: validate value against explicit type
    static void validate_value(const std::string& context, ColumnType expected_type, const Value& value);

private:
    const Schema& schema_;

    // Helper to get expected type for vector elements
    ColumnType get_vector_element_type(const std::string& collection, const std::string& attr_name) const;
};

}  // namespace psr

#endif  // PSR_TYPE_VALIDATOR_H
