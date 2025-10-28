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

#include "fixed_point_wave.h"

namespace mopo {

  FixedPointWaveLookup::FixedPointWaveLookup() {
    preprocessSin();
    preprocessTriangle();
    preprocessSquare();
    preprocessUpSaw();
    preprocessDownSaw();
    preprocessStep<3>(three_step_);
    preprocessStep<4>(four_step_);
    preprocessStep<8>(eight_step_);
    preprocessPyramid<3>(three_pyramid_);
    preprocessPyramid<5>(five_pyramid_);
    preprocessPyramid<9>(nine_pyramid_);
    preprocessPulse25();
    preprocessPulse10();
    preprocessSawSquare();
    preprocessTriangleSquare();
    preprocessSkewedSine();
    preprocessFoldedSine();
    preprocessSuperSaw();
    preprocessChirp();

    wave_type waves[kNumFixedPointWaveforms] =
        { sin_, triangle_, square_, down_saw_, up_saw_,
          three_step_, four_step_, eight_step_,
          three_pyramid_, five_pyramid_, nine_pyramid_,
          sin_,  // kSampleAndHold - placeholder for random values
          sin_,  // kSampleAndGlide - placeholder for smooth random values
          pulse25_, pulse10_, saw_square_, triangle_square_,
          skewed_sine_, folded_sine_, super_saw_, chirp_,
          sin_   // kWhiteNoise - placeholder for white noise
        };

    memcpy(waves_, waves, kNumFixedPointWaveforms * sizeof(wave_type));
  }

