#pragma once

#include <unordered_map>
#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>

namespace mopo {

// C++20 concepts pour sécuriser les types de clé et valeur
#include <concepts>
template<typename Key, typename Value, size_t CacheSize = 1024>
requires std::copyable<Key> && std::copyable<Value>
class ComputeCache {
public:
    using ComputeFunction = std::function<Value(const Key&)>;

    explicit ComputeCache(ComputeFunction compute_fn) 
        : compute_fn_(std::move(compute_fn))
        , current_index_(0) {}

    [[nodiscard]] Value get(const Key& key) noexcept {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            return cache_[it->second].value;
        }

        Value computed = compute_fn_(key);
        add_to_cache(key, computed);
        return computed;
    }

    void clear() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_map_.clear();
        current_index_ = 0;
    }

private:
    void add_to_cache(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Si la clé existe déjà, mettre à jour la valeur
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            cache_[it->second].value = value;
            return;
        }

        // Sinon, ajouter une nouvelle entrée
        size_t index = current_index_;
        current_index_ = (current_index_ + 1) % CacheSize;

        // Si l'emplacement est occupé, supprimer l'ancienne entrée
        if (cache_[index].key) {
            cache_map_.erase(*cache_[index].key);
        }

        cache_[index] = {key, value};
        cache_map_[key] = index;
    }

    struct CacheEntry {
        std::optional<Key> key;
        Value value;
    };

    std::array<CacheEntry, CacheSize> cache_;
    std::unordered_map<Key, size_t> cache_map_;
    ComputeFunction compute_fn_;
    size_t current_index_;
    std::mutex mutex_;
};

// Cache spécialisé pour les calculs de forme d'onde
template<typename T = float>
requires std::floating_point<T>
class WaveTableCache {
public:
    static constexpr size_t TABLE_SIZE = 2048;
    using WaveTable = std::array<T, TABLE_SIZE>;

    [[nodiscard]] std::span<const T> getSineTable() const noexcept {
        return std::span<const T>(sine_table_.data(), sine_table_.size());
    }

    [[nodiscard]] std::span<const T> getSawTable() const noexcept {
        return std::span<const T>(saw_table_.data(), saw_table_.size());
    }

    [[nodiscard]] std::span<const T> getSquareTable() const noexcept {
        return std::span<const T>(square_table_.data(), square_table_.size());
    }

    [[nodiscard]] std::span<const T> getTriangleTable() const noexcept {
        return std::span<const T>(triangle_table_.data(), triangle_table_.size());
    }

private:
    static WaveTable generateSineTable() noexcept {
        WaveTable table;
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            table[i] = std::sin(2.0 * M_PI * i / TABLE_SIZE);
        }
        return table;
    }

    static WaveTable generateSawTable() noexcept {
        WaveTable table;
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            table[i] = 2.0 * i / TABLE_SIZE - 1.0;
        }
        return table;
    }

    static WaveTable generateSquareTable() noexcept {
        WaveTable table;
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            table[i] = i < TABLE_SIZE / 2 ? 1.0 : -1.0;
        }
        return table;
    }

    static WaveTable generateTriangleTable() noexcept {
        WaveTable table;
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            if (i < TABLE_SIZE / 2) {
                table[i] = 2.0 * i / (TABLE_SIZE / 2) - 1.0;
            } else {
                table[i] = 1.0 - 2.0 * (i - TABLE_SIZE / 2) / (TABLE_SIZE / 2);
            }
        }
        return table;
    }

    const WaveTable sine_table_ = generateSineTable();
    const WaveTable saw_table_ = generateSawTable();
    const WaveTable square_table_ = generateSquareTable();
    const WaveTable triangle_table_ = generateTriangleTable();
};

} // namespace mopo

