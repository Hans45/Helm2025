#include "../src/synthesis/parameter_batch_interpolator.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    mopo::ParameterBatchInterpolator interp;

    const int block = 16;
    alignas(32) float out0[block];
    alignas(32) float out1[block];
    float* outs[mopo::ParameterBatchInterpolator::MAX_PARAMS] = { nullptr };
    outs[0] = out0; outs[1] = out1;

    // Set initial values
    interp.setTarget(0, 1.0f, 0); // immediate
    interp.setTarget(1, 10.0f, 8); // ramp to 10 over 8 samples

    // Fill first block (16 samples)
    interp.fillBlockAll(outs, 2, block);

    // Check out0 all 1.0
    for (int i = 0; i < block; ++i) assert(std::abs(out0[i] - 1.0f) < 1e-6f);

    // Check out1: first 8 should be ramp from 0->10 (starting current 0)
    float step = 10.0f / 8.0f;
    for (int i = 0; i < 8; ++i) {
        float expected = 0.0f + step * static_cast<float>(i);
        if (std::abs(out1[i] - expected) > 1e-5f) {
            std::cerr << "Mismatch out1["<<i<<"] got="<<out1[i]<<" exp="<<expected<<"\n";
            return 2;
        }
    }

    // After first 8 samples, remaining should be target for rest of block
    for (int i = 8; i < block; ++i) {
        if (std::abs(out1[i] - 10.0f) > 1e-5f) {
            std::cerr << "Mismatch out1["<<i<<"] got="<<out1[i]<<" exp=10\n";
            return 3;
        }
    }

    std::cout << "ParameterBatchInterpolator tests passed.\n";
    return 0;
}
