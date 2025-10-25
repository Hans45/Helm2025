#pragma once

#include "concurrentqueue/concurrentqueue.h"
#include <array>
#include <atomic>
#include <functional>
#include <memory>

namespace mopo {

// Structure pour représenter un bloc audio
struct AudioBlock {
    static constexpr size_t BLOCK_SIZE = 128;
    std::array<float, BLOCK_SIZE> left;
    std::array<float, BLOCK_SIZE> right;
    double timestamp;
    uint32_t frame_count;
};

// Structure pour les paramètres de traitement
struct ProcessingParams {
    float cutoff;
    float resonance;
    float envelope_amount;
    float lfo_rate;
    uint32_t frame_count;
};

class LockFreePipeline {
public:
    using ProcessFunction = std::function<void(AudioBlock&, const ProcessingParams&)>;
    static constexpr size_t QUEUE_SIZE = 16;

    LockFreePipeline()
        : running_(false)
        , current_block_index_(0)
        , current_param_index_(0) {
        
        // Préallocation des blocs audio
        for (size_t i = 0; i < QUEUE_SIZE; ++i) {
            audio_blocks_[i] = std::make_unique<AudioBlock>();
            param_blocks_[i] = std::make_unique<ProcessingParams>();
        }
    }

    // Ajout d'une étape de traitement au pipeline
    void addStage(ProcessFunction processor) {
        processors_.push_back(processor);
    }

    // Soumission d'un bloc audio pour traitement
    bool submitAudioBlock(const float* left, const float* right, size_t size, double timestamp) {
        if (size > AudioBlock::BLOCK_SIZE) return false;

        size_t index = current_block_index_.load(std::memory_order_relaxed);
        auto& block = *audio_blocks_[index];

        // Copie des données audio
        std::copy(left, left + size, block.left.begin());
        std::copy(right, right + size, block.right.begin());
        block.timestamp = timestamp;
        block.frame_count = size;

        // Mise à jour de l'index de manière atomique
        current_block_index_.store((index + 1) % QUEUE_SIZE, std::memory_order_release);
        return true;
    }

    // Mise à jour des paramètres de traitement
    void updateParams(const ProcessingParams& params) {
        size_t index = current_param_index_.load(std::memory_order_relaxed);
        *param_blocks_[index] = params;
        current_param_index_.store((index + 1) % QUEUE_SIZE, std::memory_order_release);
    }

    // Traitement d'un bloc audio
    void process() {
        size_t block_index = current_block_index_.load(std::memory_order_acquire);
        size_t param_index = current_param_index_.load(std::memory_order_acquire);

        if (!running_.load(std::memory_order_relaxed)) return;

        auto& audio_block = *audio_blocks_[block_index];
        const auto& params = *param_blocks_[param_index];

        // Application de chaque étape du pipeline
        for (const auto& processor : processors_) {
            processor(audio_block, params);
        }
    }

    // Démarrage du pipeline
    void start() {
        running_.store(true, std::memory_order_release);
    }

    // Arrêt du pipeline
    void stop() {
        running_.store(false, std::memory_order_release);
    }

    // Récupération du dernier bloc traité
    bool getProcessedBlock(float* left, float* right, size_t& size, double& timestamp) {
        size_t index = current_block_index_.load(std::memory_order_acquire);
        if (index == 0) index = QUEUE_SIZE - 1;
        else index--;

        const auto& block = *audio_blocks_[index];
        
        size = block.frame_count;
        timestamp = block.timestamp;
        
        std::copy(block.left.begin(), block.left.begin() + size, left);
        std::copy(block.right.begin(), block.right.begin() + size, right);
        
        return true;
    }

private:
    std::atomic<bool> running_;
    std::atomic<size_t> current_block_index_;
    std::atomic<size_t> current_param_index_;
    
    std::array<std::unique_ptr<AudioBlock>, QUEUE_SIZE> audio_blocks_;
    std::array<std::unique_ptr<ProcessingParams>, QUEUE_SIZE> param_blocks_;
    std::vector<ProcessFunction> processors_;
};

} // namespace mopo