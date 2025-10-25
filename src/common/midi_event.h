#pragma once

#include <cstdint>
#include <variant>
#include <optional>

namespace mopo {

struct MidiNoteOn {
    uint8_t note;
    uint8_t velocity;
    uint8_t channel;
    int32_t sample;

    constexpr bool operator==(const MidiNoteOn& other) const noexcept {
        return note == other.note && 
               velocity == other.velocity && 
               channel == other.channel &&
               sample == other.sample;
    }
};

struct MidiNoteOff {
    uint8_t note;
    uint8_t channel;
    int32_t sample;

    constexpr bool operator==(const MidiNoteOff& other) const noexcept {
        return note == other.note && 
               channel == other.channel &&
               sample == other.sample;
    }
};

struct MidiControlChange {
    uint8_t controller;
    uint8_t value;
    uint8_t channel;

    constexpr bool operator==(const MidiControlChange& other) const noexcept {
        return controller == other.controller && 
               value == other.value && 
               channel == other.channel;
    }
};

struct MidiPitchBend {
    int16_t value;  // -8192 to +8191
    uint8_t channel;

    constexpr bool operator==(const MidiPitchBend& other) const noexcept {
        return value == other.value && channel == other.channel;
    }
};

struct MidiAftertouch {
    uint8_t note;
    uint8_t pressure;
    uint8_t channel;
    int32_t sample;

    constexpr bool operator==(const MidiAftertouch& other) const noexcept {
        return note == other.note && 
               pressure == other.pressure && 
               channel == other.channel &&
               sample == other.sample;
    }
};

struct MidiChannelAftertouch {
    uint8_t pressure;
    uint8_t channel;
    int32_t sample;

    constexpr bool operator==(const MidiChannelAftertouch& other) const noexcept {
        return pressure == other.pressure && 
               channel == other.channel &&
               sample == other.sample;
    }
};

using MidiEvent = std::variant<
    MidiNoteOn,
    MidiNoteOff,
    MidiControlChange,
    MidiPitchBend,
    MidiAftertouch,
    MidiChannelAftertouch
>;

class MidiEventHandler {
public:
    virtual ~MidiEventHandler() = default;

    virtual void handleMidiEvent(const MidiEvent& event) noexcept = 0;

protected:
    template<typename T>
    [[nodiscard]] static constexpr std::optional<T> getMidiEventAs(const MidiEvent& event) noexcept {
        if (const T* specific = std::get_if<T>(&event)) {
            return *specific;
        }
        return std::nullopt;
    }
};

} // namespace mopo