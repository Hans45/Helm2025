#pragma once

#include <array>
#include <cmath>
#include <memory>

namespace mopo {

class WaveformCache {
public:
    static constexpr int TABLE_SIZE = 4096;
    static constexpr int ALIGNEMENT = 32; // Pour AVX

    enum WaveformType {
        Sine,
        Square,
        Saw,
        Triangle,
        NumWaveforms
    };

    WaveformCache() {
        initializeTables();
    }

    // Récupération d'une forme d'onde alignée pour SIMD
    const float* getWaveform(WaveformType type) const noexcept {
        return waveforms_[type].get();
    }

private:
    void initializeTables() {
        // Allocation alignée pour chaque forme d'onde
        for (int i = 0; i < NumWaveforms; ++i) {
            waveforms_[i] = std::unique_ptr<float[], AlignedDeleter>(
                static_cast<float*>(aligned_alloc(ALIGNEMENT, TABLE_SIZE * sizeof(float)))
            );
        }

        // Initialisation des formes d'onde
        generateSine(waveforms_[Sine].get());
        generateSquare(waveforms_[Square].get());
        generateSaw(waveforms_[Saw].get());
        generateTriangle(waveforms_[Triangle].get());
    }

    void generateSine(float* table) noexcept {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            float phase = (2.0f * M_PI * i) / TABLE_SIZE;
            table[i] = std::sin(phase);
        }
    }

    void generateSquare(float* table) noexcept {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = (i < TABLE_SIZE/2) ? 1.0f : -1.0f;
        }
    }

    void generateSaw(float* table) noexcept {
        float increment = 2.0f / TABLE_SIZE;
        float value = -1.0f;
        
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = value;
            value += increment;
        }
    }

    void generateTriangle(float* table) noexcept {
        float increment = 4.0f / TABLE_SIZE;
        float value = -1.0f;
        
        for (int i = 0; i < TABLE_SIZE/2; ++i) {
            table[i] = value;
            value += increment;
        }
        
        value = 1.0f;
        for (int i = TABLE_SIZE/2; i < TABLE_SIZE; ++i) {
            table[i] = value;
            value -= increment;
        }
    }

    // Gestionnaire de mémoire alignée
    struct AlignedDeleter {
        void operator()(void* p) const {
            free(p);
        }
    };

    std::array<std::unique_ptr<float[], AlignedDeleter>, NumWaveforms> waveforms_;
};

} // namespace mopo