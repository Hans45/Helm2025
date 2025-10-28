#pragma once

#include "lock_free_pipeline.h"
#include "simd_audio_processor.h"
#include "simd_filter_processor.h"
#include "parameter_interpolator.h"
#include "parameter_batch_interpolator.h"
#include <memory>
#include <algorithm>

namespace mopo {

class AudioPipelineManager {
public:
    AudioPipelineManager() {
        setupPipeline();
    }

    void start() {
        pipeline_.start();
    }

    void stop() {
        pipeline_.stop();
    }

    bool processAudioBlock(const float* left_in, const float* right_in,
                          float* left_out, float* right_out,
                          size_t size, double timestamp) {
        
        // Soumission du bloc audio
        if (!pipeline_.submitAudioBlock(left_in, right_in, size, timestamp))
            return false;

        // Traitement du bloc
        pipeline_.process();

        // Récupération du résultat
        size_t processed_size;
        double processed_timestamp;
        return pipeline_.getProcessedBlock(left_out, right_out, processed_size, processed_timestamp);
    }

    void updateParameters(const ProcessingParams& params) {
        pipeline_.updateParams(params);
    }

private:
    void setupPipeline() {
        // Étape 1 : Pré-traitement SIMD
        pipeline_.addStage([](AudioBlock& block, const ProcessingParams& params) {
            SimdAudioProcessor::processAudioBlock(block.left.data(), block.frame_count);
            SimdAudioProcessor::processAudioBlock(block.right.data(), block.frame_count);
        });

        // Étape 2 : Filtrage
        pipeline_.addStage([](AudioBlock& block, const ProcessingParams& params) {
            SimdFilterProcessor::processLowPass(
                block.left.data(),
                block.frame_count,
                params.cutoff,
                params.resonance
            );
            SimdFilterProcessor::processLowPass(
                block.right.data(),
                block.frame_count,
                params.cutoff,
                params.resonance
            );
        });

        // Étape 3 : Application des paramètres (batch interpolator)
        pipeline_.addStage([this](AudioBlock& block, const ProcessingParams& params) {
            // Nous allons interpoler simultanément : 0=envelope_amount, 1=cutoff_mod
            // Définir les targets pour chaque paramètre
            batch_interpolator_.setTarget(0, params.envelope_amount, static_cast<int>(block.frame_count));
            batch_interpolator_.setTarget(1, params.cutoff, static_cast<int>(block.frame_count));

            // Préparer les buffers (SOA)
            alignas(32) float env_buf[AudioBlock::BLOCK_SIZE];
            alignas(32) float cutoff_buf[AudioBlock::BLOCK_SIZE];
            float* outs[ParameterBatchInterpolator::MAX_PARAMS] = { nullptr };
            outs[0] = env_buf;
            outs[1] = cutoff_buf;

            batch_interpolator_.fillBlockAll(outs, 2, static_cast<int>(block.frame_count));

            // Appliquer enveloppe
            SimdAudioProcessor::applyEnvelope(block.left.data(), env_buf, block.frame_count);
            SimdAudioProcessor::applyEnvelope(block.right.data(), env_buf, block.frame_count);

            // Exemple d'utilisation du buffer cutoff_buf : attenuation appliquée (demo)
            // On pourrait lier cutoff_buf à un filtre mod ou autre traitement.
            for (size_t i = 0; i < block.frame_count; ++i) {
                float c = cutoff_buf[i];
                // simple scaling demo
                block.left[i] *= (1.0f - 0.001f * c);
                block.right[i] *= (1.0f - 0.001f * c);
            }
        });

        // Étape 4 : Post-traitement et finalisation
        pipeline_.addStage([](AudioBlock& block, const ProcessingParams& params) {
            // Normalisation et limitation
            for (size_t i = 0; i < block.frame_count; ++i) {
                block.left[i] = std::max(-1.0f, std::min(1.0f, block.left[i]));
                block.right[i] = std::max(-1.0f, std::min(1.0f, block.right[i]));
            }
        });
    }

    LockFreePipeline pipeline_;
    ParameterInterpolator interpolator_;
    ParameterBatchInterpolator batch_interpolator_;
};

} // namespace mopo

