#pragma once

#include "simd_utils.h"
#include <array>

namespace mopo {

class SimdEnvelope {
public:
    static constexpr int BLOCK_SIZE = 8;
    
    enum class Stage {
        Attack,
        Decay,
        Sustain,
        Release,
        Off
    };

    struct EnvelopeState {
        simd::Vec8f current_value;
        simd::Vec8f target_value;
        simd::Vec8f rate;
        Stage stage;
        bool triggered;
        bool released;
    };

    struct EnvelopeParameters {
        float attack_time;
        float decay_time;
        float sustain_level;
        float release_time;
    };

    SimdEnvelope() noexcept = default;

    [[nodiscard]] simd::Vec8f process(EnvelopeState& state) noexcept {
        if (state.triggered) {
            startAttack(state);
            state.triggered = false;
        }
        else if (state.released && state.stage != Stage::Release) {
            startRelease(state);
        }

        updateValue(state);
        return state.current_value;
    }

    void trigger(EnvelopeState& state) noexcept {
        state.triggered = true;
        state.released = false;
    }

    void release(EnvelopeState& state) noexcept {
        state.released = true;
    }

    void setParameters(EnvelopeState& state, const EnvelopeParameters& params) noexcept {
        parameters_ = params;
        updateRates();
    }

private:
    void startAttack(EnvelopeState& state) noexcept {
        state.stage = Stage::Attack;
        state.target_value = simd::Vec8f(1.0f);
        state.rate = simd::Vec8f(attack_rate_);
    }

    void startDecay(EnvelopeState& state) noexcept {
        state.stage = Stage::Decay;
        state.target_value = simd::Vec8f(parameters_.sustain_level);
        state.rate = simd::Vec8f(decay_rate_);
    }

    void startRelease(EnvelopeState& state) noexcept {
        state.stage = Stage::Release;
        state.target_value = simd::Vec8f(0.0f);
        state.rate = simd::Vec8f(release_rate_);
    }

    void updateValue(EnvelopeState& state) noexcept {
        const auto diff = state.target_value - state.current_value;
        const auto change = diff * state.rate;
        state.current_value = state.current_value + change;

        // Vérification des transitions
        const auto reached_target = (diff.abs() < simd::Vec8f(0.001f));
        
        if (state.stage == Stage::Attack && reached_target.any()) {
            startDecay(state);
        }
        else if (state.stage == Stage::Release && reached_target.any()) {
            state.stage = Stage::Off;
        }
    }

    void updateRates() noexcept {
        constexpr float MIN_TIME = 0.001f;
        attack_rate_ = 1.0f / std::max(parameters_.attack_time, MIN_TIME);
        decay_rate_ = 1.0f / std::max(parameters_.decay_time, MIN_TIME);
        release_rate_ = 1.0f / std::max(parameters_.release_time, MIN_TIME);
    }

    EnvelopeParameters parameters_;
    float attack_rate_;
    float decay_rate_;
    float release_rate_;
};

class SimdMultiEnvelope : public Processor {
public:
    SimdMultiEnvelope()
        : Processor(kNumInputs, 1)
        , output_buffer_(std::make_unique<simd::AlignedAudioBuffer<float>>(MAX_BUFFER_SIZE)) {
        
        envelope_state_.current_value = simd::Vec8f(0.0f);
        envelope_state_.target_value = simd::Vec8f(0.0f);
        envelope_state_.stage = SimdEnvelope::Stage::Off;
    }

    void process() noexcept override {
        float* output = output_buffer_->data();

        // Traitement par blocs
        for (int i = 0; i < buffer_size_; i += SimdEnvelope::BLOCK_SIZE) {
            auto value = envelope_.process(envelope_state_);
            value.store_aligned(output + i);
        }

        // Copie vers la sortie
        std::memcpy(output(0)->buffer, output_buffer_->data(), buffer_size_ * sizeof(float));
    }

    void noteOn() noexcept {
        envelope_.trigger(envelope_state_);
    }

    void noteOff() noexcept {
        envelope_.release(envelope_state_);
    }

    void setParameters(const SimdEnvelope::EnvelopeParameters& params) noexcept {
        envelope_.setParameters(envelope_state_, params);
    }

private:
    SimdEnvelope envelope_;
    SimdEnvelope::EnvelopeState envelope_state_;
    std::unique_ptr<simd::AlignedAudioBuffer<float>> output_buffer_;
};

} // namespace mopo