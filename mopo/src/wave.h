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

#pragma once
#ifndef WAVE_H
#define WAVE_H

#include "common.h"
#include "utils.h"
#include <cmath>
#include <cstdlib>

#define LOOKUP_SIZE 2048
#define HIGH_FREQUENCY 20000
#define MAX_HARMONICS 100

namespace mopo {

  class WaveLookup {
    public:
      WaveLookup() {
        // Sin lookup table.
        for (int i = 0; i < LOOKUP_SIZE + 1; ++i)
          sin_[i] = sin((2 * PI * i) / LOOKUP_SIZE);

        // Square lookup table.
        for (int i = 0; i < LOOKUP_SIZE + 1; ++i) {
          int p = i;
          mopo_float scale = 4.0 / PI;
          square_[0][i] = scale * sin_[p];

          for (int h = 1; h < MAX_HARMONICS; ++h) {
            p = (p + i) % LOOKUP_SIZE;
            square_[h][i] = square_[h - 1][i];

            if (h % 2 == 0)
              square_[h][i] += scale * sin_[p] / (h + 1);
          }
        }

        // Saw lookup table.
        for (int i = 0; i < LOOKUP_SIZE + 1; ++i) {
          int index = (i + (LOOKUP_SIZE / 2)) % LOOKUP_SIZE;
          int p = i;
          mopo_float scale = 2.0 / PI;
          saw_[0][index] = scale * sin_[p];

          for (int h = 1; h < MAX_HARMONICS; ++h) {
            p = (p + i) % LOOKUP_SIZE;
            mopo_float harmonic = scale * sin_[p] / (h + 1);

            if (h % 2 == 0)
              saw_[h][index] = saw_[h - 1][index] + harmonic;
            else
              saw_[h][index] = saw_[h - 1][index] - harmonic;
          }
        }

        // Triangle lookup table.
        for (int i = 0; i < LOOKUP_SIZE + 1; ++i) {
          int p = i;
          mopo_float scale = 8.0 / (PI * PI);
          triangle_[0][i] = scale * sin_[p];

          for (int h = 1; h < MAX_HARMONICS; ++h) {
            p = (p + i) % LOOKUP_SIZE;
            triangle_[h][i] = triangle_[h - 1][i];
            mopo_float harmonic = scale * sin_[p] / ((h + 1) * (h + 1));

            if (h % 4 == 0)
              triangle_[h][i] += harmonic;
            else if (h % 2 == 0)
              triangle_[h][i] -= harmonic;
          }
        }
      }

      static inline const WaveLookup* instance() {
        static const WaveLookup lookup;
        return &lookup;
      }

      inline mopo_float fullsin(mopo_float t) const {
        mopo_float integral;
        mopo_float fractional = utils::mod(t * LOOKUP_SIZE, &integral);
        int index = integral;
        return utils::interpolate(sin_[index], sin_[index + 1], fractional);
      }

      inline mopo_float square(mopo_float t, int harmonics) const {
        mopo_float integral;
        mopo_float fractional = utils::mod(t * LOOKUP_SIZE, &integral);
        int index = integral;
        return utils::interpolate(square_[harmonics][index],
                                  square_[harmonics][index + 1], fractional);
      }

      inline mopo_float upsaw(mopo_float t, int harmonics) const {
        mopo_float integral;
        mopo_float fractional = utils::mod(t * LOOKUP_SIZE, &integral);
        int index = integral;
        return utils::interpolate(saw_[harmonics][index],
                                  saw_[harmonics][index + 1], fractional);
      }

      inline mopo_float downsaw(mopo_float t, int harmonics) const {
        return -upsaw(t, harmonics);
      }

      inline mopo_float triangle(mopo_float t, int harmonics) const {
        mopo_float integral;
        mopo_float fractional = utils::mod(t * LOOKUP_SIZE, &integral);
        int index = integral;
        return utils::interpolate(triangle_[harmonics][index],
                                  triangle_[harmonics][index + 1], fractional);
      }

      template<size_t steps>
      inline mopo_float step(mopo_float t, int harmonics) const {
        static const mopo_float step_size = (1.0 * steps) / (steps - 1);
        return step_size * (upsaw(t, harmonics) +
               downsaw(steps * t, harmonics / steps) / steps);
      }

