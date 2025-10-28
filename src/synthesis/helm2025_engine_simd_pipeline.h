#pragma once

#include "audio_pipeline_manager.h"
#include "helm2025_voice_handler_simd.h"
#include "midi_event.h"
#include <memory>

namespace mopo {

class HelmEngineSimdPipeline {
public:
    HelmEngineSimdPipeline(int sample_rate = 44100) 
        : voice_handler_(std::make_unique<HelmVoiceHandlerSimd>(sample_rate))
        , pipeline_manager_(std::make_unique<AudioPipelineManager>())
        , sample_rate_(sample_rate) {
        
        setupDefaultParameters();
        pipeline_manager_->start();
    }

    ~HelmEngineSimdPipeline() {
        pipeline_manager_->stop();
    }

    void processBlock(float* left, float* right, int num_samples) noexcept {
        // Buffer temporaire pour le traitement des voix
        std::array<float, 2048> voice_left;
        std::array<float, 2048> voice_right;

        // Traitement des événements MIDI
        MidiEvent event;
        while (midi_queue_.dequeue(event)) {
            handleMidiEvent(event);
        }

        // Traitement des voix
        voice_handler_->processBlock(voice_left.data(), voice_right.data(), num_samples);

        // Mise à jour des paramètres de traitement
        ProcessingParams params = getCurrentParams();
        pipeline_manager_->updateParameters(params);

        // Traitement dans le pipeline lock-free
        double timestamp = getCurrentTimestamp();
        pipeline_manager_->processAudioBlock(
            voice_left.data(), voice_right.data(),
            left, right,
            num_samples, timestamp
        );
    }

    void pushMidiEvent(const MidiEvent& event) noexcept {
        midi_queue_.tryEnqueue(event);
    }

    void setSampleRate(int sample_rate) noexcept {
        sample_rate_ = sample_rate;
        setupDefaultParameters();
    }

private:
    ProcessingParams getCurrentParams() const noexcept {
        return {
            .cutoff = voice_handler_->getCurrentCutoff(),
            .resonance = voice_handler_->getCurrentResonance(),
            .envelope_amount = voice_handler_->getCurrentEnvelopeAmount(),
            .lfo_rate = voice_handler_->getCurrentLfoRate(),
            .frame_count = static_cast<uint32_t>(buffer_size_)
        };
    }

    double getCurrentTimestamp() const noexcept {
        return static_cast<double>(total_samples_) / sample_rate_;
    }

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
        buffer_size_ = 128;
        total_samples_ = 0;
        voice_handler_->setupDefaultParameters();
    }

    std::unique_ptr<HelmVoiceHandlerSimd> voice_handler_;
    std::unique_ptr<AudioPipelineManager> pipeline_manager_;
    MidiEventQueue midi_queue_;
    int sample_rate_;
    int buffer_size_;
    uint64_t total_samples_;
};

} // namespace mopo

