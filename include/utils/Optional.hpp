#ifndef CIVICCARE_UTILS_OPTIONAL_HPP
#define CIVICCARE_UTILS_OPTIONAL_HPP

#if defined(__has_include)
  #if __has_include(<optional>) && __cplusplus >= 201703L
    #include <optional>
    #define CIVICCARE_HAS_STD_OPTIONAL 1
  #endif
#endif

#ifdef CIVICCARE_HAS_STD_OPTIONAL
namespace civiccare {
    template <typename T>
    using Optional = std::optional<T>;
    constexpr auto nullopt = std::nullopt;
}
#else
#include <stdexcept>
#include <utility>

namespace civiccare {

struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};
constexpr nullopt_t nullopt{0};

template <typename T>
class Optional {
private:
    bool m_hasValue;
    alignas(T) char m_storage[sizeof(T)];

    T* ptr() noexcept { return reinterpret_cast<T*>(m_storage); }
    const T* ptr() const noexcept { return reinterpret_cast<const T*>(m_storage); }

public:
    constexpr Optional() noexcept : m_hasValue(false) {}
    constexpr Optional(nullopt_t) noexcept : m_hasValue(false) {}

    Optional(const T& value) : m_hasValue(true) {
        new (m_storage) T(value);
    }

    Optional(T&& value) : m_hasValue(true) {
        new (m_storage) T(std::move(value));
    }

    Optional(const Optional& other) : m_hasValue(other.m_hasValue) {
        if (other.m_hasValue) {
            new (m_storage) T(*other.ptr());
        }
    }

    Optional(Optional&& other) noexcept : m_hasValue(other.m_hasValue) {
        if (other.m_hasValue) {
            new (m_storage) T(std::move(*other.ptr()));
            other.reset();
        }
    }

    ~Optional() {
        reset();
    }

    Optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    Optional& operator=(const Optional& other) {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_storage) T(*other.ptr());
                m_hasValue = true;
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& other) noexcept {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_storage) T(std::move(*other.ptr()));
                m_hasValue = true;
                other.reset();
            }
        }
        return *this;
    }

    void reset() noexcept {
        if (m_hasValue) {
            ptr()->~T();
            m_hasValue = false;
        }
    }

    constexpr bool has_value() const noexcept { return m_hasValue; }
    constexpr explicit operator bool() const noexcept { return m_hasValue; }

    T& value() {
        if (!m_hasValue) throw std::logic_error("Bad optional access");
        return *ptr();
    }

    const T& value() const {
        if (!m_hasValue) throw std::logic_error("Bad optional access");
        return *ptr();
    }

    T& operator*() { return *ptr(); }
    const T& operator*() const { return *ptr(); }

    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }

    template <typename U>
    T value_or(U&& defaultValue) const {
        return m_hasValue ? *ptr() : static_cast<T>(std::forward<U>(defaultValue));
    }
};

} // namespace civiccare
#endif

#endif // CIVICCARE_UTILS_OPTIONAL_HPP
