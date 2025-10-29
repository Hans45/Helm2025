#include <vector>
#include <JuceHeader.h>
#include "user_preferences.h"

namespace {
    const char* kPrefsFile = "user_prefs.json";
    juce::File getPrefsFile() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("helm2025").getChildFile(kPrefsFile);
    }
}


namespace UserPreferences {

int loadAudioBufferSize() {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return 0;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_buffer_size"))
            return static_cast<int>(obj->getProperty("audio_buffer_size"));
    }
    return 0;
}

void saveAudioBufferSize(int bufferSize) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("audio_buffer_size", bufferSize);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}
void saveUiSize(int width, int height) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("ui_width", width);
    obj->setProperty("ui_height", height);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}
bool loadUiSize(int& width, int& height) {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return false;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("ui_width") && obj->hasProperty("ui_height")) {
            width = static_cast<int>(obj->getProperty("ui_width"));
            height = static_cast<int>(obj->getProperty("ui_height"));
            return true;
        }
    }
    return false;
}

std::vector<juce::String> loadAudioInputPorts() {
    auto file = getPrefsFile();
    std::vector<juce::String> result;
    if (!file.existsAsFile()) return result;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_input_ports")) {
            auto arr = obj->getProperty("audio_input_ports");
            if (arr.isArray()) {
                for (auto& v : *arr.getArray())
                    result.push_back(v.toString());
            }
        }
    }
    return result;
}

void saveAudioInputPorts(const std::vector<juce::String>& ports) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    juce::Array<juce::var> arr;
    for (const auto& name : ports) arr.add(name);
    obj->setProperty("audio_input_ports", arr);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}

std::vector<juce::String> loadAudioOutputPorts() {
    auto file = getPrefsFile();
    std::vector<juce::String> result;
    if (!file.existsAsFile()) return result;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_output_ports")) {
            auto arr = obj->getProperty("audio_output_ports");
            if (arr.isArray()) {
                for (auto& v : *arr.getArray())
                    result.push_back(v.toString());
            }
        }
    }
    return result;
}

void saveAudioOutputPorts(const std::vector<juce::String>& ports) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    juce::Array<juce::var> arr;
    for (const auto& name : ports) arr.add(name);
    obj->setProperty("audio_output_ports", arr);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}
// (Déjà défini plus haut, suppression de la redondance)
float loadUiScale() {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return 1.0f;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("ui_scale"))
            return static_cast<float>(obj->getProperty("ui_scale"));
    }
    return 1.0f;
}
void saveUiScale(float scale) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("ui_scale", scale);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}

juce::String loadAudioDevice() {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return {};
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_device"))
            return obj->getProperty("audio_device").toString();
    }
    return {};
}
void saveAudioDevice(const juce::String& deviceName) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("audio_device", deviceName);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}

juce::String loadAudioDeviceType() {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return {};
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_device_type"))
            return obj->getProperty("audio_device_type").toString();
    }
    return {};
}
void saveAudioDeviceType(const juce::String& type) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("audio_device_type", type);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}

std::vector<juce::String> loadMidiDevices() {
    auto file = getPrefsFile();
    std::vector<juce::String> result;
    if (!file.existsAsFile()) return result;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("midi_devices")) {
            auto arr = obj->getProperty("midi_devices");
            if (arr.isArray()) {
                for (auto& v : *arr.getArray())
                    result.push_back(v.toString());
            }
        }
    }
    return result;
}

void saveMidiDevices(const std::vector<juce::String>& deviceNames) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    juce::Array<juce::var> arr;
    for (const auto& name : deviceNames) arr.add(name);
    obj->setProperty("midi_devices", arr);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}



double UserPreferences::loadAudioSampleRate() {
    auto file = getPrefsFile();
    if (!file.existsAsFile()) return 0.0;
    juce::var data = juce::JSON::parse(file);
    if (auto* obj = data.getDynamicObject()) {
        if (obj->hasProperty("audio_sample_rate"))
            return static_cast<double>(obj->getProperty("audio_sample_rate"));
    }
    return 0.0;
}

void UserPreferences::saveAudioSampleRate(double rate) {
    auto file = getPrefsFile();
    juce::var data;
    if (file.existsAsFile()) data = juce::JSON::parse(file);
    if (!data.isObject()) data = new juce::DynamicObject();
    auto* obj = data.getDynamicObject();
    obj->setProperty("audio_sample_rate", rate);
    file.create();
    file.replaceWithText(juce::JSON::toString(data));
}

} // namespace UserPreferences
