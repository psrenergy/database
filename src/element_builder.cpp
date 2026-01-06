#include "psr/element_builder.h"

namespace psr {

ElementBuilder& ElementBuilder::set(const std::string& name, int64_t value) {
    scalars_[name] = value;
    return *this;
}

ElementBuilder& ElementBuilder::set(const std::string& name, double value) {
    scalars_[name] = value;
    return *this;
}

ElementBuilder& ElementBuilder::set(const std::string& name, const std::string& value) {
    scalars_[name] = value;
    return *this;
}

ElementBuilder& ElementBuilder::set_null(const std::string& name) {
    scalars_[name] = nullptr;
    return *this;
}

ElementBuilder& ElementBuilder::set_vector(const std::string& name, std::vector<int64_t> values) {
    vectors_[name] = std::move(values);
    return *this;
}

ElementBuilder& ElementBuilder::set_vector(const std::string& name, std::vector<double> values) {
    vectors_[name] = std::move(values);
    return *this;
}

ElementBuilder& ElementBuilder::set_vector(const std::string& name, std::vector<std::string> values) {
    vectors_[name] = std::move(values);
    return *this;
}

const std::map<std::string, Value>& ElementBuilder::scalars() const {
    return scalars_;
}

const std::map<std::string, Value>& ElementBuilder::vectors() const {
    return vectors_;
}

bool ElementBuilder::has_scalars() const {
    return !scalars_.empty();
}

bool ElementBuilder::has_vectors() const {
    return !vectors_.empty();
}

void ElementBuilder::clear() {
    scalars_.clear();
    vectors_.clear();
}

}  // namespace psr
