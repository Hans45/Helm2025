/* Copyright 2013-2017 Matt Tytel
 *
 * mopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mopo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "helm2025_lfo.h"

#include "common.h"
#include "utils.h"
#include "synced_random.h"

#include <cmath>

namespace mopo {

  namespace {
    mopo_float randomLfoValue() {
      return 2.0 * rand() / RAND_MAX - 1.0;
    }
  } // namespace



  HelmLfo::HelmLfo() : Processor(kNumInputs, kNumOutputs, false), offset_(0.0),
                       cycle_seed_(0), cycle_count_(0), randoms_(), random_index_(0) { }

  void HelmLfo::process() {
    int reset_offset = -1;
    if (input(kReset)->source->triggered) {
      reset_offset = input(kReset)->source->trigger_offset;
    }

    Wave::Type waveform = static_cast<Wave::Type>(static_cast<int>(input(kWaveform)->at(0)));
    mopo_float frequency = input(kFrequency)->at(0);
    mopo_float phase = input(kPhase)->at(0);
    mopo_float delta = frequency / sample_rate_;

    // Définir la résolution du cycle (nombre de steps randoms par cycle)
    int cycle_resolution = 64; // Valeur par défaut, peut être ajustée
    if (waveform == Wave::kSampleAndHold || waveform == Wave::kSampleAndGlide) {
      // Pour S&H/S&G, résolution musicale fixe
      cycle_resolution = 16;
    } else if (waveform == Wave::kWhiteNoise) {
      // WhiteNoise : résolution audio (pour affichage fluide)
      cycle_resolution = std::max(8, std::min(512, static_cast<int>(sample_rate_ / std::max(1.0, frequency))));
    }

    mopo_float* osc_phase_buffer = output(kOscPhase)->buffer;
    mopo_float* value_buffer = output(kValue)->buffer;

    for (int i = 0; i < samples_to_process_; ++i) {
      // Reset if triggered at this sample
      if (i == reset_offset) {
        offset_ = 0.0;
        // Nouveau cycle : seed unique basé sur le compteur de cycles
        cycle_seed_ = static_cast<uint32_t>(cycle_count_++);
        randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution);
        random_index_ = 0;
      }

      mopo_float offset_integral;
      mopo_float current_offset = utils::mod(offset_, &offset_integral);
      mopo_float phase_integral;
      mopo_float phased_offset = utils::mod(current_offset + phase, &phase_integral);

      // Détection du wrap de cycle (free-run) : renouvellement du seed et du random
      if (phased_offset < last_phased_offset_) {
        cycle_seed_ = static_cast<uint32_t>(cycle_count_++);
        randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution);
        random_index_ = 0;
      }
      last_phased_offset_ = phased_offset;

      osc_phase_buffer[i] = phased_offset;

      // Pour S&H/S&G/WhiteNoise, utiliser la séquence synchronisée
      if (waveform == Wave::kWhiteNoise) {
        // White noise : nouvelle valeur à chaque sample
        if (randoms_.empty()) randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution);
        value_buffer[i] = randoms_[(random_index_++) % cycle_resolution];
      }
      else if (waveform == Wave::kSampleAndHold) {
        // S&H : valeur constante sur chaque step
        int step = static_cast<int>(phased_offset * cycle_resolution);
        if (randoms_.empty()) randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution);
        value_buffer[i] = randoms_[std::min(step, cycle_resolution - 1)];
      }
      else if (waveform == Wave::kSampleAndGlide) {
        // S&G : interpolation entre deux randoms synchronisés
        float phasef = phased_offset * (cycle_resolution - 1);
        int index = static_cast<int>(phasef);
        float t = phasef - index;
        if (randoms_.empty()) randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution);
        float r1 = randoms_[std::min(index, cycle_resolution - 1)];
        float r2 = randoms_[std::min(index + 1, cycle_resolution - 1)];
        value_buffer[i] = utils::interpolate(r1, r2, t);
      }
      else {
        value_buffer[i] = Wave::wave(waveform, phased_offset);
      }

      offset_ += delta;
    }
  }

  void HelmLfo::correctToTime(mopo_float samples) {
    mopo_float frequency = input(kFrequency)->at(0);
    offset_ = samples * frequency / sample_rate_;
    mopo_float integral;
    offset_ = utils::mod(offset_, &integral);
    // Réinitialiser la séquence random si besoin
    randoms_.clear();
  }
// Membres privés à ajouter dans helm2025_lfo.h :
// uint32_t cycle_seed_;
// uint64_t cycle_count_;
// std::vector<float> randoms_;
// int random_index_;
} // namespace mopo


