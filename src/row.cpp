#include "psr/row.h"

#include "psr/value.h"

namespace psr {

Row::Row(std::vector<Value> values) : values_(std::move(values)) {}

std::size_t Row::size() const {
    return values_.size();
}

bool Row::empty() const {
    return values_.empty();
}

const Value& Row::at(std::size_t index) const {
    return values_.at(index);
}

const Value& Row::operator[](std::size_t index) const {
    return values_[index];
}

bool Row::is_null(std::size_t index) const {
    if (index >= values_.size())
        return true;
    return std::holds_alternative<std::nullptr_t>(values_[index]);
}

std::optional<int64_t> Row::get_int(std::size_t index) const {
    if (index >= values_.size())
        return std::nullopt;
    if (const auto* val = std::get_if<int64_t>(&values_[index])) {
        return *val;
    }
    return std::nullopt;
}

std::optional<double> Row::get_double(std::size_t index) const {
    if (index >= values_.size())
        return std::nullopt;
    if (const auto* val = std::get_if<double>(&values_[index])) {
        return *val;
    }
    return std::nullopt;
}

std::optional<std::string> Row::get_string(std::size_t index) const {
    if (index >= values_.size())
        return std::nullopt;
    if (const auto* val = std::get_if<std::string>(&values_[index])) {
        return *val;
    }
    return std::nullopt;
}

std::optional<std::vector<uint8_t>> Row::get_blob(std::size_t index) const {
    if (index >= values_.size())
        return std::nullopt;
    if (const auto* val = std::get_if<std::vector<uint8_t>>(&values_[index])) {
        return *val;
    }
    return std::nullopt;
}
}  // namespace psr