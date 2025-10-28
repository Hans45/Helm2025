#pragma once

#include <immintrin.h>

namespace mopo {

class SimdFilterProcessor {
public:
    static constexpr int VECTOR_SIZE = 8; // AVX

    // Filtre passe-bas vectorisé
    static inline void processLowPass(float* buffer, int size, float cutoff, float resonance) noexcept {
        __m256 feedback = _mm256_set1_ps(resonance * 0.95f);
        __m256 freq = _mm256_set1_ps(2.0f * std::sin(M_PI * cutoff / 44100.0f));
        
        __m256 low1 = _mm256_setzero_ps();
        __m256 low2 = _mm256_setzero_ps();
        __m256 band1 = _mm256_setzero_ps();
        __m256 band2 = _mm256_setzero_ps();
        
        int i = 0;
        for (; i <= size - VECTOR_SIZE; i += VECTOR_SIZE) {
            __m256 input = _mm256_loadu_ps(buffer + i);
            
            // Calcul du filtre
            __m256 high = _mm256_sub_ps(input, 
                _mm256_add_ps(
                    _mm256_mul_ps(feedback, band1),
                    low1
                )
            );
            
            band2 = _mm256_add_ps(band1,
                _mm256_mul_ps(freq, high)
            );
            
            low2 = _mm256_add_ps(low1,
                _mm256_mul_ps(freq, band1)
            );
            
            // Stockage des résultats
            _mm256_storeu_ps(buffer + i, low2);
            
            // Mise à jour des états
            low1 = low2;
            band1 = band2;
        }
        
        // Traitement des échantillons restants
        for (; i < size; ++i) {
            float high = buffer[i] - (feedback[0] * band1[0] + low1[0]);
            float band = band1[0] + (freq[0] * high);
            float low = low1[0] + (freq[0] * band1[0]);
            
            buffer[i] = low;
            
            low1[0] = low;
            band1[0] = band;
        }
    }

    // Filtre passe-haut vectorisé
    static inline void processHighPass(float* buffer, int size, float cutoff, float resonance) noexcept {
        __m256 feedback = _mm256_set1_ps(resonance * 0.95f);
        __m256 freq = _mm256_set1_ps(2.0f * std::sin(M_PI * cutoff / 44100.0f));
        
        __m256 high1 = _mm256_setzero_ps();
        __m256 high2 = _mm256_setzero_ps();
        __m256 band1 = _mm256_setzero_ps();
        __m256 band2 = _mm256_setzero_ps();
        
        int i = 0;
        for (; i <= size - VECTOR_SIZE; i += VECTOR_SIZE) {
            __m256 input = _mm256_loadu_ps(buffer + i);
            
            band2 = _mm256_add_ps(band1,
                _mm256_mul_ps(freq,
                    _mm256_sub_ps(input, band1)
                )
            );
            
            high2 = _mm256_sub_ps(input,
                _mm256_add_ps(
                    _mm256_mul_ps(feedback, band2),
                    low1
                )
            );
            
            _mm256_storeu_ps(buffer + i, high2);
            
            high1 = high2;
            band1 = band2;
        }
    }

    // Filtre passe-bande vectorisé
    static inline void processBandPass(float* buffer, int size, float cutoff, float bandwidth) noexcept {
        __m256 freq = _mm256_set1_ps(2.0f * std::sin(M_PI * cutoff / 44100.0f));
        __m256 bw = _mm256_set1_ps(bandwidth);
        
        __m256 band1 = _mm256_setzero_ps();
        __m256 band2 = _mm256_setzero_ps();
        
        int i = 0;
        for (; i <= size - VECTOR_SIZE; i += VECTOR_SIZE) {
            __m256 input = _mm256_loadu_ps(buffer + i);
            
            band2 = _mm256_add_ps(
                _mm256_mul_ps(
                    _mm256_sub_ps(_mm256_set1_ps(1.0f), bw),
                    band1
                ),
                _mm256_mul_ps(freq,
                    _mm256_sub_ps(input, band1)
                )
            );
            
            _mm256_storeu_ps(buffer + i, band2);
            
            band1 = band2;
        }
    }

private:
    static inline __m256 processSVF(__m256 input, __m256& low, __m256& band,
                                   __m256 freq, __m256 damp) noexcept {
        __m256 high = _mm256_sub_ps(input, 
            _mm256_add_ps(low, _mm256_mul_ps(damp, band)));
            
        band = _mm256_add_ps(band, _mm256_mul_ps(freq, high));
        low = _mm256_add_ps(low, _mm256_mul_ps(freq, band));
        
        return high;
    }
};

} // namespace mopo

