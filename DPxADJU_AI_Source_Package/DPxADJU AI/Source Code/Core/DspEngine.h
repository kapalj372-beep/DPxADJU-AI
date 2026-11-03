#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dpx {
enum class Mode : uint8_t { vocal = 0, mastering = 1 };
enum class Style : uint8_t { pop, rnb, streaming, warm, dynamic, commercial };
struct Metrics {
    float rmsDb = -120.0f, peakDb = -120.0f, crestDb = 0.0f;
    float lowEnergy = 0.0f, midEnergy = 0.0f, highEnergy = 0.0f;
    float stereoCorrelation = 1.0f, sibilance = 0.0f;
    float estimatedLufs = -120.0f;
    uint64_t samples = 0;
};
struct Settings {
    Mode mode = Mode::vocal;
    Style style = Style::pop;
    float inputGainDb = 0.0f, outputGainDb = 0.0f, dryWet = 1.0f;
    float lowCutHz = 75.0f, presenceDb = 1.5f, airDb = 0.5f;
    float thresholdDb = -18.0f, ratio = 2.5f, attackMs = 10.0f, releaseMs = 100.0f;
    float deEssAmount = 0.25f, saturation = 0.08f, reverb = 0.08f, delay = 0.0f;
    float stereoWidth = 1.0f, limiterCeilingDb = -1.0f;
    bool bypass = false;
};
struct AnalysisResult { Metrics metrics; Settings suggested; bool valid = false; };

class DspEngine {
public:
    void prepare(double sampleRate, std::size_t maxBlockSize);
    void reset() noexcept;
    void setSettings(const Settings& s) noexcept { settings_ = s; }
    const Settings& settings() const noexcept { return settings_; }
    void process(float* left, float* right, std::size_t frames) noexcept;
    void analyse(const float* left, const float* right, std::size_t frames) noexcept;
    AnalysisResult makeSuggestion(Mode mode, Style style) const noexcept;
    AnalysisResult makeSuggestion(Mode mode, Style style, const Metrics& snapshot) const noexcept;
    const Metrics& metrics() const noexcept { return metrics_; }
private:
    struct Biquad { float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0; float tick(float x) noexcept; void reset() noexcept { z1=z2=0; } };
    void updateFilters() noexcept;
    static float dbToGain(float db) noexcept;
    static float gainToDb(float gain) noexcept;
    double sr_ = 48000.0;
    float cachedLowCut_ = -1.0f, cachedPresence_ = 999.0f, cachedAir_ = 999.0f;
    Settings settings_{};
    Metrics metrics_{};
    Biquad lowL_, lowR_, presenceL_, presenceR_, airL_, airR_;
    float compEnvL_=0, compEnvR_=0, deEssEnvL_=0, deEssEnvR_=0;
    float delayStateL_=0, delayStateR_=0, reverbStateL_=0, reverbStateR_=0;
    std::vector<float> delayBufferL_, delayBufferR_;
    std::size_t delayWrite_=0, delaySamples_=1;
    double sumSq_=0, sumLeft_=0, sumRight_=0, sumCross_=0;
    float peak_=0, low_=0, mid_=0, high_=0, sib_=0;
};
}
