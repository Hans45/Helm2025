#include "user_preferences.h"
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

#include "helm2025_editor.h"

#include "default_look_and_feel.h"
#include "helm2025_common.h"
#include "load_save.h"
#include "mopo.h"
#include "startup.h"
#include "utils.h"

#define MAX_OUTPUT_MEMORY 1048576
#define MAX_BUFFER_PROCESS 256

HelmEditor::HelmEditor(bool use_gui) : SynthGuiInterface(this, use_gui) {
  // Initialisation par défaut
  // Application des préférences utilisateur AVANT initialisation du device audio
  // Charger toutes les préférences audio/MIDI
  juce::String audioType = UserPreferences::loadAudioDeviceType();
  juce::String audioDevice = UserPreferences::loadAudioDevice();
  double audioSampleRate = UserPreferences::loadAudioSampleRate();
  int audioBufferSize = UserPreferences::loadAudioBufferSize();
  std::vector<juce::String> audioInputPorts = UserPreferences::loadAudioInputPorts();
  std::vector<juce::String> audioOutputPorts = UserPreferences::loadAudioOutputPorts();
    // (JUCE ne permet pas de forcer les ports actifs via l'API publique AudioIODevice)
    std::vector<juce::String> midiDevs = UserPreferences::loadMidiDevices();

  // Appliquer le type de device et initialiser le périphérique (ASIO...)
  if (audioType.isNotEmpty())
    deviceManager.setCurrentAudioDeviceType(audioType, true);

  // Appliquer la configuration complète (nom, sample rate, buffer size)
  AudioDeviceManager::AudioDeviceSetup setup;
  deviceManager.getAudioDeviceSetup(setup);
  if (audioDevice.isNotEmpty())
    setup.outputDeviceName = audioDevice;
  if (audioSampleRate > 0.0)
    setup.sampleRate = audioSampleRate;
  if (audioBufferSize > 0)
    setup.bufferSize = audioBufferSize;
  setAudioChannels(0, mopo::NUM_CHANNELS);
  deviceManager.setAudioDeviceSetup(setup, true);


  // Vérification et avertissement si la configuration appliquée diffère de celle du JSON
  {
    AudioDeviceManager::AudioDeviceSetup applied;
    deviceManager.getAudioDeviceSetup(applied);
    juce::String appliedType = deviceManager.getCurrentAudioDeviceType();
    bool mismatch = false;
    juce::String msg;
    if (audioType.isNotEmpty() && audioType != appliedType) {
      mismatch = true;
      msg << "Audio device type: requested '" << audioType << "', applied '" << appliedType << "'\n";
    }
    if (audioDevice.isNotEmpty() && audioDevice != applied.outputDeviceName) {
      mismatch = true;
      msg << "Audio device: requested '" << audioDevice << "', applied '" << applied.outputDeviceName << "'\n";
    }
    if (audioSampleRate > 0.0 && std::abs(audioSampleRate - applied.sampleRate) > 1.0) {
      mismatch = true;
      msg << "Sample rate: requested '" << audioSampleRate << "', applied '" << applied.sampleRate << "'\n";
    }
    if (audioBufferSize > 0 && audioBufferSize != applied.bufferSize) {
      mismatch = true;
      msg << "Buffer size: requested '" << audioBufferSize << "', applied '" << applied.bufferSize << "'\n";
    }
    if (mismatch) {
      juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
        "Audio configuration warning",
        "The requested audio configuration could not be fully applied.\n" + msg + "\nCheck your device availability and settings.");
    }
  }

  // Appliquer les ports actifs si possible
  if (auto* dev = deviceManager.getCurrentAudioDevice()) {
    auto ins = dev->getInputChannelNames();
    auto outs = dev->getOutputChannelNames();
    juce::BigInteger activeIn, activeOut;
    for (int i = 0; i < ins.size(); ++i)
      if (std::find(audioInputPorts.begin(), audioInputPorts.end(), ins[i]) != audioInputPorts.end())
        activeIn.setBit(i);
    for (int i = 0; i < outs.size(); ++i)
      if (std::find(audioOutputPorts.begin(), audioOutputPorts.end(), outs[i]) != audioOutputPorts.end())
        activeOut.setBit(i);
      // dev->setActiveInputChannels(activeIn);
      // dev->setActiveOutputChannels(activeOut);
  }

  // Appliquer les devices MIDI actifs
  if (!midiDevs.empty()) {
    for (const auto& dev : midiDevs) {
      deviceManager.setMidiInputEnabled(dev, true);
    }
  } else {
    const StringArray all_midi_ins(MidiInput::getDevices());
    std::vector<juce::String> enabledMidi;
    for (int i = 0; i < all_midi_ins.size(); ++i) {
      deviceManager.setMidiInputEnabled(all_midi_ins[i], true);
      enabledMidi.push_back(all_midi_ins[i]);
    }
    UserPreferences::saveMidiDevices(enabledMidi);
  }


  // UI scale et taille : charger les valeurs mais n'appliquer la taille qu'en toute fin
  int w = 0, h = 0;
  float ui_scale = 1.0f;
  if (use_gui) {
    UserPreferences::loadUiSize(w, h);
    ui_scale = UserPreferences::loadUiScale();
  }
  // ...suite du constructeur...
  computer_keyboard_ = std::make_unique<HelmComputerKeyboard>(&engine_, keyboard_state_.get());

  // (supprimé : initialisation redondante du device audio)

  if (use_gui) {
    setLookAndFeel(DefaultLookAndFeel::instance());
    addAndMakeVisible(gui_.get());
    gui_->setOutputMemory(getOutputMemory());
  // Ne pas écraser la taille restaurée du JSON

    setWantsKeyboardFocus(true);
    addKeyListener(computer_keyboard_.get());
    setOpaque(true);
    // Appliquer la taille restaurée en toute fin
    if (w > 0 && h > 0)
      setSize(w, h);
    if (ui_scale > 0.1f && ui_scale < 10.0f)
      setTransform(AffineTransform::scale(ui_scale));
  }

  // ...suite du constructeur...
  computer_keyboard_ = std::make_unique<HelmComputerKeyboard>(&engine_, keyboard_state_.get());
  // (supprimé : réinitialisation redondante du device audio)

  if (deviceManager.getCurrentAudioDevice() == nullptr) {
    const OwnedArray<AudioIODeviceType>& device_types = deviceManager.getAvailableDeviceTypes();

    for (AudioIODeviceType* device_type : device_types) {
      deviceManager.setCurrentAudioDeviceType(device_type->getTypeName(), true);
      if (deviceManager.getCurrentAudioDevice())
        break;
    }
  }

  const StringArray all_midi_ins(MidiInput::getDevices());


  // (déjà géré ci-dessus)

  deviceManager.addMidiInputCallback("", midi_manager_.get());
}


