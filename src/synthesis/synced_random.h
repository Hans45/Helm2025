#pragma once
#include <vector>
#include <random>
#include <cstdint>

namespace mopo {
// Génère une séquence de randoms [-1,1] pour un cycle, à partir d'un seed et d'une résolution
inline std::vector<float> generateSyncedRandoms(uint32_t seed, int resolution) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::vector<float> values(resolution);
    for (int i = 0; i < resolution; ++i) {
        values[i] = dist(rng);
    }
    return values;
}
} // namespace mopo