      template<size_t steps>
      inline mopo_float pyramid(mopo_float t, int harmonics) const {
        static const size_t squares = steps - 1;
        static const mopo_float phase_increment = 1.0 / (2.0 * squares);

        mopo_float phase = 0.75 + t;
        mopo_float out = 0.0;

        mopo_float integral;
        for (size_t i = 0; i < squares; ++i) {
          out += square(utils::mod(phase, &integral), harmonics);
          phase += phase_increment;
        }
        out /= squares;
        return out;
      }

    private:
      // Make them 1 larger for wrapping.
      mopo_float sin_[LOOKUP_SIZE + 1];
      mopo_float square_[MAX_HARMONICS][LOOKUP_SIZE + 1];
      mopo_float saw_[MAX_HARMONICS][LOOKUP_SIZE + 1];
      mopo_float triangle_[MAX_HARMONICS][LOOKUP_SIZE + 1];
  };

  class Wave {
    public:
      enum Type {
        kSin,
        kTriangle,
        kSquare,
        kDownSaw,
        kUpSaw,
        kThreeStep,
        kFourStep,
        kEightStep,
        kThreePyramid,
        kFivePyramid,
        kNinePyramid,
        kSampleAndHold,    // index 11 - corresponds to "sample and hold"
        kSampleAndGlide,   // index 12 - corresponds to "sample and glide"
        kPulse25,          // index 13 - corresponds to "pulse 25%"
        kPulse10,          // index 14 - corresponds to "pulse 10%"
        kSawSquare,        // index 15 - corresponds to "saw+square"
        kTriangleSquare,   // index 16 - corresponds to "tri+square"
        kSkewedSine,       // index 17 - corresponds to "skewed sine"
        kFoldedSine,       // index 18 - corresponds to "folded sine"
        kSuperSaw,         // index 19 - corresponds to "super saw"
        kChirp,            // index 20 - corresponds to "chirp"
        kWhiteNoise,       // Keep for backward compatibility but not used in UI
        kNumWaveforms
      };

      static inline mopo_float blwave(Wave::Type waveform, mopo_float t,
                                      mopo_float frequency) {
        const WaveLookup* lookup = WaveLookup::instance();
        if (fabs(frequency) < 1)
          return Wave::wave(waveform, t);
        int harmonics = HIGH_FREQUENCY / fabs(frequency) - 1.0;
        if (harmonics >= MAX_HARMONICS)
          return Wave::wave(waveform, t);

        switch (waveform) {
          case Wave::kSin:
            return lookup->fullsin(t);
          case Wave::kTriangle:
            return lookup->triangle(t, harmonics);
          case Wave::kSquare:
            return lookup->square(t, harmonics);
          case Wave::kDownSaw:
            return lookup->downsaw(t, harmonics);
          case Wave::kUpSaw:
            return lookup->upsaw(t, harmonics);
          case Wave::kThreeStep:
            return lookup->step<3>(t, harmonics);
          case Wave::kFourStep:
            return lookup->step<4>(t, harmonics);
          case Wave::kEightStep:
            return lookup->step<8>(t, harmonics);
          case Wave::kThreePyramid:
            return lookup->pyramid<3>(t, harmonics);
          case Wave::kFivePyramid:
            return lookup->pyramid<5>(t, harmonics);
          case Wave::kNinePyramid:
            return lookup->pyramid<9>(t, harmonics);
          case Wave::kSampleAndHold:
          case Wave::kSampleAndGlide:
          case Wave::kWhiteNoise:
          case Wave::kPulse25:
          case Wave::kPulse10:
          case Wave::kSawSquare:
          case Wave::kTriangleSquare:
          case Wave::kSkewedSine:
          case Wave::kFoldedSine:
          case Wave::kSuperSaw:
          case Wave::kChirp:
            // For new waveforms, fall back to non-bandlimited version for now
            return Wave::wave(waveform, t);
          default:
            return Wave::wave(waveform, t);
        }
      }