HelmEditor::~HelmEditor() {
  // --- Sauvegarde des préférences utilisateur ---
  // UI scale
  float scale = getTransform().isIdentity() ? 1.0f : getTransform().mat00;
  UserPreferences::saveUiScale(scale);
 
   // (plus de sauvegarde audio/MIDI à la fermeture)

  shutdownAudio();
  midi_manager_ = nullptr;
  computer_keyboard_ = nullptr;
  gui_ = nullptr;
  keyboard_state_ = nullptr;
}

void HelmEditor::prepareToPlay(int buffer_size, double sample_rate) {
  engine_.setSampleRate(sample_rate);
  engine_.setBufferSize(std::min(buffer_size, MAX_BUFFER_PROCESS));
  engine_.updateAllModulationSwitches();
  midi_manager_->setSampleRate(sample_rate);
  // Sauvegarde immédiate du buffer size
  UserPreferences::saveAudioBufferSize(buffer_size);
  // Sauvegarde immédiate des paramètres audio principaux
  juce::String audioType = deviceManager.getCurrentAudioDeviceType();
  UserPreferences::saveAudioDeviceType(audioType);
  if (auto* dev = deviceManager.getCurrentAudioDevice()) {
    UserPreferences::saveAudioDevice(dev->getName());
    double currentRate = dev->getCurrentSampleRate();
    double storedRate = UserPreferences::loadAudioSampleRate();
    if (std::abs(currentRate - storedRate) > 1.0) // tolérance 1 Hz
      UserPreferences::saveAudioSampleRate(currentRate);
    std::vector<juce::String> inPorts, outPorts;
    auto ins = dev->getInputChannelNames();
    auto outs = dev->getOutputChannelNames();
    auto activeIn = dev->getActiveInputChannels();
    auto activeOut = dev->getActiveOutputChannels();
    for (int i = 0; i < ins.size(); ++i) if (activeIn[i]) inPorts.push_back(ins[i]);
    for (int i = 0; i < outs.size(); ++i) if (activeOut[i]) outPorts.push_back(outs[i]);
    UserPreferences::saveAudioInputPorts(inPorts);
    UserPreferences::saveAudioOutputPorts(outPorts);
  }
}

void HelmEditor::getNextAudioBlock(const AudioSourceChannelInfo& buffer) {
  ScopedLock lock(getCriticalSection());

  int num_samples = buffer.buffer->getNumSamples();
  int synth_samples = std::min(num_samples, MAX_BUFFER_PROCESS);

  processControlChanges();
  processModulationChanges();
  MidiBuffer midi_messages;
  midi_manager_->removeNextBlockOfMessages(midi_messages, num_samples);
  processMidi(midi_messages);
  processKeyboardEvents(midi_messages, num_samples);

  for (int b = 0; b < num_samples; b += synth_samples) {
    int current_samples = std::min<int>(synth_samples, num_samples - b);

    processAudio(buffer.buffer, mopo::NUM_CHANNELS, current_samples, b);
  }
}

void HelmEditor::releaseResources() {
}

void HelmEditor::paint(Graphics& g) {
}

void HelmEditor::resized() {
  if (gui_)
    gui_->setBounds(getBounds());
  // Sauvegarde du zoom UI et de la taille à chaque redimensionnement
  float scale = getTransform().isIdentity() ? 1.0f : getTransform().mat00;
  UserPreferences::saveUiScale(scale);
  UserPreferences::saveUiSize(getWidth(), getHeight());
}

void HelmEditor::animate(bool animate) {
  if (gui_)
    gui_->animate(animate);
}


