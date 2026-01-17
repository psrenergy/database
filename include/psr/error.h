#ifndef PSR_DATABASE_ERROR_H
#define PSR_DATABASE_ERROR_H

#include "export.h"

#include <string>
#include <system_error>

namespace psr {

/**
 * @brief Error codes for database operations
 */
enum class ErrorCode {
    Success = 0,

    // Schema errors
    NoSchemaLoaded = 1,
    CollectionNotFound = 2,
    AttributeNotFound = 3,
    InvalidSchema = 4,

    // Type errors
    TypeMismatch = 10,
    InvalidType = 11,

    // Element errors
    ElementNotFound = 20,
    DuplicateElement = 21,
    EmptyElement = 22,

    // Constraint errors
    ConstraintViolation = 30,
    ForeignKeyViolation = 31,
    UniqueViolation = 32,
    NotNullViolation = 33,

    // SQL errors
    SqlError = 40,
    SqlSyntaxError = 41,

    // IO errors
    FileNotFound = 50,
    PermissionDenied = 51,
    DiskFull = 52,

    // Validation errors
    InvalidIdentifier = 60,
    InvalidValue = 61,

    // Internal errors
    InternalError = 100,
    NotImplemented = 101,
};

/**
 * @brief Error information for database operations
 */
struct PSR_API Error {
    ErrorCode code;
    std::string message;
    std::string context;  // Additional context (e.g., collection name, attribute)

    Error() : code(ErrorCode::Success), message(), context() {}

    Error(ErrorCode c, std::string msg) : code(c), message(std::move(msg)), context() {}

    Error(ErrorCode c, std::string msg, std::string ctx)
        : code(c), message(std::move(msg)), context(std::move(ctx)) {}

    bool is_success() const { return code == ErrorCode::Success; }
    bool is_error() const { return code != ErrorCode::Success; }

    std::string to_string() const;
};

/**
 * @brief Result type for operations that can fail
 * 
 * This provides a type-safe way to return either a value or an error,
 * without the overhead of exceptions.
 * 
 * @tparam T The success value type
 */
template <typename T>
class Result {
public:
    // Factory methods
    static Result<T> Ok(T value) {
        Result<T> r;
        r.has_value_ = true;
        new (&r.storage_.value) T(std::move(value));
        return r;
    }

    static Result<T> Err(Error error) {
        Result<T> r;
        r.has_value_ = false;
        new (&r.storage_.error) Error(std::move(error));
        return r;
    }

    static Result<T> Err(ErrorCode code, std::string message, std::string context = "") {
        return Err(Error(code, std::move(message), std::move(context)));
    }

    // Destructor
    ~Result() {
        if (has_value_) {
            storage_.value.~T();
        } else {
            storage_.error.~Error();
        }
    }

    // Copy constructor
    Result(const Result& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_.value) T(other.storage_.value);
        } else {
            new (&storage_.error) Error(other.storage_.error);
        }
    }

    // Move constructor
    Result(Result&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_.value) T(std::move(other.storage_.value));
        } else {
            new (&storage_.error) Error(std::move(other.storage_.error));
        }
    }

    // Copy assignment
    Result& operator=(const Result& other) {
        if (this != &other) {
            this->~Result();
            new (this) Result(other);
        }
        return *this;
    }

    // Move assignment
    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            this->~Result();
            new (this) Result(std::move(other));
        }
        return *this;
    }

    // Status queries
    bool is_ok() const { return has_value_; }
    bool is_err() const { return !has_value_; }
    explicit operator bool() const { return is_ok(); }

    // Value access (throws if error)
    const T& value() const& {
        if (!has_value_) {
            throw std::runtime_error("Result::value() called on error: " + storage_.error.to_string());
        }
        return storage_.value;
    }

    T& value() & {
        if (!has_value_) {
            throw std::runtime_error("Result::value() called on error: " + storage_.error.to_string());
        }
        return storage_.value;
    }

    T&& value() && {
        if (!has_value_) {
            throw std::runtime_error("Result::value() called on error: " + storage_.error.to_string());
        }
        return std::move(storage_.value);
    }

    // Error access (throws if success)
    const Error& error() const {
        if (has_value_) {
            throw std::runtime_error("Result::error() called on success");
        }
        return storage_.error;
    }

    // Safe access with default
    T value_or(T default_value) const& {
        return has_value_ ? storage_.value : default_value;
    }

    T value_or(T default_value) && {
        return has_value_ ? std::move(storage_.value) : default_value;
    }

    // Unwrap (throws descriptive error on failure)
    T unwrap() && {
        if (!has_value_) {
            throw std::runtime_error("Result::unwrap() failed: " + storage_.error.to_string());
        }
        return std::move(storage_.value);
    }

    // Expect (throws custom message on failure)
    T expect(const std::string& message) && {
        if (!has_value_) {
            throw std::runtime_error(message + ": " + storage_.error.to_string());
        }
        return std::move(storage_.value);
    }

private:
    Result() = default;

    bool has_value_;
    union Storage {
        T value;
        Error error;

        Storage() {}
        ~Storage() {}
    } storage_;
};

// Specialization for void
template <>
class Result<void> {
public:
    static Result<void> Ok() {
        Result<void> r;
        r.has_value_ = true;
        return r;
    }

    static Result<void> Err(Error error) {
        Result<void> r;
        r.has_value_ = false;
        r.error_ = std::move(error);
        return r;
    }

    static Result<void> Err(ErrorCode code, std::string message, std::string context = "") {
        return Err(Error(code, std::move(message), std::move(context)));
    }

    bool is_ok() const { return has_value_; }
    bool is_err() const { return !has_value_; }
    explicit operator bool() const { return is_ok(); }

    const Error& error() const {
        if (has_value_) {
            throw std::runtime_error("Result::error() called on success");
        }
        return error_;
    }

    void unwrap() const {
        if (!has_value_) {
            throw std::runtime_error("Result::unwrap() failed: " + error_.to_string());
        }
    }

    void expect(const std::string& message) const {
        if (!has_value_) {
            throw std::runtime_error(message + ": " + error_.to_string());
        }
    }

private:
    Result() = default;

    bool has_value_;
    Error error_;
};

}  // namespace psr

#endif  // PSR_DATABASE_ERROR_H
