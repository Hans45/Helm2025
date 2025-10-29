#pragma once
#include <JuceHeader.h>
#include <vector>

namespace UserPreferences {
    int loadAudioBufferSize();
    void saveAudioBufferSize(int bufferSize);
    float loadUiScale();
    void saveUiScale(float scale);

    juce::String loadAudioDevice();
    void saveAudioDevice(const juce::String& deviceName);

    juce::String loadAudioDeviceType();
    void saveAudioDeviceType(const juce::String& type);

    std::vector<juce::String> loadMidiDevices();
    void saveMidiDevices(const std::vector<juce::String>& deviceNames);

    std::vector<juce::String> loadAudioInputPorts();
    void saveAudioInputPorts(const std::vector<juce::String>& ports);
    std::vector<juce::String> loadAudioOutputPorts();
    void saveAudioOutputPorts(const std::vector<juce::String>& ports);

    double loadAudioSampleRate();
    void saveAudioSampleRate(double rate);

    void saveUiSize(int width, int height);
    bool loadUiSize(int& width, int& height);
}