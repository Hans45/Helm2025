#pragma once

#include "helm2025_plugin_simd.h"
#include "juce_gui_basics/juce_gui_basics.h"

class HelmEditorSimd : public juce::AudioProcessorEditor {
public:
    HelmEditorSimd(HelmPluginSimd& plugin)
        : AudioProcessorEditor(plugin)
        , plugin_(plugin) {
        
        setSize(800, 600);

        // Configuration des contrôles de l'interface
        setupOscillatorControls();
        setupFilterControls();
        setupEnvelopeControls();

        // Rafraîchissement de l'interface toutes les 50ms
        startTimerHz(20);
    }

    ~HelmEditorSimd() override = default;

    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void resized() override {
        auto area = getLocalBounds();
        
        // Section des oscillateurs
        oscillatorSection.setBounds(area.removeFromTop(200));
        
        // Section des filtres
        filterSection.setBounds(area.removeFromTop(200));
        
        // Section des enveloppes
        envelopeSection.setBounds(area);
    }

private:
    void setupOscillatorControls() {
        addAndMakeVisible(oscillatorSection);
        // Configuration des contrôles d'oscillateur...
    }

    void setupFilterControls() {
        addAndMakeVisible(filterSection);
        // Configuration des contrôles de filtre...
    }

    void setupEnvelopeControls() {
        addAndMakeVisible(ampEnvelope_);
        addAndMakeVisible(modEnvelope_);
        
        ampEnvelope_.onParametersChanged = [this](const SimdEnvelope::EnvelopeParameters& params) {
            // Mise à jour des paramètres d'enveloppe d'amplitude
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[6]))
                p->setValueNotifyingHost(params.attack_time);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[7]))
                p->setValueNotifyingHost(params.decay_time);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[8]))
                p->setValueNotifyingHost(params.sustain_level);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[9]))
                p->setValueNotifyingHost(params.release_time);
        };
        
        modEnvelope_.onParametersChanged = [this](const SimdEnvelope::EnvelopeParameters& params) {
            // Mise à jour des paramètres d'enveloppe de modulation
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[10]))
                p->setValueNotifyingHost(params.attack_time);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[11]))
                p->setValueNotifyingHost(params.decay_time);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[12]))
                p->setValueNotifyingHost(params.sustain_level);
            if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(plugin_.getParameters()[13]))
                p->setValueNotifyingHost(params.release_time);
        };
    }

    void timerCallback() {
        // Mise à jour des visualisations en temps réel
        repaint();
        ampEnvelope_.repaint();
        modEnvelope_.repaint();
    }

    HelmPluginSimd& plugin_;
    
    // Sections de l'interface
    juce::Component oscillatorSection;
    juce::Component filterSection;
    EnvelopeEditorSimd ampEnvelope_{"Amplitude"};
    EnvelopeEditorSimd modEnvelope_{"Modulation"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelmEditorSimd)
};

// Implémentation de la fonction createEditor() du plugin
inline juce::AudioProcessorEditor* HelmPluginSimd::createEditor() {
    return new HelmEditorSimd(*this);
}

