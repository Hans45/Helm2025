#pragma once

#include "midi_event.h"
#include "concurrentqueue.h"
#include <memory>
#include <vector>

namespace mopo {

class MidiEventQueue {
public:
    static constexpr size_t DEFAULT_QUEUE_SIZE = 1024;

    explicit MidiEventQueue(size_t capacity = DEFAULT_QUEUE_SIZE) noexcept
        : queue_(capacity) {}

    template<typename T>
    requires std::same_as<std::decay_t<T>, MidiEvent>
    bool enqueue(T&& event) noexcept {
        return queue_.enqueue(std::forward<T>(event));
    }

    template<typename T>
    requires std::same_as<std::decay_t<T>, MidiEvent>
    bool tryEnqueue(T&& event) noexcept {
        return queue_.try_enqueue(std::forward<T>(event));
    }

    [[nodiscard]] bool dequeue(MidiEvent& event) noexcept {
        return queue_.try_dequeue(event);
    }

    template<typename Handler>
    requires requires(Handler h, MidiEvent& e) { h.handleMidiEvent(e); }
    void processEvents(Handler& handler) noexcept {
        MidiEvent event;
        while (queue_.try_dequeue(event)) {
            handler.handleMidiEvent(event);
        }
    }

    void clear() noexcept {
        MidiEvent event;
        while (queue_.try_dequeue(event)) {}
    }

    [[nodiscard]] bool empty() const noexcept {
        return queue_.size_approx() == 0;
    }

    [[nodiscard]] size_t size() const noexcept {
        return queue_.size_approx();
    }

private:
    moodycamel::ConcurrentQueue<MidiEvent> queue_;
};

} // namespace mopo

