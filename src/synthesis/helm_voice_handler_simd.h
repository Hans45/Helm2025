#pragma once

#include "simd_oscillators.h"
#include "simd_filters.h"
#include "simd_envelope.h"
#include "helm_common.h"
#include <array>
#include <vector>

namespace mopo {

class HelmVoiceHandlerSimd {
public:
    static constexpr int BLOCK_SIZE = 8;
    static constexpr int MAX_VOICES = 32;
    
    struct Voice {
        int note;
        int channel;
        bool active;
        SimdOscillator::OscillatorState osc1_state;
        SimdOscillator::OscillatorState osc2_state;
        SimdStateVariableFilter::FilterState filter1_state;
        SimdStateVariableFilter::FilterState filter2_state;
        SimdEnvelope::EnvelopeState amp_env_state;
        SimdEnvelope::EnvelopeState filter_env_state;
    };

    HelmVoiceHandlerSimd(int sample_rate = 44100) 
        : sample_rate_(sample_rate)
        , active_voices_(0)
        , output_buffer_(std::make_unique<simd::AlignedAudioBuffer<float>>(MAX_BUFFER_SIZE)) {
        
        initializeVoices();
    }

    void processBlock(float* output_left, float* output_right, int num_samples) noexcept {
        // Traitement par blocs de 8 échantillons
        for (int i = 0; i < num_samples; i += BLOCK_SIZE) {
            simd::Vec8f mix_left(0.0f);
            simd::Vec8f mix_right(0.0f);

            // Pour chaque groupe de 8 voix
            for (int v = 0; v < active_voices_; v += BLOCK_SIZE) {
                auto voice_output = processVoiceGroup(v);
                mix_left = mix_left + voice_output.first;
                mix_right = mix_right + voice_output.second;
            }

            // Écriture dans les buffers de sortie
            mix_left.store_aligned(output_left + i);
            mix_right.store_aligned(output_right + i);
        }
    }

    void noteOn(int note, float velocity, int channel) noexcept {
        if (active_voices_ >= MAX_VOICES) {
            return;
        }

        // Trouver une voix libre
        for (int i = 0; i < MAX_VOICES; ++i) {
            if (!voices_[i].active) {
                activateVoice(i, note, velocity, channel);
                active_voices_++;
                break;
            }
        }
    }

    void noteOff(int note, int channel) noexcept {
        for (int i = 0; i < MAX_VOICES; ++i) {
            if (voices_[i].active && voices_[i].note == note && voices_[i].channel == channel) {
                releaseVoice(i);
            }
        }
    }

    void setOscillatorParameters(int oscillator, float frequency, float amplitude, int waveform) noexcept {
        for (auto& voice : voices_) {
            auto& osc_state = (oscillator == 1) ? voice.osc1_state : voice.osc2_state;
            osc_state.phase_inc = simd::Vec8f(frequency / sample_rate_);
            osc_state.amplitude = simd::Vec8f(amplitude);
            if (waveform >= 0) {
                osc_state.waveform = waveform;
            }
        }
    }

    void setFilterParameters(int filter, float frequency, float resonance, SimdStateVariableFilter::Type type) noexcept {
        for (auto& voice : voices_) {
            auto& filter_state = (filter == 1) ? voice.filter1_state : voice.filter2_state;
            filter_state.frequency = simd::Vec8f(frequency);
            filter_state.resonance = simd::Vec8f(resonance);
            filter_state.type = type;
        }
    }

    void setEnvelopeParameters(bool is_amp_env, const SimdEnvelope::EnvelopeParameters& params) noexcept {
        for (auto& voice : voices_) {
            auto& env_state = is_amp_env ? voice.amp_env_state : voice.filter_env_state;
            envelope_.setParameters(env_state, params);
        }
    }

private:
    void initializeVoices() {
        voices_.resize(MAX_VOICES);
        for (auto& voice : voices_) {
            voice.active = false;
            resetVoiceStates(voice);
        }
    }

    void resetVoiceStates(Voice& voice) noexcept {
        voice.osc1_state.reset = true;
        voice.osc2_state.reset = true;
        voice.filter1_state.reset = true;
        voice.filter2_state.reset = true;
        voice.amp_env_state.current_value = simd::Vec8f(0.0f);
        voice.filter_env_state.current_value = simd::Vec8f(0.0f);
    }

    void activateVoice(int index, int note, float velocity, int channel) noexcept {
        auto& voice = voices_[index];
        voice.note = note;
        voice.channel = channel;
        voice.active = true;
        
        // Calcul de la fréquence de base
        float frequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
        
        // Configuration des oscillateurs
        voice.osc1_state.phase_inc = simd::Vec8f(frequency / sample_rate_);
        voice.osc2_state.phase_inc = simd::Vec8f(frequency / sample_rate_);
        
        // Déclenchement des enveloppes
        envelope_.trigger(voice.amp_env_state);
        envelope_.trigger(voice.filter_env_state);
    }

    void releaseVoice(int index) noexcept {
        auto& voice = voices_[index];
        envelope_.release(voice.amp_env_state);
        envelope_.release(voice.filter_env_state);
        
        // La voix sera désactivée une fois l'enveloppe terminée
        if (voice.amp_env_state.stage == SimdEnvelope::Stage::Off) {
            voice.active = false;
            active_voices_--;
        }
    }

    std::pair<simd::Vec8f, simd::Vec8f> processVoiceGroup(int start_voice) noexcept {
        simd::Vec8f voice_mix_left(0.0f);
        simd::Vec8f voice_mix_right(0.0f);

        for (int i = 0; i < BLOCK_SIZE && (start_voice + i) < active_voices_; ++i) {
            auto& voice = voices_[start_voice + i];
            if (!voice.active) continue;

            // Traitement des oscillateurs
            auto osc1_out = oscillator1_.process(voice.osc1_state);
            auto osc2_out = oscillator2_.process(voice.osc2_state);
            auto osc_mix = osc1_out + osc2_out;

            // Traitement des filtres
            auto filter1_out = filter1_.process(voice.filter1_state, osc_mix);
            auto filter2_out = filter2_.process(voice.filter2_state, filter1_out);

            // Traitement des enveloppes
            auto amp_env = envelope_.process(voice.amp_env_state);
            auto filter_env = envelope_.process(voice.filter_env_state);

            // Application des modulations
            auto filtered = filter2_out * filter_env;
            auto final_output = filtered * amp_env;

            // Panoramique simple (alternance gauche/droite)
            voice_mix_left = voice_mix_left + final_output * (i % 2 ? 0.7f : 0.3f);
            voice_mix_right = voice_mix_right + final_output * (i % 2 ? 0.3f : 0.7f);
        }

        return {voice_mix_left, voice_mix_right};
    }

    int sample_rate_;
    int active_voices_;
    std::vector<Voice> voices_;
    
    SimdOscillator oscillator1_;
    SimdOscillator oscillator2_;
    SimdStateVariableFilter filter1_;
    SimdStateVariableFilter filter2_;
    SimdEnvelope envelope_;

    std::unique_ptr<simd::AlignedAudioBuffer<float>> output_buffer_;
};

} // namespace mopo