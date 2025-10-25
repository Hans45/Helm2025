#include "../src/synthesis/parameter_interpolator.h"
#include <iostream>
#include <vector>
#include <cassert>

int main() {
    mopo::ParameterInterpolator interp;

    const int block = 16;
    float out[block];

    // Test 1: immediate target
    interp.setTarget(1.0f, 0);
    interp.fillBlock(out, block);
    for (int i = 0; i < block; ++i)
        assert(out[i] == 1.0f);

    // Test 2: linear ramp over 8 samples
    interp.setTarget(2.0f, 8);
    interp.fillBlock(out, 8);
    // Expected values: start at 1.0, step = (2-1)/8 = 0.125
    for (int i = 0; i < 8; ++i) {
        float expected = 1.0f + 0.125f * static_cast<float>(i);
        float diff = std::abs(out[i] - expected);
        if (diff > 1e-6f) {
            std::cerr << "Mismatch at " << i << " got=" << out[i] << " exp=" << expected << "\n";
            return 2;
        }
    }

    // Test 3: continuing ramp across blocks
    interp.fillBlock(out, 8);
    // After 8 samples the interpolator should have reached target
    for (int i = 0; i < 8; ++i)
        assert(std::abs(out[i] - 2.0f) < 1e-6f);

    std::cout << "All ParameterInterpolator tests passed.\n";
    return 0;
}
