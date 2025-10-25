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

#pragma once
#ifndef HELM_ENGINE_H
#define HELM_ENGINE_H

#include "mopo.h"
#include "helm_common.h"
#include "midi_event.h"
#include "midi_queue.h"
#include "helm_module.h"

namespace mopo {
  class Arpeggiator;
  class HelmVoiceHandler;
  class HelmLfo;
  class PeakMeter;
  class Value;
  class ValueSwitch;

  // The overall helm engine. All audio processing is contained in here.
  class HelmEngine final : public HelmModule, public NoteHandler, public MidiEventHandler {
    public:
      using ModConnectionSet = std::set<ModulationConnection*>;
      using EventQueue = MidiEventQueue;

      HelmEngine();
      ~HelmEngine() override;

      void init() override;
      void process() noexcept override;
      void setBufferSize(int buffer_size) noexcept override;
      void setSampleRate(int sample_rate) noexcept override;

      void handleMidiEvent(const MidiEvent& event) noexcept override;

      const ModConnectionSet& getModulationConnections() const noexcept { return mod_connections_; }
      bool isModulationActive(ModulationConnection* connection) const noexcept;
      CircularQueue<mopo::mopo_float>& getPressedNotes() noexcept;
      void connectModulation(ModulationConnection* connection) noexcept;
      void disconnectModulation(ModulationConnection* connection) noexcept;
      [[nodiscard]] int getNumActiveVoices() const noexcept;
      [[nodiscard]] mopo_float getLastActiveNote() const noexcept;

      // Keyboard events.
      void allNotesOff(int sample = 0) noexcept override;
      void noteOn(mopo_float note, mopo_float velocity = 1.0,
                  int sample = 0, int channel = 0) noexcept override;
      [[nodiscard]] VoiceEvent noteOff(mopo_float note, int sample = 0) noexcept override;
      void setModWheel(mopo_float value, int channel = 0) noexcept;
      void setPitchWheel(mopo_float value, int channel = 0) noexcept;
      void setBpm(mopo_float bpm) noexcept;
      void correctToTime(mopo_float samples) noexcept override;
      void setAftertouch(mopo_float note, mopo_float value, int sample = 0) noexcept;
      void setChannelAftertouch(int channel, mopo_float value, int sample = 0) noexcept;

      // Sustain pedal events.
      void sustainOn() noexcept;
      void sustainOff() noexcept;

    private:
  HelmVoiceHandler* voice_handler_;
  Arpeggiator* arpeggiator_;
      ValueSwitch* arp_on_;
      bool was_playing_arp_;

      Value* lfo_1_retrigger_;
      Value* lfo_2_retrigger_;
      Value* step_sequencer_retrigger_;
      Value* bps_;
  HelmLfo* lfo_1_;
  HelmLfo* lfo_2_;
  PeakMeter* peak_meter_;
  StepGenerator* step_sequencer_;

      std::set<ModulationConnection*> mod_connections_;
  };
} // namespace mopo

#endif // HELM_ENGINE_H
