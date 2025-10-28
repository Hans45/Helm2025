/* Copyright 2013-2017 Matt Tytel
 *
 * helm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * helm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with helm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "helm2025_oscillators.h"

#include "detune_lookup.h"

#define RAND_DECAY 0.999

namespace mopo {
  const mopo_float HelmOscillators::scales[] = {
      1.0, 1.0,
      sqrt(1.0 / 2.0), sqrt(1.0 / 2.0),
      sqrt(1.0 / 3.0), sqrt(1.0 / 3.0),
      sqrt(1.0 / 4.0), sqrt(1.0 / 4.0),
      sqrt(1.0 / 5.0), sqrt(1.0 / 5.0),
      sqrt(1.0 / 6.0), sqrt(1.0 / 6.0),
      sqrt(1.0 / 7.0), sqrt(1.0 / 7.0),
      sqrt(1.0 / 8.0), sqrt(1.0 / 8.0),
  };

  HelmOscillators::HelmOscillators() : Processor(kNumInputs, 1) {
    utils::zeroBuffer(oscillator1_cross_mods_, MAX_BUFFER_SIZE + 1);
    utils::zeroBuffer(oscillator2_cross_mods_, MAX_BUFFER_SIZE + 1);

    oscillator1_phase_base_ = 0.0;
    oscillator2_phase_base_ = 0.0;
    last_phase1_ = 0;
    last_phase2_ = 0;

    for (int v = 0; v < MAX_UNISON; ++v) {
      oscillator1_phases_[v] = 0;
      oscillator2_phases_[v] = 0;
      wave_buffers1_[v] = nullptr;
      wave_buffers2_[v] = nullptr;
      detune_diffs1_[v] = 0;
      detune_diffs2_[v] = 0;
    }

    // Initialize random buffers for Sample & Hold and Sample & Glide
    regenerateSampleAndHold(sample_hold_buffer1_);
    regenerateSampleAndHold(sample_hold_buffer2_);
    regenerateSampleAndGlide(sample_glide_buffer1_);
    regenerateSampleAndGlide(sample_glide_buffer2_);

    for (int i = 0; i < MAX_BUFFER_SIZE; ++i) {
      oscillator1_phase_diffs_[i] = 0;
      oscillator2_phase_diffs_[i] = 0;
    }
  }

  void HelmOscillators::reset(int i) {
    oscillator1_cross_mods_[i] = 0;
    oscillator2_cross_mods_[i] = 0;
    oscillator1_cross_mods_[i + 1] = 0;
    oscillator2_cross_mods_[i + 1] = 0;

    oscillator1_phase_base_ = 0.0;
    oscillator2_phase_base_ = 0.0;
    oscillator1_phases_[0] = 0;
    oscillator2_phases_[0] = 0;

    for (int u = 1; u < MAX_UNISON; ++u) {
      oscillator1_phases_[u] = (UINT_MAX / RAND_MAX) * rand();
      oscillator2_phases_[u] = (UINT_MAX / RAND_MAX) * rand();
    }
  }

  void HelmOscillators::loadBasePhaseInc() {
    int samples = buffer_size_;

    int* dest1 = oscillator1_phase_diffs_;
    int* dest2 = oscillator2_phase_diffs_;

    const mopo_float* src1 = input(kOscillator1PhaseInc)->source->buffer;
    const mopo_float* src2 = input(kOscillator2PhaseInc)->source->buffer;

    VECTORIZE_LOOP
    for (int i = 0; i < samples; ++i) {
      dest1[i] = UINT_MAX * src1[i];
      dest2[i] = UINT_MAX * src2[i];
    }

    for (int i = 1; i < samples; ++i) {
      dest1[i] += dest1[i - 1];
      dest2[i] += dest2[i - 1];
    }
  }

  void HelmOscillators::computeDetuneRatios(int* detune_diffs,
                                            int oscillator_diff,
                                            bool harmonize, mopo_float detune,
                                            int voices) {
    int harmonize_mult = harmonize ? 1 : 0;
    for (int v = 0; v < MAX_UNISON; ++v) {
      mopo_float amount = (detune * ((v + 1) / 2)) / ((voices + 1) / 2);

      if (v % 2)
        amount = -amount;

      mopo_float harmonic = harmonize_mult * v;

      mopo_float detune_ratio = harmonic + DetuneLookup::detuneLookup(amount);
      detune_diffs[v] = detune_ratio * oscillator_diff - oscillator_diff;
    }
  }

  void HelmOscillators::prepareBuffers(mopo_float** wave_buffers,
                                       const int* detune_diffs,
                                       const int* oscillator_phase_diffs,
                                       int waveform) {
    // Determine which buffer to use (oscillator 1 or 2)
    bool is_osc1 = (wave_buffers == wave_buffers1_);
    
    for (int v = 0; v < MAX_UNISON; ++v) {
      int phase_diff = detune_diffs[v] + oscillator_phase_diffs[0];
      int harmonic_index = FixedPointWave::getHarmonicIndex(phase_diff);
      
      // Use dynamic buffers for Sample & Hold and Sample & Glide
      if (waveform == FixedPointWaveLookup::kSampleAndHold) {
        wave_buffers[v] = is_osc1 ? sample_hold_buffer1_[harmonic_index] : sample_hold_buffer2_[harmonic_index];
      }
      else if (waveform == FixedPointWaveLookup::kSampleAndGlide) {
        wave_buffers[v] = is_osc1 ? sample_glide_buffer1_[harmonic_index] : sample_glide_buffer2_[harmonic_index];
      }
      else {
        wave_buffers[v] = FixedPointWave::getBuffer(waveform, phase_diff);
      }
    }
  }

  void HelmOscillators::processInitial() {
    loadBasePhaseInc();

    int voices1 = utils::iclamp(input(kUnisonVoices1)->source->buffer[0], 1, MAX_UNISON);
    int voices2 = utils::iclamp(input(kUnisonVoices2)->source->buffer[0], 1, MAX_UNISON);
    mopo_float detune1 = input(kUnisonDetune1)->source->buffer[0];
    mopo_float detune2 = input(kUnisonDetune2)->source->buffer[0];
    mopo_float harmonize1 = input(kHarmonize1)->source->buffer[0];
    mopo_float harmonize2 = input(kHarmonize2)->source->buffer[0];

    computeDetuneRatios(detune_diffs1_, oscillator1_phase_diffs_[0],
                        harmonize1, detune1, voices1);
    computeDetuneRatios(detune_diffs2_, oscillator2_phase_diffs_[0],
                        harmonize2, detune2, voices2);

    int wave1 = static_cast<int>(input(kOscillator1Waveform)->source->buffer[0] + 0.5);
    int wave2 = static_cast<int>(input(kOscillator2Waveform)->source->buffer[0] + 0.5);
    wave1 = utils::iclamp(wave1, 0, FixedPointWaveLookup::kNumFixedPointWaveforms - 1);
    wave2 = utils::iclamp(wave2, 0, FixedPointWaveLookup::kNumFixedPointWaveforms - 1);

    prepareBuffers(wave_buffers1_, detune_diffs1_, oscillator1_phase_diffs_, wave1);
    prepareBuffers(wave_buffers2_, detune_diffs2_, oscillator2_phase_diffs_, wave2);
  }

  void HelmOscillators::processCrossMod() {
    mopo_float cross_mod = input(kCrossMod)->at(0);
    const int* phase_diffs1 = oscillator1_phase_diffs_;
    const int* phase_diffs2 = oscillator2_phase_diffs_;
    int* dest_cross_mod2 = oscillator2_cross_mods_;
    int* dest_cross_mod1 = oscillator1_cross_mods_;

    if (cross_mod == 0.0) {
      utils::zeroBuffer(dest_cross_mod1, buffer_size_);
      utils::zeroBuffer(dest_cross_mod2, buffer_size_);
      return;
    }

    int i = 0;
    if (input(kReset)->source->triggered) {
      int trigger_offset = input(kReset)->source->trigger_offset;
      for (; i < trigger_offset; ++i) {
        tickCrossMod(i, cross_mod, dest_cross_mod1, dest_cross_mod2,
                     oscillator1_phase_base_ + phase_diffs1[i],
                     oscillator2_phase_base_ + phase_diffs2[i]);
      }

      oscillator1_cross_mods_[i] = 0;
      oscillator2_cross_mods_[i] = 0;
      oscillator1_cross_mods_[i + 1] = 0;
      oscillator2_cross_mods_[i + 1] = 0;

      oscillator1_phase_base_ = 0.0;
      oscillator2_phase_base_ = 0.0;
    }
    for (; i < buffer_size_; ++i) {
      tickCrossMod(i, cross_mod, dest_cross_mod1, dest_cross_mod2,
                   oscillator1_phase_base_ + phase_diffs1[i],
                   oscillator2_phase_base_ + phase_diffs2[i]);
    }
  }

  void HelmOscillators::processVoices() {
    int voices1 = utils::iclamp(input(kUnisonVoices1)->source->buffer[0], 1, MAX_UNISON);
    int voices2 = utils::iclamp(input(kUnisonVoices2)->source->buffer[0], 1, MAX_UNISON);

    utils::zeroBuffer(oscillator1_totals_, buffer_size_);
    utils::zeroBuffer(oscillator2_totals_, buffer_size_);

    int j = 0;
    if (input(kReset)->source->triggered) {
      int trigger_offset = input(kReset)->source->trigger_offset;
      for (; j < trigger_offset; ++j)
        tickInitialVoices(j);

      oscillator1_phases_[0] = 0;
      oscillator2_phases_[0] = 0;
      last_phase1_ = 0;
      last_phase2_ = 0;
      
      // Regenerate random waveforms on reset
      int wave1 = static_cast<int>(input(kOscillator1Waveform)->source->buffer[0] + 0.5);
      int wave2 = static_cast<int>(input(kOscillator2Waveform)->source->buffer[0] + 0.5);
      
      if (wave1 == FixedPointWaveLookup::kSampleAndHold) {
        regenerateSampleAndHold(sample_hold_buffer1_);
      }
      else if (wave1 == FixedPointWaveLookup::kSampleAndGlide) {
        regenerateSampleAndGlide(sample_glide_buffer1_);
      }
      
      if (wave2 == FixedPointWaveLookup::kSampleAndHold) {
        regenerateSampleAndHold(sample_hold_buffer2_);
      }
      else if (wave2 == FixedPointWaveLookup::kSampleAndGlide) {
        regenerateSampleAndGlide(sample_glide_buffer2_);
      }
    }

    for (; j < buffer_size_; ++j)
      tickInitialVoices(j);

    for (int v = 1; v < voices1; ++v) {
      const mopo_float* wave_buffer = wave_buffers1_[v];
      unsigned int start_phase = oscillator1_phases_[v];
      int detune = detune_diffs1_[v];

      int i = 0;
      if (input(kReset)->source->triggered) {
        int trigger_offset = input(kReset)->source->trigger_offset;
        for (; i < trigger_offset; ++i)
          tickVoice1(i, v, wave_buffer, start_phase, detune);

        oscillator1_phases_[v] = (UINT_MAX / RAND_MAX) * rand();
      }

      for (; i < buffer_size_; ++i)
        tickVoice1(i, v, wave_buffer, start_phase, detune);
    }

    for (int v = 1; v < voices2; ++v) {
      const mopo_float* wave_buffer = wave_buffers2_[v];
      unsigned int start_phase = oscillator2_phases_[v];
      int detune = detune_diffs2_[v];

      int i = 0;
      if (input(kReset)->source->triggered) {
        int trigger_offset = input(kReset)->source->trigger_offset;
        for (; i < trigger_offset; ++i)
          tickVoice2(i, v, wave_buffer, start_phase, detune);

        oscillator2_phases_[v] = (UINT_MAX / RAND_MAX) * rand();
      }
      for (; i < buffer_size_; ++i)
        tickVoice2(i, v, wave_buffer, start_phase, detune);
    }

    finishVoices(voices1, voices2);
  }

  void HelmOscillators::finishVoices(int voices1, int voices2) {
    mopo_float scale1 = scales[voices1];
    mopo_float scale2 = scales[voices2];

    mopo_float* dest = output()->buffer;
    const mopo_float* amp1 = input(kOscillator1Amplitude)->source->buffer;
    const mopo_float* amp2 = input(kOscillator2Amplitude)->source->buffer;
    const mopo_float* oscillator1_totals = oscillator1_totals_;
    const mopo_float* oscillator2_totals = oscillator2_totals_;

    VECTORIZE_LOOP
    for (int j = 0; j < buffer_size_; ++j)
      tickOut(j, dest, amp1, amp2, oscillator1_totals, oscillator2_totals, scale1, scale2);

    oscillator1_cross_mods_[0] = oscillator1_cross_mods_[buffer_size_];
    oscillator2_cross_mods_[0] = oscillator2_cross_mods_[buffer_size_];

    oscillator1_phase_base_ += oscillator1_phase_diffs_[buffer_size_ - 1];
    oscillator2_phase_base_ += oscillator2_phase_diffs_[buffer_size_ - 1];

    for (int v = 0; v < MAX_UNISON; ++v) {
      oscillator1_phases_[v] += oscillator1_phase_diffs_[buffer_size_ - 1] +
                                buffer_size_ * detune_diffs1_[v];
      oscillator2_phases_[v] += oscillator2_phase_diffs_[buffer_size_ - 1] +
                                buffer_size_ * detune_diffs2_[v];
    }
  }

  void HelmOscillators::process() {
    processInitial();
    processCrossMod();
    processVoices();
  }

  void HelmOscillators::regenerateSampleAndHold(mopo_float buffer[][2 * FixedPointWaveLookup::FIXED_LOOKUP_SIZE]) {
    // Generate Sample & Hold waveform: random values that stay constant
    for (int h = 0; h < FixedPointWaveLookup::HARMONICS + 1; ++h) {
      mopo_float current_value = (2.0 * rand()) / RAND_MAX - 1.0;
      
      for (int i = 0; i < FixedPointWaveLookup::FIXED_LOOKUP_SIZE; ++i) {
        // Change value at each step
        if (i % (FixedPointWaveLookup::FIXED_LOOKUP_SIZE / 8) == 0)
          current_value = (2.0 * rand()) / RAND_MAX - 1.0;
        
        buffer[h][i] = current_value;
        // Diff for interpolation (stays constant for S&H)
        buffer[h][i + FixedPointWaveLookup::FIXED_LOOKUP_SIZE] = 0.0;
      }
    }
  }

  void HelmOscillators::regenerateSampleAndGlide(mopo_float buffer[][2 * FixedPointWaveLookup::FIXED_LOOKUP_SIZE]) {
    // Generate Sample & Glide waveform: smoothly interpolated random values
    for (int h = 0; h < FixedPointWaveLookup::HARMONICS + 1; ++h) {
      const int steps = 8; // Number of random values per cycle
      const int samples_per_step = FixedPointWaveLookup::FIXED_LOOKUP_SIZE / steps;
      
      mopo_float random_values[steps + 1];
      for (int s = 0; s <= steps; ++s) {
        random_values[s] = (2.0 * rand()) / RAND_MAX - 1.0;
      }
      
      for (int i = 0; i < FixedPointWaveLookup::FIXED_LOOKUP_SIZE; ++i) {
        int step = i / samples_per_step;
        if (step >= steps) step = steps - 1;
        
        mopo_float t = (mopo_float)(i % samples_per_step) / samples_per_step;
        // Smooth interpolation using cosine
        mopo_float smooth_t = (1.0 - cos(PI * t)) / 2.0;
        
        mopo_float value = utils::interpolate(random_values[step], random_values[step + 1], smooth_t);
        buffer[h][i] = value;
        
        // Calculate diff for next sample
        mopo_float next_t = (mopo_float)((i + 1) % samples_per_step) / samples_per_step;
        mopo_float next_smooth_t = (1.0 - cos(PI * next_t)) / 2.0;
        mopo_float next_value = utils::interpolate(random_values[step], random_values[step + 1], next_smooth_t);
        buffer[h][i + FixedPointWaveLookup::FIXED_LOOKUP_SIZE] = next_value - value;
      }
    }
  }
} // namespace mopo


