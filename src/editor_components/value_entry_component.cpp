#include "value_entry_component.h"

value_entry_component::value_entry_component(juce::String initialValue,
                                            std::function<void(juce::String)> onSubmit)
    : submitCallback(std::move(onSubmit))
{
    addAndMakeVisible(editor);
    editor.setText(initialValue);
    editor.setSelectAllWhenFocused(true);
    editor.setFocusContainer(true);
    editor.addListener(this);
    editor.addKeyListener(this);
    setSize(150, 40);
}

bool value_entry_component::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (callOutBox) callOutBox->dismiss();
        return true;
    }
    return false;
}

void value_entry_component::resized()
{
    editor.setBounds(getLocalBounds());
}

void value_entry_component::setCallOutBox(juce::CallOutBox* boxToClose)
{
    callOutBox = boxToClose;
}

void value_entry_component::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (submitCallback)
        submitCallback(editor.getText());

    if (callOutBox != nullptr)
        callOutBox->dismiss();
}


