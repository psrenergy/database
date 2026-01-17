#include "psr/error.h"

#include <sstream>

namespace psr {

std::string Error::to_string() const {
    std::ostringstream oss;

    // Error code name
    oss << "Error(";
    switch (code) {
    case ErrorCode::Success:
        oss << "Success";
        break;
    case ErrorCode::NoSchemaLoaded:
        oss << "NoSchemaLoaded";
        break;
    case ErrorCode::CollectionNotFound:
        oss << "CollectionNotFound";
        break;
    case ErrorCode::AttributeNotFound:
        oss << "AttributeNotFound";
        break;
    case ErrorCode::InvalidSchema:
        oss << "InvalidSchema";
        break;
    case ErrorCode::TypeMismatch:
        oss << "TypeMismatch";
        break;
    case ErrorCode::InvalidType:
        oss << "InvalidType";
        break;
    case ErrorCode::ElementNotFound:
        oss << "ElementNotFound";
        break;
    case ErrorCode::DuplicateElement:
        oss << "DuplicateElement";
        break;
    case ErrorCode::EmptyElement:
        oss << "EmptyElement";
        break;
    case ErrorCode::ConstraintViolation:
        oss << "ConstraintViolation";
        break;
    case ErrorCode::ForeignKeyViolation:
        oss << "ForeignKeyViolation";
        break;
    case ErrorCode::UniqueViolation:
        oss << "UniqueViolation";
        break;
    case ErrorCode::NotNullViolation:
        oss << "NotNullViolation";
        break;
    case ErrorCode::SqlError:
        oss << "SqlError";
        break;
    case ErrorCode::SqlSyntaxError:
        oss << "SqlSyntaxError";
        break;
    case ErrorCode::FileNotFound:
        oss << "FileNotFound";
        break;
    case ErrorCode::PermissionDenied:
        oss << "PermissionDenied";
        break;
    case ErrorCode::DiskFull:
        oss << "DiskFull";
        break;
    case ErrorCode::InvalidIdentifier:
        oss << "InvalidIdentifier";
        break;
    case ErrorCode::InvalidValue:
        oss << "InvalidValue";
        break;
    case ErrorCode::InternalError:
        oss << "InternalError";
        break;
    case ErrorCode::NotImplemented:
        oss << "NotImplemented";
        break;
    default:
        oss << "Unknown(" << static_cast<int>(code) << ")";
        break;
    }

    oss << "): " << message;

    if (!context.empty()) {
        oss << " [" << context << "]";
    }

    return oss.str();
}

}  // namespace psr
