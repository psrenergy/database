#ifndef PSR_VALIDATION_H
#define PSR_VALIDATION_H

#include "error.h"
#include "export.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <unordered_set>

namespace psr {

/**
 * @brief Input validation utilities for database operations
 * 
 * Provides validation for identifiers, values, and other inputs
 * to prevent SQL injection and ensure data integrity.
 */
class PSR_API Validation {
public:
    /**
     * @brief Validates a SQL identifier (table/column name)
     * 
     * Valid identifiers:
     * - Start with letter or underscore
     * - Contain only letters, numbers, underscores
     * - Length between 1 and 128 characters
     * 
     * @param identifier The identifier to validate
     * @return true if valid, false otherwise
     */
    static bool is_valid_identifier(const std::string& identifier) {
        if (identifier.empty() || identifier.length() > 128) {
            return false;
        }

        // Must start with letter or underscore
        if (!std::isalpha(identifier[0]) && identifier[0] != '_') {
            return false;
        }

        // Rest must be alphanumeric or underscore
        for (char c : identifier) {
            if (!std::isalnum(c) && c != '_') {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Validates and throws on invalid identifier
     * 
     * @param identifier The identifier to validate
     * @param context Context for error message (e.g., "collection", "attribute")
     * @throws std::runtime_error if identifier is invalid
     */
    static void require_valid_identifier(const std::string& identifier, const std::string& context) {
        if (!is_valid_identifier(identifier)) {
            throw std::runtime_error("Invalid " + context + " identifier: '" + identifier +
                                     "'. Must start with letter/underscore and contain only alphanumeric/underscore "
                                     "characters (max 128 chars).");
        }
    }

    /**
     * @brief Returns Result for identifier validation
     */
    static Result<void> validate_identifier(const std::string& identifier, const std::string& context) {
        if (!is_valid_identifier(identifier)) {
            return Result<void>::Err(ErrorCode::InvalidIdentifier,
                                     "Invalid identifier: '" + identifier +
                                         "'. Must start with letter/underscore and contain only "
                                         "alphanumeric/underscore characters (max 128 chars).",
                                     context);
        }
        return Result<void>::Ok();
    }

    /**
     * @brief Validates a database ID
     * 
     * @param id The ID to validate
     * @return true if valid (> 0), false otherwise
     */
    static bool is_valid_id(int64_t id) { return id > 0; }

    /**
     * @brief Validates and throws on invalid ID
     * 
     * @param id The ID to validate
     * @param context Context for error message
     * @throws std::runtime_error if ID is invalid
     */
    static void require_valid_id(int64_t id, const std::string& context) {
        if (!is_valid_id(id)) {
            throw std::runtime_error("Invalid " + context + " ID: " + std::to_string(id) + ". Must be > 0.");
        }
    }

    /**
     * @brief Returns Result for ID validation
     */
    static Result<void> validate_id(int64_t id, const std::string& context) {
        if (!is_valid_id(id)) {
            return Result<void>::Err(ErrorCode::InvalidValue, "Invalid ID: " + std::to_string(id) + ". Must be > 0.",
                                     context);
        }
        return Result<void>::Ok();
    }

    /**
     * @brief Checks if a string is a reserved SQL keyword
     * 
     * @param word The word to check
     * @return true if reserved, false otherwise
     */
    static bool is_reserved_keyword(const std::string& word) {
        // Common SQL keywords that should not be used as identifiers
        static const std::unordered_set<std::string> keywords = {
            "SELECT", "INSERT", "UPDATE", "DELETE", "DROP",   "CREATE", "ALTER",  "TABLE",  "INDEX",  "VIEW",
            "FROM",   "WHERE",  "JOIN",   "INNER",  "OUTER",  "LEFT",   "RIGHT",  "ON",     "AND",    "OR",
            "NOT",    "NULL",   "IS",     "IN",     "LIKE",   "BETWEEN", "EXISTS", "UNION",  "ALL",    "DISTINCT",
            "ORDER",  "BY",     "GROUP",  "HAVING", "LIMIT",  "OFFSET", "ASC",    "DESC",   "AS",     "CASE",
            "WHEN",   "THEN",   "ELSE",   "END",    "BEGIN",  "COMMIT", "ROLLBACK", "PRAGMA", "STRICT"};

        std::string upper = word;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        return keywords.find(upper) != keywords.end();
    }

    /**
     * @brief Validates that identifier is not a reserved keyword
     */
    static Result<void> validate_not_reserved(const std::string& identifier, const std::string& context) {
        if (is_reserved_keyword(identifier)) {
            return Result<void>::Err(ErrorCode::InvalidIdentifier,
                                     "Cannot use reserved SQL keyword as identifier: '" + identifier + "'", context);
        }
        return Result<void>::Ok();
    }
};

}  // namespace psr

#endif  // PSR_VALIDATION_H
