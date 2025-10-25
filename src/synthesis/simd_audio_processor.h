#pragma once

#include <cmath>
#include <immintrin.h>

namespace mopo {

class SimdAudioProcessor {
public:
    // Traitement vectorisé des blocs audio
    static inline void processAudioBlock(float* buffer, int size) noexcept {
        // Traitement par blocs de 8 échantillons (AVX)
        int i = 0;
        for (; i <= size - 8; i += 8) {
            __m256 samples = _mm256_loadu_ps(buffer + i);
            samples = processAVX(samples);
            _mm256_storeu_ps(buffer + i, samples);
        }
        
        // Traitement des échantillons restants
        for (; i < size; ++i) {
            buffer[i] = processSample(buffer[i]);
        }
    }

    // Mixage vectorisé de plusieurs voix
    static inline void mixVoices(float** voices, int numVoices, float* output, int size) noexcept {
        // Initialisation du buffer de sortie à zéro
        __m256 sum = _mm256_setzero_ps();
        
        for (int i = 0; i <= size - 8; i += 8) {
            sum = _mm256_setzero_ps();
            
            // Accumulation des voix
            for (int v = 0; v < numVoices; ++v) {
                __m256 voice = _mm256_loadu_ps(voices[v] + i);
                sum = _mm256_add_ps(sum, voice);
            }
            
            // Normalisation
            __m256 scale = _mm256_set1_ps(1.0f / numVoices);
            sum = _mm256_mul_ps(sum, scale);
            
            _mm256_storeu_ps(output + i, sum);
        }
    }

    // Application vectorisée d'une enveloppe
    static inline void applyEnvelope(float* buffer, const float* envelope, int size) noexcept {
        int i = 0;
        for (; i <= size - 8; i += 8) {
            __m256 samples = _mm256_loadu_ps(buffer + i);
            __m256 env = _mm256_loadu_ps(envelope + i);
            __m256 result = _mm256_mul_ps(samples, env);
            _mm256_storeu_ps(buffer + i, result);
        }
        
        for (; i < size; ++i) {
            buffer[i] *= envelope[i];
        }
    }

private:
    static inline __m256 processAVX(__m256 input) noexcept {
        // Exemple de traitement : soft clipping
        __m256 one = _mm256_set1_ps(1.0f);
        __m256 neg_one = _mm256_set1_ps(-1.0f);
        
        // max(-1, min(1, input))
        input = _mm256_min_ps(_mm256_max_ps(input, neg_one), one);
        
        return input;
    }

    static inline float processSample(float input) noexcept {
        // Version scalaire du même traitement
        return std::max(-1.0f, std::min(1.0f, input));
    }
};

} // namespace mopo