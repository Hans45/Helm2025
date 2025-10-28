#pragma once

#include <array>
#include <queue>
#include <mutex>
#include <memory>

namespace mopo {

template<typename T, size_t PoolSize = 1024>
class ObjectPool {
public:
    template<typename... Args>
    [[nodiscard]] std::unique_ptr<T, std::function<void(T*)>> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        T* ptr;
        if (free_objects_.empty()) {
            ptr = new T(std::forward<Args>(args)...);
        } else {
            ptr = free_objects_.front();
            free_objects_.pop();
            new (ptr) T(std::forward<Args>(args)...);  // Placement new
        }

        return std::unique_ptr<T, std::function<void(T*)>>(
            ptr,
            [this](T* obj) {
                recycle(obj);
            }
        );
    }

private:
    void recycle(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (free_objects_.size() < PoolSize) {
            obj->~T();  // Appel explicite du destructeur
            free_objects_.push(obj);
        } else {
            delete obj;
        }
    }

    std::queue<T*> free_objects_;
    std::mutex mutex_;
};

// Pool spécialisé pour les événements MIDI
template<typename T>
class MidiEventPool {
public:
    static MidiEventPool& instance() {
        static MidiEventPool pool;
        return pool;
    }

    template<typename... Args>
    [[nodiscard]] auto acquireEvent(Args&&... args) {
        return pool_.acquire(std::forward<Args>(args)...);
    }

private:
    MidiEventPool() = default;
    ObjectPool<T> pool_;
};

} // namespace mopo