      static inline mopo_float wave(Type waveform, mopo_float t) {
        switch (waveform) {
          case kSin:
            return fullsin(t);
          case kSquare:
            return square(t);
          case kTriangle:
            return triangle(t);
          case kDownSaw:
            return downsaw(t);
          case kUpSaw:
            return upsaw(t);
          case kThreeStep:
            return step<3>(t);
          case kFourStep:
            return step<4>(t);
          case kEightStep:
            return step<8>(t);
          case kThreePyramid:
            return pyramid<3>(t);
          case kFivePyramid:
            return pyramid<5>(t);
          case kNinePyramid:
            return pyramid<9>(t);
          case kSampleAndHold:
            return whitenoise();  // Sample and hold uses random values
          case kSampleAndGlide:
            return whitenoise();  // Sample and glide uses smooth random values
          case kPulse25:
            return pulse25(t);
          case kPulse10:
            return pulse10(t);
          case kSawSquare:
            return 0.6 * downsaw(t) + 0.4 * square(t);
          case kTriangleSquare:
            return 0.7 * triangle(t) + 0.3 * square(t);
          case kSkewedSine:
            return skewedSine(t);
          case kFoldedSine:
            return foldedSine(t);
          case kSuperSaw:
            return superSaw(t);
          case kChirp:
            return chirp(t);
          default:
            return 0;
        }
      }

      static inline mopo_float nullwave() {
        return 0;
      }

      static inline mopo_float whitenoise() {
        return (2.0 * rand()) / RAND_MAX - 1;
      }

      static inline mopo_float fullsin(mopo_float t) {
        return utils::quickSin1(t);
      }

      static inline mopo_float square(mopo_float t) {
        return t < 0.5 ? 1 : -1;
      }

      static inline mopo_float triangle(mopo_float t) {
        mopo_float integral;
        return fabs(2.0 - 4.0 * utils::mod(t + 0.75, &integral)) - 1;
      }

      static inline mopo_float downsaw(mopo_float t) {
        return -upsaw(t);
      }

      static inline mopo_float upsaw(mopo_float t) {
        return t * 2 - 1;
      }

      static inline mopo_float hannwave(mopo_float t) {
        return 0.5 * (1.0 - cosf(2.0 * PI * t));
      }

      static inline mopo_float pulse25(mopo_float t) {
        return t < 0.25 ? 1 : -1;
      }

      static inline mopo_float pulse10(mopo_float t) {
        return t < 0.1 ? 1 : -1;
      }

      static inline mopo_float skewedSine(mopo_float t) {
        // Asymmetric sine wave with different rise/fall times
        mopo_float skewed_t = t < 0.5 ? t * t * 2.0 : 1.0 - (1.0 - t) * (1.0 - t) * 2.0;
        return utils::quickSin1(skewed_t);
      }

      static inline mopo_float foldedSine(mopo_float t) {
        mopo_float sine_val = utils::quickSin1(t);
        // Wave folding effect
        return sine_val > 0.5 ? 1.0 - sine_val : (sine_val < -0.5 ? -1.0 - sine_val : sine_val);
      }

      static inline mopo_float superSaw(mopo_float t) {
        mopo_float sum = 0.0;
        // Superposition of 7 slightly detuned saw waves
        for (int voice = 0; voice < 7; ++voice) {
          mopo_float detune = (voice - 3) * 0.01; // Small detuning
          mopo_float detuned_t = t + detune;
          // Normalize to [0,1]
          mopo_float integral;
          detuned_t = utils::mod(detuned_t, &integral);
          sum += upsaw(detuned_t) / 7.0;
        }
        return sum;
      }

      static inline mopo_float chirp(mopo_float t) {
        // Frequency sweep from low to high
        mopo_float frequency_mult = 1.0 + 4.0 * t; // 1x to 5x frequency
        return utils::quickSin1(t * frequency_mult);
      }

      template<size_t steps>
      static inline mopo_float step(mopo_float t) {
        mopo_float section = (int)(steps * t);
        return 2 * section / (steps - 1) - 1;
      }

      template<size_t steps>
      static inline mopo_float pyramid(mopo_float t) {
        static const size_t squares = steps - 1;
        static const mopo_float phase_increment = 1.0 / (2.0 * squares);

        mopo_float phase = 0.75 + t;
        mopo_float out = 0.0;

        mopo_float integral;
        for (size_t i = 0; i < squares; ++i) {
          out += square(utils::mod(phase, &integral));
          phase += phase_increment;
        }
        out /= squares;
        return out;
      }
  };
} // namespace mopo

#endif // WAVE_H
