#pragma once

#include <optional>
#include <memory>
#include <string_view>

namespace mopo {
namespace utils {

// Concepts pour sécuriser les utilitaires
#include <concepts>
template<typename T>
requires std::default_initializable<T>
[[nodiscard]] inline bool isValidPointer(const T* ptr) noexcept {
    return ptr != nullptr;
}

template<typename T>
requires std::totally_ordered<T>
[[nodiscard]] inline bool isValidValue(T value, T min, T max) noexcept {
    return value >= min && value <= max;
}

template<typename T>
requires std::totally_ordered<T>
[[nodiscard]] inline std::optional<T> validateRange(T value, T min, T max) noexcept {
    if (isValidValue(value, min, max)) {
        return value;
    }
    return std::nullopt;
}

template<typename T, typename... Args>
requires std::default_initializable<T>
[[nodiscard]] inline std::unique_ptr<T> makeUnique(Args&&... args) noexcept {
    try {
        return std::make_unique<T>(std::forward<Args>(args)...);
    } catch (...) {
        return nullptr;
    }
}

} // namespace utils
} // namespace mopo

