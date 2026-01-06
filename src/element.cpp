#include "psr/element.h"

namespace psr {

Element& Element::set(const std::string& name, int64_t value) {
    scalars_[name] = value;
    return *this;
}

Element& Element::set(const std::string& name, double value) {
    scalars_[name] = value;
    return *this;
}

Element& Element::set(const std::string& name, const std::string& value) {
    scalars_[name] = value;
    return *this;
}

Element& Element::set_null(const std::string& name) {
    scalars_[name] = nullptr;
    return *this;
}

Element& Element::set_vector(const std::string& name, std::vector<int64_t> values) {
    vectors_[name] = std::move(values);
    return *this;
}

Element& Element::set_vector(const std::string& name, std::vector<double> values) {
    vectors_[name] = std::move(values);
    return *this;
}

Element& Element::set_vector(const std::string& name, std::vector<std::string> values) {
    vectors_[name] = std::move(values);
    return *this;
}

const std::map<std::string, Value>& Element::scalars() const {
    return scalars_;
}

const std::map<std::string, Value>& Element::vectors() const {
    return vectors_;
}

bool Element::has_scalars() const {
    return !scalars_.empty();
}

bool Element::has_vectors() const {
    return !vectors_.empty();
}

void Element::clear() {
    scalars_.clear();
    vectors_.clear();
}

}  // namespace psr