  void FixedPointWaveLookup::preprocessSin() {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i)
        sin_[h][i] = sin((2 * PI * i) / FIXED_LOOKUP_SIZE);
    }

    preprocessDiffs(sin_);
  }

  void FixedPointWaveLookup::preprocessTriangle() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      triangle_[0][i] = Wave::triangle((1.0 * i) / FIXED_LOOKUP_SIZE);

      int p = i;
      mopo_float scale = 8.0 / (PI * PI);
      triangle_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        triangle_[HARMONICS - h][i] = triangle_[HARMONICS - h + 1][i];
        mopo_float harmonic = scale * sin_[0][p] / ((h + 1) * (h + 1));

        if (h % 4 == 0)
          triangle_[HARMONICS - h][i] += harmonic;
        else if (h % 2 == 0)
          triangle_[HARMONICS - h][i] -= harmonic;
      }
    }

    preprocessDiffs(triangle_);
  }

  void FixedPointWaveLookup::preprocessSquare() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      square_[0][i] = Wave::square((1.0 * i) / FIXED_LOOKUP_SIZE);

      int p = i;
      mopo_float scale = 4.0 / PI;
      square_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        square_[HARMONICS - h][i] = square_[HARMONICS - h + 1][i];

        if (h % 2 == 0)
          square_[HARMONICS - h][i] += scale * sin_[0][p] / (h + 1);
      }
    }

    preprocessDiffs(square_);
  }

  void FixedPointWaveLookup::preprocessDownSaw() {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i)
        down_saw_[h][i] = -up_saw_[h][i];
    }

    preprocessDiffs(down_saw_);
  }

  void FixedPointWaveLookup::preprocessUpSaw() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      up_saw_[0][i] = Wave::upsaw((1.0 * i) / FIXED_LOOKUP_SIZE);

      int index = (i + (FIXED_LOOKUP_SIZE / 2)) % FIXED_LOOKUP_SIZE;
      int p = i;
      mopo_float scale = 2.0 / PI;
      up_saw_[HARMONICS][index] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        mopo_float harmonic = scale * sin_[0][p] / (h + 1);

        if (h % 2 == 0)
          up_saw_[HARMONICS - h][index] = up_saw_[HARMONICS - h + 1][index] + harmonic;
        else
          up_saw_[HARMONICS - h][index] = up_saw_[HARMONICS - h + 1][index] - harmonic;
      }
    }

    preprocessDiffs(up_saw_);
  }

  template<size_t steps>
  void FixedPointWaveLookup::preprocessStep(wave_type buffer) {
    static int num_steps = steps;
    static const mopo_float step_size = num_steps / (num_steps - 1.0);

    for (int h = 0; h < HARMONICS + 1; ++h) {
      int base_num_harmonics = HARMONICS + 1 - h;
      int harmony_num_harmonics = base_num_harmonics / num_steps;
      int harmony_h = HARMONICS + 1 - harmony_num_harmonics;

      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
        buffer[h][i] = step_size * up_saw_[h][i];

        if (harmony_num_harmonics) {
          int harm_index = (num_steps * i) % FIXED_LOOKUP_SIZE;
          buffer[h][i] += step_size * down_saw_[harmony_h][harm_index] / num_steps;
        }
      }
    }

    preprocessDiffs(buffer);
  }

  template<size_t steps>
  void FixedPointWaveLookup::preprocessPyramid(wave_type buffer) {
    static const int squares = steps - 1;
    static const int offset = 3 * FIXED_LOOKUP_SIZE / 4;

    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
        buffer[h][i] = 0;

        for (size_t s = 0; s < squares; ++s) {
          int square_offset = (s * FIXED_LOOKUP_SIZE) / (2 * squares);
          int phase = (i + offset + square_offset) % FIXED_LOOKUP_SIZE;
          buffer[h][i] += square_[h][phase] / squares;
        }
      }
    }

    preprocessDiffs(buffer);
  }

  void FixedPointWaveLookup::preprocessPulse25() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      pulse25_[0][i] = (i < FIXED_LOOKUP_SIZE / 4) ? 1.0 : -1.0;

      int p = i;
      mopo_float scale = 4.0 / PI;
      pulse25_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        pulse25_[HARMONICS - h][i] = pulse25_[HARMONICS - h + 1][i];

        mopo_float harmonic_mult = sin((h + 1) * PI / 4.0) / (h + 1);
        pulse25_[HARMONICS - h][i] += 2.0 * scale * harmonic_mult * sin_[0][p];
      }
    }
    preprocessDiffs(pulse25_);
  }

  void FixedPointWaveLookup::preprocessPulse10() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      pulse10_[0][i] = (i < FIXED_LOOKUP_SIZE / 10) ? 1.0 : -1.0;

      int p = i;
      mopo_float scale = 4.0 / PI;
      pulse10_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        pulse10_[HARMONICS - h][i] = pulse10_[HARMONICS - h + 1][i];

        mopo_float harmonic_mult = sin((h + 1) * PI / 10.0) / (h + 1);
        pulse10_[HARMONICS - h][i] += 2.0 * scale * harmonic_mult * sin_[0][p];
      }
    }
    preprocessDiffs(pulse10_);
  }

  void FixedPointWaveLookup::preprocessSawSquare() {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
        saw_square_[h][i] = 0.6 * down_saw_[h][i] + 0.4 * square_[h][i];
      }
    }
    preprocessDiffs(saw_square_);
  }

  void FixedPointWaveLookup::preprocessTriangleSquare() {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
        triangle_square_[h][i] = 0.7 * triangle_[h][i] + 0.3 * square_[h][i];
      }
    }
    preprocessDiffs(triangle_square_);
  }

  void FixedPointWaveLookup::preprocessSkewedSine() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      mopo_float t = (1.0 * i) / FIXED_LOOKUP_SIZE;
      // Asymmetric sine wave with different rise/fall times
      mopo_float skewed_t = t < 0.5 ? t * t * 2.0 : 1.0 - (1.0 - t) * (1.0 - t) * 2.0;
      skewed_sine_[0][i] = sin(2 * PI * skewed_t);

      int p = i;
      mopo_float scale = 1.0;
      skewed_sine_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        skewed_sine_[HARMONICS - h][i] = skewed_sine_[HARMONICS - h + 1][i];

        mopo_float harmonic_mult = 1.0 / ((h + 1) * (h + 1));
        skewed_sine_[HARMONICS - h][i] += scale * harmonic_mult * sin_[0][p];
      }
    }
    preprocessDiffs(skewed_sine_);
  }

  void FixedPointWaveLookup::preprocessFoldedSine() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      mopo_float t = (1.0 * i) / FIXED_LOOKUP_SIZE;
      mopo_float sine_val = sin(2 * PI * t);
      // Wave folding effect
      folded_sine_[0][i] = sine_val > 0.5 ? 1.0 - sine_val : (sine_val < -0.5 ? -1.0 - sine_val : sine_val);

      int p = i;
      mopo_float scale = 1.0;
      folded_sine_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        folded_sine_[HARMONICS - h][i] = folded_sine_[HARMONICS - h + 1][i];
        folded_sine_[HARMONICS - h][i] += scale * sin_[0][p] / (h + 1);
      }
    }
    preprocessDiffs(folded_sine_);
  }

  void FixedPointWaveLookup::preprocessSuperSaw() {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
        super_saw_[h][i] = 0.0;
        // Superposition of 7 slightly detuned saw waves
        for (int voice = 0; voice < 7; ++voice) {
          mopo_float detune = (voice - 3) * 0.1; // Small detuning
          int phase_offset = (int)(detune * FIXED_LOOKUP_SIZE);
          int offset_index = (i + phase_offset) % FIXED_LOOKUP_SIZE;
          if (offset_index < 0) offset_index += FIXED_LOOKUP_SIZE;
          super_saw_[h][i] += down_saw_[h][offset_index] / 7.0;
        }
      }
    }
    preprocessDiffs(super_saw_);
  }

  void FixedPointWaveLookup::preprocessChirp() {
    for (int i = 0; i < FIXED_LOOKUP_SIZE; ++i) {
      mopo_float t = (1.0 * i) / FIXED_LOOKUP_SIZE;
      // Frequency sweep from low to high
      mopo_float frequency_mult = 1.0 + 4.0 * t; // 1x to 5x frequency
      chirp_[0][i] = sin(2 * PI * t * frequency_mult);

      int p = i;
      mopo_float scale = 1.0;
      chirp_[HARMONICS][i] = scale * sin_[0][p];

      for (int h = 1; h < HARMONICS; ++h) {
        p = (p + i) % FIXED_LOOKUP_SIZE;
        chirp_[HARMONICS - h][i] = chirp_[HARMONICS - h + 1][i];
        chirp_[HARMONICS - h][i] += scale * sin_[0][p] / (h + 1);
      }
    }
    preprocessDiffs(chirp_);
  }

  void FixedPointWaveLookup::preprocessDiffs(wave_type wave) {
    for (int h = 0; h < HARMONICS + 1; ++h) {
      for (int i = 0; i < FIXED_LOOKUP_SIZE - 1; ++i)
        wave[h][i + FIXED_LOOKUP_SIZE] = FRACTIONAL_MULT * (wave[h][i + 1] - wave[h][i]);

      mopo_float last_delta = wave[h][0] - wave[h][FIXED_LOOKUP_SIZE - 1];
      wave[h][2 * FIXED_LOOKUP_SIZE - 1] = FRACTIONAL_MULT * last_delta;
    }
  }

  const FixedPointWaveLookup FixedPointWave::lookup_;
} // namespace mopo


