#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class WavesScanner {
public:
    juce::String scanInstalledVST3();
    juce::String getDatabasePath() const;
private:
    juce::KnownPluginList knownPlugins;
};
