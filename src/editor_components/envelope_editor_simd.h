#pragma once

#include "../synthesis/helm2025_engine_simd.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_graphics/juce_graphics.h"

class EnvelopeEditorSimd : public juce::Component,
                          public juce::Timer {
public:
    EnvelopeEditorSimd(const juce::String& name)
        : envelopeName_(name) {
        
        // Configuration des sliders ADSR
        setupSlider(attackSlider_, "Attack", 0.001f, 10.0f, 0.01f);
        setupSlider(decaySlider_, "Decay", 0.001f, 10.0f, 0.1f);
        setupSlider(sustainSlider_, "Sustain", 0.0f, 1.0f, 0.7f);
        setupSlider(releaseSlider_, "Release", 0.001f, 10.0f, 0.2f);

        // Configuration de la courbe
        setupSlider(attackCurveSlider_, "Attack Curve", -1.0f, 1.0f, 0.0f);
        setupSlider(decayCurveSlider_, "Decay Curve", -1.0f, 1.0f, 0.0f);
        setupSlider(releaseCurveSlider_, "Release Curve", -1.0f, 1.0f, 0.0f);

        // Configuration des boutons de mode
        setupModeButton(linearMode_, "Linear");
        setupModeButton(exponentialMode_, "Exponential");
        setupModeButton(scurveMode_, "S-Curve");

        // Démarrage du timer pour le rafraîchissement
        startTimerHz(30);
    }

    ~EnvelopeEditorSimd() override {
        stopTimer();
    }

    void resized() override {
        auto area = getLocalBounds();
        
        // Zone de visualisation de l'enveloppe
        visualizer_ = area.removeFromTop(area.getHeight() * 0.6);
        
        // Zone des contrôles
        auto controls = area;
        auto sliderWidth = controls.getWidth() / 4;
        
        // Placement des sliders ADSR
        attackSlider_.setBounds(controls.removeFromLeft(sliderWidth));
        decaySlider_.setBounds(controls.removeFromLeft(sliderWidth));
        sustainSlider_.setBounds(controls.removeFromLeft(sliderWidth));
        releaseSlider_.setBounds(controls.removeFromLeft(sliderWidth));
        
        // Placement des contrôles de courbe
        area = area.reduced(5);
        auto curveControls = area.removeFromBottom(60);
        auto curveWidth = curveControls.getWidth() / 3;
        
        attackCurveSlider_.setBounds(curveControls.removeFromLeft(curveWidth));
        decayCurveSlider_.setBounds(curveControls.removeFromLeft(curveWidth));
        releaseCurveSlider_.setBounds(curveControls);
        
        // Placement des boutons de mode
        auto modeButtons = area.removeFromBottom(30);
        auto buttonWidth = modeButtons.getWidth() / 3;
        
        linearMode_.setBounds(modeButtons.removeFromLeft(buttonWidth));
        exponentialMode_.setBounds(modeButtons.removeFromLeft(buttonWidth));
        scurveMode_.setBounds(modeButtons);
    }

    void paint(juce::Graphics& g) override {
        // Dessine le fond
        g.fillAll(juce::Colours::black);
        
        // Dessine la grille
        drawGrid(g);
        
        // Dessine l'enveloppe
        drawEnvelope(g);
        
        // Dessine les marqueurs de temps
        drawTimeMarkers(g);
    }

    void timerCallback() override {
        // Mise à jour de la visualisation en temps réel
        repaint(visualizer_);
    }

    SimdEnvelope::EnvelopeParameters getParameters() const {
        return {
            .attack_time = static_cast<float>(attackSlider_.getValue()),
            .decay_time = static_cast<float>(decaySlider_.getValue()),
            .sustain_level = static_cast<float>(sustainSlider_.getValue()),
            .release_time = static_cast<float>(releaseSlider_.getValue()),
            .attack_curve = static_cast<float>(attackCurveSlider_.getValue()),
            .decay_curve = static_cast<float>(decayCurveSlider_.getValue()),
            .release_curve = static_cast<float>(releaseCurveSlider_.getValue()),
            .mode = getCurrentMode()
        };
    }

private:
    void setupSlider(juce::Slider& slider, const juce::String& name,
                    float min, float max, float defaultValue) {
        addAndMakeVisible(slider);
        slider.setRange(min, max);
        slider.setValue(defaultValue);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        slider.setName(name);
        slider.addListener(this);
    }

    void setupModeButton(juce::TextButton& button, const juce::String& name) {
        addAndMakeVisible(button);
        button.setButtonText(name);
        button.setClickingTogglesState(true);
        button.addListener(this);
    }

    void drawGrid(juce::Graphics& g) {
        g.setColour(juce::Colours::darkgrey);
        
        // Lignes horizontales
        for (int i = 0; i <= 10; ++i) {
            float y = visualizer_.getY() + (i * visualizer_.getHeight() / 10.0f);
            g.drawHorizontalLine(static_cast<int>(y), 
                               static_cast<float>(visualizer_.getX()),
                               static_cast<float>(visualizer_.getRight()));
        }
        
        // Lignes verticales
        for (int i = 0; i <= 10; ++i) {
            float x = visualizer_.getX() + (i * visualizer_.getWidth() / 10.0f);
            g.drawVerticalLine(static_cast<int>(x),
                             static_cast<float>(visualizer_.getY()),
                             static_cast<float>(visualizer_.getBottom()));
        }
    }

    void drawEnvelope(juce::Graphics& g) {
        auto params = getParameters();
        
        // Configuration du tracé
        g.setColour(juce::Colours::cyan);
        g.strokePath(generateEnvelopePath(params), 
                    juce::PathStrokeType(2.0f));
    }

    void drawTimeMarkers(juce::Graphics& g) {
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        
        auto params = getParameters();
        float totalTime = params.attack_time + params.decay_time + params.release_time;
        
        // Marqueurs de temps pour chaque phase
        float x = visualizer_.getX() + (params.attack_time / totalTime) * visualizer_.getWidth();
        g.drawText("A", x - 10, visualizer_.getBottom() + 5, 20, 20, juce::Justification::centred);
        
        x = visualizer_.getX() + ((params.attack_time + params.decay_time) / totalTime) * visualizer_.getWidth();
        g.drawText("D", x - 10, visualizer_.getBottom() + 5, 20, 20, juce::Justification::centred);
        
        g.drawText("S", visualizer_.getX() + visualizer_.getWidth() / 2 - 10,
                  visualizer_.getY() - 25, 20, 20, juce::Justification::centred);
        
        x = visualizer_.getRight() - (params.release_time / totalTime) * visualizer_.getWidth();
        g.drawText("R", x - 10, visualizer_.getBottom() + 5, 20, 20, juce::Justification::centred);
    }

    juce::Path generateEnvelopePath(const SimdEnvelope::EnvelopeParameters& params) {
        juce::Path path;
        
        float totalTime = params.attack_time + params.decay_time + params.release_time;
        float maxHeight = visualizer_.getHeight();
        float width = static_cast<float>(visualizer_.getWidth());
        
        // Point de départ
        path.startNewSubPath(visualizer_.getX(), visualizer_.getBottom());
        
        // Phase d'attaque
        float attackX = visualizer_.getX() + (params.attack_time / totalTime) * width;
        float attackY = visualizer_.getBottom() - maxHeight;
        path.quadraticTo(attackX * (1.0f - std::abs(params.attack_curve)),
                        visualizer_.getBottom() - maxHeight * (params.attack_curve > 0 ? 
                            1.0f - std::abs(params.attack_curve) : 1.0f),
                        attackX, attackY);
        
        // Phase de decay
        float decayX = attackX + (params.decay_time / totalTime) * width;
        float sustainY = visualizer_.getBottom() - (maxHeight * params.sustain_level);
        path.quadraticTo(attackX + (decayX - attackX) * (1.0f - std::abs(params.decay_curve)),
                        attackY - (attackY - sustainY) * (params.decay_curve > 0 ?
                            std::abs(params.decay_curve) : 0.0f),
                        decayX, sustainY);
        
        // Phase de sustain
        path.lineTo(visualizer_.getRight() - (params.release_time / totalTime) * width,
                   sustainY);
        
        // Phase de release
        path.quadraticTo(visualizer_.getRight() * (1.0f - std::abs(params.release_curve)),
                        sustainY + (visualizer_.getBottom() - sustainY) * 
                            (params.release_curve > 0 ? std::abs(params.release_curve) : 0.0f),
                        visualizer_.getRight(), visualizer_.getBottom());
        
        return path;
    }

    SimdEnvelope::Mode getCurrentMode() const {
        if (linearMode_.getToggleState())
            return SimdEnvelope::Mode::Linear;
        if (exponentialMode_.getToggleState())
            return SimdEnvelope::Mode::Exponential;
        return SimdEnvelope::Mode::SCurve;
    }

    juce::String envelopeName_;
    juce::Rectangle<int> visualizer_;

    // Contrôles ADSR
    juce::Slider attackSlider_;
    juce::Slider decaySlider_;
    juce::Slider sustainSlider_;
    juce::Slider releaseSlider_;

    // Contrôles de courbe
    juce::Slider attackCurveSlider_;
    juce::Slider decayCurveSlider_;
    juce::Slider releaseCurveSlider_;

    // Boutons de mode
    juce::TextButton linearMode_{"Linear"};
    juce::TextButton exponentialMode_{"Exponential"};
    juce::TextButton scurveMode_{"S-Curve"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeEditorSimd)
};

