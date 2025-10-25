#pragma once

#include <immintrin.h>
#include <array>
#include <cstdint>

namespace mopo {
namespace simd {

class Vec8f {
public:
    static constexpr size_t Size = 8;

    Vec8f() noexcept : data_(_mm256_setzero_ps()) {}
    explicit Vec8f(float value) noexcept : data_(_mm256_set1_ps(value)) {}
    explicit Vec8f(__m256 data) noexcept : data_(data) {}
    
    Vec8f(const Vec8f&) = default;
    Vec8f& operator=(const Vec8f&) = default;

    // Opérations arithmétiques vectorisées
    [[nodiscard]] Vec8f operator+(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_add_ps(data_, other.data_));
    }

    [[nodiscard]] Vec8f operator-(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_sub_ps(data_, other.data_));
    }

    [[nodiscard]] Vec8f operator*(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_mul_ps(data_, other.data_));
    }

    [[nodiscard]] Vec8f operator/(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_div_ps(data_, other.data_));
    }

    // Opérations de comparaison
    [[nodiscard]] Vec8f min(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_min_ps(data_, other.data_));
    }

    [[nodiscard]] Vec8f max(const Vec8f& other) const noexcept {
        return Vec8f(_mm256_max_ps(data_, other.data_));
    }

    // Fonctions mathématiques optimisées
    [[nodiscard]] Vec8f sqrt() const noexcept {
        return Vec8f(_mm256_sqrt_ps(data_));
    }

    [[nodiscard]] Vec8f abs() const noexcept {
        return Vec8f(_mm256_andnot_ps(_mm256_set1_ps(-0.0f), data_));
    }

    // Chargement/Sauvegarde aligné
    static Vec8f load_aligned(const float* ptr) noexcept {
        return Vec8f(_mm256_load_ps(ptr));
    }

    static Vec8f load_unaligned(const float* ptr) noexcept {
        return Vec8f(_mm256_loadu_ps(ptr));
    }

    void store_aligned(float* ptr) const noexcept {
        _mm256_store_ps(ptr, data_);
    }

    void store_unaligned(float* ptr) const noexcept {
        _mm256_storeu_ps(ptr, data_);
    }

    // Conversion vers tableau standard
    [[nodiscard]] std::array<float, Size> to_array() const noexcept {
        std::array<float, Size> result;
        store_unaligned(result.data());
        return result;
    }

    // Opérations spéciales pour le traitement audio
    [[nodiscard]] Vec8f interpolate(const Vec8f& target, const Vec8f& amount) const noexcept {
        return *this + (target - *this) * amount;
    }

    [[nodiscard]] Vec8f tanh_approx() const noexcept {
        // Approximation rapide de tanh utilisant une fonction rationnelle
        const auto x2 = _mm256_mul_ps(data_, data_);
        const auto a = _mm256_add_ps(data_, _mm256_mul_ps(data_, x2));
        const auto b = _mm256_add_ps(_mm256_set1_ps(1.0f), x2);
        return Vec8f(_mm256_div_ps(a, b));
    }

private:
    __m256 data_;
};

// Pool d'allocation alignée pour les tableaux audio
template<typename T, size_t Alignment = 32>
class AlignedAudioBuffer {
public:
    explicit AlignedAudioBuffer(size_t size) 
        : size_(size)
        , data_(static_cast<T*>(_mm_malloc(size * sizeof(T), Alignment))) {}

    ~AlignedAudioBuffer() {
        if (data_) {
            _mm_free(data_);
        }
    }

    AlignedAudioBuffer(const AlignedAudioBuffer&) = delete;
    AlignedAudioBuffer& operator=(const AlignedAudioBuffer&) = delete;

    AlignedAudioBuffer(AlignedAudioBuffer&& other) noexcept
        : size_(other.size_)
        , data_(other.data_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    [[nodiscard]] T* data() noexcept { return data_; }
    [[nodiscard]] const T* data() const noexcept { return data_; }
    [[nodiscard]] size_t size() const noexcept { return size_; }

private:
    size_t size_;
    T* data_;
};

} // namespace simd
} // namespace mopo