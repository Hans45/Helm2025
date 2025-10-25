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
#ifndef COMMON_H
#define COMMON_H

// Utilities.
#define UNUSED(x) (void)(x)

// Processor cloning macro (resolves TODO in processor.h)
#define MOPO_DECLARE_CLONE(ClassName) \
  virtual Processor* clone() const override { \
    return new ClassName(*this); \
  }

// C++17 modernization helpers
#if __cplusplus >= 201703L
  #define MOPO_CONSTEXPR inline constexpr
#else
  #define MOPO_CONSTEXPR static const
#endif

// Debugging.
#if DEBUG
#include <cassert>
#define MOPO_ASSERT(x) assert(x)
#else
#define MOPO_ASSERT(x) ((void)0)
#endif // DEBUG

#ifdef __clang__
#define VECTORIZE_LOOP _Pragma("clang loop vectorize(enable) interleave(enable)")
#elif _MSC_VER
#define VECTORIZE_LOOP __pragma(loop(hint_parallel(8)))
#else
#define VECTORIZE_LOOP
#endif

namespace mopo {

  using mopo_float = double;  // C++11 alias instead of typedef

  MOPO_CONSTEXPR mopo_float PI = 3.1415926535897932384626433832795;
  MOPO_CONSTEXPR int MAX_BUFFER_SIZE = 256;
  MOPO_CONSTEXPR int DEFAULT_BUFFER_SIZE = 256;
  MOPO_CONSTEXPR int DEFAULT_SAMPLE_RATE = 44100;
  MOPO_CONSTEXPR int MAX_SAMPLE_RATE = 192000;
  MOPO_CONSTEXPR int MIDI_SIZE = 128;
  MOPO_CONSTEXPR int MAX_POLYPHONY = 33;

  MOPO_CONSTEXPR int PPQ = 960; // Pulses per quarter note.
  MOPO_CONSTEXPR mopo_float VOICE_KILL_TIME = 0.02;
  MOPO_CONSTEXPR int NUM_MIDI_CHANNELS = 16;

  // Common types of events across different Processors.
  enum VoiceEvent {
    kVoiceOff,     // Stop. (e.g. release in an envelope)
    kVoiceOn,      // Start. (e.g. start attack in an envelope)
    kVoiceReset,   // Reset. Immediately reset to initial state and play.
    kVoiceKill,   // Kill. Silence voice as fast as possible.
  };
} // namespace mopo

#endif // COMMON_H
