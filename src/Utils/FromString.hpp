#ifndef UTILS_FROMSTRING_HPP
#define UTILS_FROMSTRING_HPP

#include <stdexcept>
#include <string_view>

template<typename T>
constexpr T fromString(const std::string_view &value) {
    throw std::runtime_error("not implemented");
}

#endif // UTILS_FROMSTRING_HPP
