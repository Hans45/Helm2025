#pragma once

#include "../synthesis/helm2025_engine_simd.h"
#include "juce_audio_processors/juce_audio_processors.h"

class HelmPluginSimd : public juce::AudioProcessor {
public:
    HelmPluginSimd()
        : AudioProcessor(BusesProperties()
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
        , engine_(std::make_unique<mopo::HelmEngineSimd>()) {
        
        setupParameters();
    }

    ~HelmPluginSimd() override = default;

    const juce::String getName() const override { return "Helm SIMD"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool isMidiEffect() const override { return false; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        engine_->setSampleRate(static_cast<int>(sampleRate));
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, 
                     juce::MidiBuffer& midiMessages) override {
        // Traitement des événements MIDI
        for (const auto metadata : midiMessages) {
            const auto message = metadata.getMessage();
            if (message.isNoteOn()) {
                engine_->pushMidiEvent(mopo::MidiNoteOn{
                    static_cast<uint8_t>(message.getNoteNumber()),
                    static_cast<uint8_t>(message.getVelocity()),
                    static_cast<uint8_t>(message.getChannel())
                });
            }
            else if (message.isNoteOff()) {
                engine_->pushMidiEvent(mopo::MidiNoteOff{
                    static_cast<uint8_t>(message.getNoteNumber()),
                    static_cast<uint8_t>(message.getChannel())
                });
            }
        }

        // Traitement audio
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        engine_->processBlock(left, right, buffer.getNumSamples());
    }

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override {
        // Sérialisation des paramètres
        juce::XmlElement xml("HelmSIMD");
        for (auto* param : getParameters()) {
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(param)) {
                xml.setAttribute(p->getParameterID(), p->get());
            }
        }
        copyXmlToBinary(xml, destData);
    }

    void setStateInformation(const void* data, int sizeInBytes) override {
        // Désérialisation des paramètres
        std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
        if (xml && xml->hasTagName("HelmSIMD")) {
            for (auto* param : getParameters()) {
                if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(param)) {
                    if (xml->hasAttribute(p->getParameterID())) {
                        p->setValueNotifyingHost(xml->getDoubleAttribute(p->getParameterID()));
                    }
                }
            }
        }
    }

private:
    void setupParameters() {
        // Paramètres des oscillateurs
        addParameter(new juce::AudioParameterFloat("osc1_freq", "Oscillator 1 Frequency",
            20.0f, 20000.0f, 440.0f));
        addParameter(new juce::AudioParameterFloat("osc1_amp", "Oscillator 1 Amplitude",
            0.0f, 1.0f, 0.5f));
        addParameter(new juce::AudioParameterInt("osc1_wave", "Oscillator 1 Waveform",
            0, 3, 0));

        // Paramètres des filtres
        addParameter(new juce::AudioParameterFloat("filter1_freq", "Filter 1 Frequency",
            20.0f, 20000.0f, 2000.0f));
        addParameter(new juce::AudioParameterFloat("filter1_res", "Filter 1 Resonance",
            0.0f, 1.0f, 0.707f));
        addParameter(new juce::AudioParameterInt("filter1_type", "Filter 1 Type",
            0, 3, 0));

        // Paramètres des enveloppes
        addParameter(new juce::AudioParameterFloat("amp_attack", "Amplitude Attack",
            0.001f, 10.0f, 0.01f));
        addParameter(new juce::AudioParameterFloat("amp_decay", "Amplitude Decay",
            0.001f, 10.0f, 0.1f));
        addParameter(new juce::AudioParameterFloat("amp_sustain", "Amplitude Sustain",
            0.0f, 1.0f, 0.7f));
        addParameter(new juce::AudioParameterFloat("amp_release", "Amplitude Release",
            0.001f, 10.0f, 0.2f));
    }

    std::unique_ptr<mopo::HelmEngineSimd> engine_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelmPluginSimd)
};

// Création du plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new HelmPluginSimd();
}

