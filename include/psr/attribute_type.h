#ifndef PSR_ATTRIBUTE_TYPE_H
#define PSR_ATTRIBUTE_TYPE_H

#include "export.h"

namespace psr {

enum class AttributeStructure { Scalar, Vector, Set };
enum class AttributeDataType { Integer, Real, Text };

struct PSR_API AttributeType {
    AttributeStructure structure;
    AttributeDataType data_type;
};

}  // namespace psr

#endif  // PSR_ATTRIBUTE_TYPE_H
