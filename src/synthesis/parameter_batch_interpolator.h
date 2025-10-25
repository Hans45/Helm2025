#pragma once

#include <immintrin.h>
#include <array>
#include <algorithm>

namespace mopo {

// Batch interpolator for multiple parameters (SOA layout)
// Supports up to MAX_PARAMS parameters interpolated in parallel per block.
class ParameterBatchInterpolator {
public:
    static constexpr int MAX_BLOCK = 128;
    static constexpr int MAX_PARAMS = 8; // number of simultaneous params we handle

    ParameterBatchInterpolator() noexcept {
        for (int p = 0; p < MAX_PARAMS; ++p) {
            current_values_[p] = 0.0f;
            target_values_[p] = 0.0f;
            steps_[p] = 0.0f;
            remaining_[p] = 0;
        }
    }

    // Set target for a parameter index with given duration in samples
    void setTarget(int param_index, float target, int duration_samples) noexcept {
        if (param_index < 0 || param_index >= MAX_PARAMS) return;
        target_values_[param_index] = target;
        if (duration_samples <= 0) {
            current_values_[param_index] = target;
            remaining_[param_index] = 0;
            steps_[param_index] = 0.0f;
            return;
        }
        remaining_[param_index] = duration_samples;
        steps_[param_index] = (target_values_[param_index] - current_values_[param_index]) / static_cast<float>(duration_samples);
    }

    // Fill block for all parameters: out is an array of pointers for each parameter (SOA)
    // out[p][i] will be written for param p sample i
    void fillBlockAll(float* out[MAX_PARAMS], int params_count, int count) noexcept {
        if (params_count <= 0) return;
        params_count = std::min(params_count, MAX_PARAMS);

#if defined(__AVX__) || defined(__AVX2__)
        // Build __m256 vectors for current and steps
        __m256 cur[MAX_PARAMS];
        __m256 stepv[MAX_PARAMS];
        for (int p = 0; p < params_count; ++p) {
            cur[p] = _mm256_set1_ps(current_values_[p]);
            stepv[p] = _mm256_set1_ps(steps_[p]);
        }

        int i = 0;
        for (; i <= count - 8; i += 8) {
            // idx vector [0..7]
            __m256 idx = _mm256_set_ps(7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
            __m256 incr = _mm256_mul_ps(stepv[0], idx); // temp reused
            for (int p = 0; p < params_count; ++p) {
                __m256 res = _mm256_add_ps(cur[p], _mm256_mul_ps(stepv[p], idx));
                _mm256_storeu_ps(out[p] + i, res);
            }
        }

        // remainder scalar
        for (int j = i; j < count; ++j) {
            for (int p = 0; p < params_count; ++p) {
                out[p][j] = current_values_[p] + steps_[p] * static_cast<float>(j);
            }
        }
#else
        // scalar fallback
        for (int j = 0; j < count; ++j) {
            for (int p = 0; p < params_count; ++p) {
                out[p][j] = current_values_[p] + steps_[p] * static_cast<float>(j);
            }
        }
#endif

        // Advance internal state
        for (int p = 0; p < params_count; ++p) {
            int stepped = std::min(count, remaining_[p]);
            current_values_[p] += steps_[p] * static_cast<float>(stepped);
            remaining_[p] = std::max(0, remaining_[p] - stepped);
            if (remaining_[p] == 0) current_values_[p] = target_values_[p];
        }
    }

    float currentValue(int p) const noexcept { return (p >= 0 && p < MAX_PARAMS) ? current_values_[p] : 0.0f; }

private:
    float current_values_[MAX_PARAMS];
    float target_values_[MAX_PARAMS];
    float steps_[MAX_PARAMS];
    int remaining_[MAX_PARAMS];
};

} // namespace mopo