#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "../Core/DspEngine.h"
#include "WavesScanner.h"

class DPxADJUAudioProcessor final : public juce::AudioProcessor {
public:
    DPxADJUAudioProcessor();
    ~DPxADJUAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.5; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    void runAutoAnalysis();
    dpx::Metrics getMetrics() const;
    juce::String scanWaves();
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    dpx::DspEngine engine;
    std::atomic<float> meterRms{-120.0f}, meterPeak{-120.0f}, meterCrest{0.0f}, meterLufs{-120.0f};
    std::atomic<float> meterLow{0.0f}, meterMid{0.0f}, meterHigh{0.0f}, meterCorr{1.0f}, meterSibilance{0.0f};
    std::atomic<unsigned long long> meterSamples{0};
    WavesScanner scanner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DPxADJUAudioProcessor)
};
