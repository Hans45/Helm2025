#pragma once

#include <immintrin.h>
#include <array>
#include <algorithm>

namespace mopo {

class ParameterInterpolator {
public:
    // Tailles compatibles avec notre pipeline BLOCK_SIZE
    static constexpr int MAX_BLOCK = 128;

    ParameterInterpolator() noexcept : current_value_(0.0f), target_value_(0.0f), remaining_steps_(0) {}

    // Définit la nouvelle cible et la durée en échantillons
    void setTarget(float target, int duration_samples) noexcept {
        target_value_ = target;
        if (duration_samples <= 0) {
            current_value_ = target;
            remaining_steps_ = 0;
            step_ = 0.0f;
            return;
        }
        remaining_steps_ = duration_samples;
        step_ = (target_value_ - current_value_) / static_cast<float>(duration_samples);
    }

    // Remplit "out" de taille "count" avec les valeurs interpolées et met à jour l'état
    void fillBlock(float* out, int count) noexcept {
        if (count <= 0) return;

        int i = 0;
    // Traitement par vecteurs de 8 pour AVX si disponible, sinon fallback scalaire
#if defined(__AVX__) || defined(__AVX2__)
    for (; i <= count - 8; i += 8) {
        // Charge la valeur courante
        __m256 v = _mm256_set1_ps(current_value_);
        __m256 s = _mm256_set1_ps(step_);
        // Création d'un vecteur [0..7] (0 at lowest lane)
        __m256 idx = _mm256_set_ps(7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m256 incr = _mm256_mul_ps(s, idx);
        __m256 result = _mm256_add_ps(v, incr);
        _mm256_storeu_ps(out + i, result);
    }
#else
    // Fallback simple : remplir scalairement
    for (; i <= count - 1; ++i) {
        out[i] = current_value_ + step_ * static_cast<float>(i);
    }
    // We've filled all requested entries, jump to updating state below.
    i = count;
#endif

        // Restant scalaire
        for (; i < count; ++i) {
            out[i] = current_value_ + step_ * static_cast<float>(i);
        }

        // Avance l'état
        int stepped = std::min(count, remaining_steps_);
        current_value_ += step_ * static_cast<float>(stepped);
        remaining_steps_ = std::max(0, remaining_steps_ - stepped);

        // Si terminé, assure la valeur cible exacte
        if (remaining_steps_ == 0) current_value_ = target_value_;
    }

    float current() const noexcept { return current_value_; }
    bool isActive() const noexcept { return remaining_steps_ > 0; }

private:
    float current_value_;
    float target_value_;
    float step_;
    int remaining_steps_;
};

} // namespace mopo

