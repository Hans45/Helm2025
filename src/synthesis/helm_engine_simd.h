#pragma once

#include "helm_voice_handler_simd.h"
#include "midi_event.h"
#include <memory>

namespace mopo {

class HelmEngineSimd {
public:
    HelmEngineSimd(int sample_rate = 44100) 
        : voice_handler_(std::make_unique<HelmVoiceHandlerSimd>(sample_rate))
        , sample_rate_(sample_rate) {
        
        setupDefaultParameters();
    }

    void processBlock(float* left, float* right, int num_samples) noexcept {
        // Traitement des événements MIDI en attente
        MidiEvent event;
        while (midi_queue_.dequeue(event)) {
            handleMidiEvent(event);
        }

        // Traitement audio
        voice_handler_->processBlock(left, right, num_samples);
    }

    void pushMidiEvent(const MidiEvent& event) noexcept {
        midi_queue_.tryEnqueue(event);
    }

    void setSampleRate(int sample_rate) noexcept {
        sample_rate_ = sample_rate;
        setupDefaultParameters();
    }

    void setOscillatorParameters(int oscillator, 
                               float frequency, 
                               float amplitude, 
                               int waveform) noexcept {
        voice_handler_->setOscillatorParameters(oscillator, frequency, amplitude, waveform);
    }

    void setFilterParameters(int filter,
                           float frequency,
                           float resonance,
                           SimdStateVariableFilter::Type type) noexcept {
        voice_handler_->setFilterParameters(filter, frequency, resonance, type);
    }

    void setEnvelopeParameters(bool is_amp_env,
                             const SimdEnvelope::EnvelopeParameters& params) noexcept {
        voice_handler_->setEnvelopeParameters(is_amp_env, params);
    }

private:
    void handleMidiEvent(const MidiEvent& event) noexcept {
        std::visit([this](const auto& e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, MidiNoteOn>) {
                voice_handler_->noteOn(e.note, e.velocity / 127.0f, e.channel);
            }
            else if constexpr (std::is_same_v<T, MidiNoteOff>) {
                voice_handler_->noteOff(e.note, e.channel);
            }
        }, event);
    }

    void setupDefaultParameters() noexcept {
        // Configuration par défaut des oscillateurs
        setOscillatorParameters(1, 440.0f, 0.5f, 0); // Sine
        setOscillatorParameters(2, 440.0f, 0.5f, 0); // Sine

        // Configuration par défaut des filtres
        setFilterParameters(1, 2000.0f, 0.707f, SimdStateVariableFilter::Type::LowPass);
        setFilterParameters(2, 2000.0f, 0.707f, SimdStateVariableFilter::Type::LowPass);

        // Configuration par défaut des enveloppes
        SimdEnvelope::EnvelopeParameters amp_env = {
            .attack_time = 0.01f,
            .decay_time = 0.1f,
            .sustain_level = 0.7f,
            .release_time = 0.2f
        };
        setEnvelopeParameters(true, amp_env);

        SimdEnvelope::EnvelopeParameters filter_env = {
            .attack_time = 0.1f,
            .decay_time = 0.2f,
            .sustain_level = 0.5f,
            .release_time = 0.3f
        };
        setEnvelopeParameters(false, filter_env);
    }

    std::unique_ptr<HelmVoiceHandlerSimd> voice_handler_;
    MidiEventQueue midi_queue_;
    int sample_rate_;
};

} // namespace mopo