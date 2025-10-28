#pragma once

#include "simd_utils.h"
#include "compute_cache.h"
#include "fixed_point_wave.h"
#include <array>

namespace mopo {

class SimdOscillator {
public:
    static constexpr int BLOCK_SIZE = 8;  // Taille du bloc AVX
    
    struct OscillatorState {
        simd::Vec8f phase;
        simd::Vec8f phase_inc;
        simd::Vec8f amplitude;
        int waveform;
        bool reset;
    };

    SimdOscillator() noexcept 
        : wave_table_cache_(std::make_unique<WaveTableCache<float>>()) {}

    [[nodiscard]] simd::Vec8f process(OscillatorState& state) noexcept {
        if (state.reset) {
            state.phase = simd::Vec8f(0.0f);
            state.reset = false;
        }

        // Obtenir la table d'onde appropriée
        const auto& wave_table = getWaveTable(state.waveform);
        
        // Calculer les indices et les fractions pour l'interpolation
        auto phase_int = state.phase.to_array();
        std::array<float, BLOCK_SIZE> output;

        // Traitement vectorisé par blocs
        #pragma omp simd
        for (int i = 0; i < BLOCK_SIZE; ++i) {
            unsigned int index = static_cast<unsigned int>(phase_int[i]) & (WaveTableCache<float>::TABLE_SIZE - 1);
            float frac = phase_int[i] - static_cast<float>(index);
            
            float sample1 = wave_table[index];
            float sample2 = wave_table[(index + 1) & (WaveTableCache<float>::TABLE_SIZE - 1)];
            
            output[i] = sample1 + frac * (sample2 - sample1);
        }

        // Mise à jour de la phase
        state.phase = state.phase + state.phase_inc;

        // Conversion en Vec8f et application de l'amplitude
        return simd::Vec8f::load_aligned(output.data()) * state.amplitude;
    }

    void setWaveform(OscillatorState& state, int waveform) noexcept {
        state.waveform = waveform;
    }

    void setPhaseInc(OscillatorState& state, const simd::Vec8f& phase_inc) noexcept {
        state.phase_inc = phase_inc;
    }

    void setAmplitude(OscillatorState& state, const simd::Vec8f& amplitude) noexcept {
        state.amplitude = amplitude;
    }

    void reset(OscillatorState& state) noexcept {
        state.reset = true;
    }

private:
    [[nodiscard]] const WaveTableCache<float>::WaveTable& getWaveTable(int waveform) const noexcept {
        switch (waveform) {
            case FixedPointWaveLookup::kSin:
                return wave_table_cache_->getSineTable();
            case FixedPointWaveLookup::kTriangle:
                return wave_table_cache_->getTriangleTable();
            case FixedPointWaveLookup::kSquare:
                return wave_table_cache_->getSquareTable();
            case FixedPointWaveLookup::kDownSaw:
            case FixedPointWaveLookup::kUpSaw:
                return wave_table_cache_->getSawTable();
            default:
                return wave_table_cache_->getSineTable();
        }
    }

    std::unique_ptr<WaveTableCache<float>> wave_table_cache_;
};

class SimdHelmOscillators : public Processor {
public:
    SimdHelmOscillators() : Processor(kNumInputs, 2) {
        oscillator1_state_.waveform = FixedPointWaveLookup::kSin;
        oscillator2_state_.waveform = FixedPointWaveLookup::kSin;
        
        output_buffer1_ = std::make_unique<simd::AlignedAudioBuffer<float>>(MAX_BUFFER_SIZE);
        output_buffer2_ = std::make_unique<simd::AlignedAudioBuffer<float>>(MAX_BUFFER_SIZE);
    }

    void process() noexcept override {
        // Mise à jour des paramètres
        updateOscillatorStates();

        // Traitement par blocs de 8 échantillons
        for (int i = 0; i < buffer_size_; i += SimdOscillator::BLOCK_SIZE) {
            auto osc1_output = oscillator1_.process(oscillator1_state_);
            auto osc2_output = oscillator2_.process(oscillator2_state_);

            osc1_output.store_aligned(output_buffer1_->data() + i);
            osc2_output.store_aligned(output_buffer2_->data() + i);
        }

        // Copie des sorties
        std::memcpy(output(0)->buffer, output_buffer1_->data(), buffer_size_ * sizeof(float));
        std::memcpy(output(1)->buffer, output_buffer2_->data(), buffer_size_ * sizeof(float));
    }

private:
    void updateOscillatorStates() noexcept {
        // Mise à jour des paramètres des oscillateurs
        simd::Vec8f phase_inc1(input(kOscillator1PhaseInc)->at(0));
        simd::Vec8f phase_inc2(input(kOscillator2PhaseInc)->at(0));
        simd::Vec8f amp1(input(kOscillator1Amplitude)->at(0));
        simd::Vec8f amp2(input(kOscillator2Amplitude)->at(0));

        oscillator1_.setPhaseInc(oscillator1_state_, phase_inc1);
        oscillator2_.setPhaseInc(oscillator2_state_, phase_inc2);
        oscillator1_.setAmplitude(oscillator1_state_, amp1);
        oscillator2_.setAmplitude(oscillator2_state_, amp2);

        if (input(kReset)->source->triggered) {
            oscillator1_.reset(oscillator1_state_);
            oscillator2_.reset(oscillator2_state_);
        }

        int waveform1 = static_cast<int>(input(kOscillator1Waveform)->at(0));
        int waveform2 = static_cast<int>(input(kOscillator2Waveform)->at(0));
        
        oscillator1_.setWaveform(oscillator1_state_, waveform1);
        oscillator2_.setWaveform(oscillator2_state_, waveform2);
    }

    SimdOscillator oscillator1_;
    SimdOscillator oscillator2_;
    SimdOscillator::OscillatorState oscillator1_state_;
    SimdOscillator::OscillatorState oscillator2_state_;
    
    std::unique_ptr<simd::AlignedAudioBuffer<float>> output_buffer1_;
    std::unique_ptr<simd::AlignedAudioBuffer<float>> output_buffer2_;
};

} // namespace mopo

