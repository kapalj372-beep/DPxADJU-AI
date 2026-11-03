#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class DPxADJUAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit DPxADJUAudioProcessorEditor(DPxADJUAudioProcessor&);
    ~DPxADJUAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    DPxADJUAudioProcessor& processor;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    juce::TextButton vocalButton{"AI VOCAL MIX"}, masterButton{"AI MASTERING"}, autoButton{"ANALYZE + APPLY"}, scanButton{"SCAN WAVES VST3"}, bypassButton{"BYPASS"};
    juce::Label title, status, meter;
    juce::Slider inputMeter, outputMeter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DPxADJUAudioProcessorEditor)
};
