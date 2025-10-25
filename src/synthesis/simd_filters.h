#pragma once

#include "simd_utils.h"
#include <array>
#include <memory>

namespace mopo {

// Filtre État Variable (SVF) optimisé SIMD
class SimdStateVariableFilter {
public:
    static constexpr int BLOCK_SIZE = 8;

    enum class Type {
        LowPass,
        HighPass,
        BandPass,
        Notch
    };

    struct FilterState {
        simd::Vec8f low;
        simd::Vec8f band;
        simd::Vec8f high;
        simd::Vec8f notch;
        simd::Vec8f frequency;
        simd::Vec8f resonance;
        Type type;
        bool reset;
    };

    SimdStateVariableFilter() noexcept = default;

    [[nodiscard]] simd::Vec8f process(FilterState& state, const simd::Vec8f& input) noexcept {
        if (state.reset) {
            state.low = simd::Vec8f(0.0f);
            state.band = simd::Vec8f(0.0f);
            state.high = simd::Vec8f(0.0f);
            state.notch = simd::Vec8f(0.0f);
            state.reset = false;
        }

        // Calcul des coefficients
        const auto f = compute_f(state.frequency);
        const auto q = compute_q(state.resonance);
        
        // Calcul des composantes du filtre
        const auto high = input - (state.band * q + state.low);
        const auto band = f * high + state.band;
        const auto low = f * band + state.low;
        const auto notch = high + low;

        // Mise à jour de l'état
        state.high = high;
        state.band = band;
        state.low = low;
        state.notch = notch;

        // Sélection de la sortie selon le type
        switch (state.type) {
            case Type::LowPass:
                return low;
            case Type::HighPass:
                return high;
            case Type::BandPass:
                return band;
            case Type::Notch:
                return notch;
            default:
                return low;
        }
    }

    void setFrequency(FilterState& state, const simd::Vec8f& frequency) noexcept {
        state.frequency = frequency;
    }

    void setResonance(FilterState& state, const simd::Vec8f& resonance) noexcept {
        state.resonance = resonance;
    }

    void setType(FilterState& state, Type type) noexcept {
        state.type = type;
    }

    void reset(FilterState& state) noexcept {
        state.reset = true;
    }

private:
    [[nodiscard]] static simd::Vec8f compute_f(const simd::Vec8f& frequency) noexcept {
        // f = 2 * sin(π * frequency / sampleRate)
        constexpr float TWO_PI = 6.28318530718f;
        return simd::Vec8f(2.0f) * (frequency * simd::Vec8f(TWO_PI / 44100.0f)).sin();
    }

    [[nodiscard]] static simd::Vec8f compute_q(const simd::Vec8f& resonance) noexcept {
        // q = 1/Q où Q est la résonance (limitée pour éviter l'auto-oscillation)
        return simd::Vec8f(1.0f) / resonance.max(simd::Vec8f(0.5f));
    }
};

// Filtre DC optimisé SIMD
class SimdDcFilter {
public:
    static constexpr int BLOCK_SIZE = 8;

    struct FilterState {
        simd::Vec8f past_in;
        simd::Vec8f past_out;
        float coefficient;
        bool reset;
    };

    SimdDcFilter() noexcept = default;

    [[nodiscard]] simd::Vec8f process(FilterState& state, const simd::Vec8f& input) noexcept {
        if (state.reset) {
            state.past_in = simd::Vec8f(0.0f);
            state.past_out = simd::Vec8f(0.0f);
            state.reset = false;
        }

        const auto coeff = simd::Vec8f(state.coefficient);
        const auto output = input - state.past_in + coeff * state.past_out;
        
        state.past_in = input;
        state.past_out = output;

        return output;
    }

    void setCoefficient(FilterState& state, float sample_rate) noexcept {
        state.coefficient = 1.0f - COEFFICIENT_TO_SR_CONSTANT / sample_rate;
    }

    void reset(FilterState& state) noexcept {
        state.reset = true;
    }
};

// Classe principale des filtres optimisés
class SimdFilterProcessor : public Processor {
public:
    SimdFilterProcessor() 
        : Processor(kNumInputs, 1)
        , output_buffer_(std::make_unique<simd::AlignedAudioBuffer<float>>(MAX_BUFFER_SIZE)) {
        
        svf_state_.frequency = simd::Vec8f(1000.0f);  // Fréquence par défaut
        svf_state_.resonance = simd::Vec8f(0.707f);   // Q par défaut
        svf_state_.type = SimdStateVariableFilter::Type::LowPass;
        
        dc_filter_.setCoefficient(dc_state_, getSampleRate());
    }

    void process() noexcept override {
        const float* input = input(kAudio)->buffer;
        float* output = output_buffer_->data();

        // Traitement par blocs de 8 échantillons
        for (int i = 0; i < buffer_size_; i += BLOCK_SIZE) {
            // Chargement des données d'entrée
            auto input_block = simd::Vec8f::load_unaligned(input + i);
            
            // Application des filtres en cascade
            auto filtered = svf_.process(svf_state_, input_block);
            filtered = dc_filter_.process(dc_state_, filtered);
            
            // Stockage du résultat
            filtered.store_aligned(output + i);
        }

        // Copie vers la sortie
        std::memcpy(output(0)->buffer, output_buffer_->data(), buffer_size_ * sizeof(float));
    }

    void setSampleRate(int sample_rate) noexcept override {
        Processor::setSampleRate(sample_rate);
        dc_filter_.setCoefficient(dc_state_, static_cast<float>(sample_rate));
    }

    void updateFilterParameters(float frequency, float resonance, SimdStateVariableFilter::Type type) noexcept {
        svf_state_.frequency = simd::Vec8f(frequency);
        svf_state_.resonance = simd::Vec8f(resonance);
        svf_state_.type = type;
    }

private:
    static constexpr int BLOCK_SIZE = 8;

    SimdStateVariableFilter svf_;
    SimdStateVariableFilter::FilterState svf_state_;
    
    SimdDcFilter dc_filter_;
    SimdDcFilter::FilterState dc_state_;

    std::unique_ptr<simd::AlignedAudioBuffer<float>> output_buffer_;
};

} // namespace mopo