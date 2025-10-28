#include "helm2025_engine.h"
#include "midi_event.h"
#include <variant>

namespace mopo {
void HelmEngine::handleMidiEvent(const MidiEvent& event) noexcept {
    (void)std::visit([this](const auto& e) -> void {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, MidiNoteOn>) {
            (void)noteOn(e.note, e.velocity / 127.0f, e.sample, e.channel);
        }
        else if constexpr (std::is_same_v<T, MidiNoteOff>) {
            (void)noteOff(e.note, e.sample);
        }
        else if constexpr (std::is_same_v<T, MidiControlChange>) {
            if (e.controller == 1) { // Modulation wheel
                (void)setModWheel(e.value / 127.0f, e.channel);
            }
            else if (e.controller == 64) { // Sustain pedal
                (void)(e.value >= 64 ? sustainOn() : sustainOff());
            }
        }
        else if constexpr (std::is_same_v<T, MidiPitchBend>) {
            // Convert from -8192..8191 to -1..1
            float normalized = e.value / 8192.0f;
            (void)setPitchWheel(normalized, e.channel);
        }
        else if constexpr (std::is_same_v<T, MidiAftertouch>) {
            (void)setAftertouch(e.note, e.pressure / 127.0f, e.sample);
        }
        else if constexpr (std::is_same_v<T, MidiChannelAftertouch>) {
            (void)setChannelAftertouch(e.channel, e.pressure / 127.0f, e.sample);
        }
    }, event);
}
} // namespace mopo

