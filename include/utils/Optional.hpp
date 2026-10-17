#pragma once

#if defined(__has_include)
    #if __has_include(<optional>) && (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
        #include <optional>
        namespace CivicCare {
            template <typename T>
            using Optional = std::optional<T>;
            constexpr auto NullOpt = std::nullopt;
        }
        #define CIVICCARE_HAS_STD_OPTIONAL 1
    #endif
#endif

#ifndef CIVICCARE_HAS_STD_OPTIONAL
#include <stdexcept>
#include <utility>

namespace CivicCare {

struct NullOptTag {};
constexpr NullOptTag NullOpt{};

/**
 * @brief Lightweight, zero-overhead C++14/C++17 compatible Optional template.
 * Allows Member 3 code to compile on any standard C++ compiler (including GCC 6.x to GCC 14+).
 */
template <typename T>
class Optional {
private:
    bool hasValue_ = false;
    T value_{};

public:
    Optional() : hasValue_(false), value_{} {}
    Optional(NullOptTag) : hasValue_(false), value_{} {}
    Optional(const T& val) : hasValue_(true), value_(val) {}
    Optional(T&& val) : hasValue_(true), value_(std::move(val)) {}

    Optional& operator=(NullOptTag) {
        hasValue_ = false;
        value_ = T{};
        return *this;
    }

    Optional& operator=(const T& val) {
        hasValue_ = true;
        value_ = val;
        return *this;
    }

    Optional& operator=(T&& val) {
        hasValue_ = true;
        value_ = std::move(val);
        return *this;
    }

    bool has_value() const noexcept { return hasValue_; }
    explicit operator bool() const noexcept { return hasValue_; }

    const T& value() const {
        if (!hasValue_) {
            throw std::runtime_error("Bad optional access: no value present");
        }
        return value_;
    }

    T& value() {
        if (!hasValue_) {
            throw std::runtime_error("Bad optional access: no value present");
        }
        return value_;
    }

    const T& value_or(const T& defaultVal) const {
        return hasValue_ ? value_ : defaultVal;
    }

    const T& operator*() const { return value(); }
    T& operator*() { return value(); }
    const T* operator->() const { return &value(); }
    T* operator->() { return &value(); }

    bool operator==(NullOptTag) const noexcept { return !hasValue_; }
    bool operator!=(NullOptTag) const noexcept { return hasValue_; }

    bool operator==(const T& otherVal) const noexcept {
        return hasValue_ && (value_ == otherVal);
    }
    bool operator!=(const T& otherVal) const noexcept {
        return !(*this == otherVal);
    }
};

} // namespace CivicCare
#endif
