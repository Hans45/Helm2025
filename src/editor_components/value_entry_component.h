#pragma once

#include <JuceHeader.h>

class value_entry_component : public juce::Component,
                              private juce::TextEditor::Listener,
                              private juce::KeyListener
{
public:
    value_entry_component(juce::String initialValue,
                        std::function<void(juce::String)> onSubmit);
    
    ~value_entry_component() override = default;

    void resized() override;

    void setCallOutBox(juce::CallOutBox* boxToClose);

private:
    juce::TextEditor editor;
    std::function<void(juce::String)> submitCallback;
    juce::CallOutBox* callOutBox = nullptr;
    
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(value_entry_component)
};
