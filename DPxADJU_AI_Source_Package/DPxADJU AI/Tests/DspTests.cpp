#include "../Source Code/Core/DspEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

namespace {
constexpr float pi = 3.14159265358979323846f;
void require(bool ok, const char* message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
std::vector<float> tone(float frequency, float amplitude, std::size_t count, float sr) {
    std::vector<float> x(count);
    for (std::size_t i = 0; i < count; ++i)
        x[i] = amplitude * std::sin(2.0f * pi * frequency * static_cast<float>(i) / sr);
    return x;
}
}

int main() {
    constexpr float sr = 48000.0f;
    constexpr std::size_t count = 48000;
    auto left = tone(220.0f, 0.8f, count, sr);
    auto right = left;
    dpx::DspEngine engine;
    engine.prepare(sr, 512);
    engine.analyse(left.data(), right.data(), count);
    const auto metrics = engine.metrics();
    require(metrics.samples == count, "analysis sample count");
    require(std::isfinite(metrics.rmsDb) && metrics.rmsDb < -1.0f && metrics.rmsDb > -20.0f, "RMS measurement");
    require(std::isfinite(metrics.peakDb) && metrics.peakDb <= 0.0f, "peak measurement");
    require(metrics.stereoCorrelation > 0.99f, "mono-compatible stereo correlation");

    auto vocal = engine.makeSuggestion(dpx::Mode::vocal, dpx::Style::pop);
    auto master = engine.makeSuggestion(dpx::Mode::mastering, dpx::Style::streaming);
    require(vocal.valid && master.valid, "automatic suggestions available after analysis");
    require(vocal.suggested.lowCutHz >= 20.0f && vocal.suggested.lowCutHz <= 300.0f, "vocal filter bounds");
    require(master.suggested.limiterCeilingDb <= 0.0f, "master ceiling is not positive");

    engine.setSettings(vocal.suggested);
    engine.process(left.data(), right.data(), count);
    for (std::size_t i = 0; i < count; ++i)
        require(std::isfinite(left[i]) && std::isfinite(right[i]), "finite processed samples");

    // Dry/wet at zero must preserve the original samples exactly.
    auto dryL = tone(997.0f, 0.6f, 2048, sr);
    auto dryR = dryL;
    const auto originalL = dryL;
    dpx::Settings drySettings;
    drySettings.dryWet = 0.0f;
    drySettings.delay = 0.5f;
    drySettings.reverb = 0.5f;
    engine.setSettings(drySettings);
    engine.process(dryL.data(), dryR.data(), dryL.size());
    require(dryL == originalL && dryR == originalL, "100 percent dry signal is unchanged");

    // Bypass must preserve the signal exactly.
    auto bypassL = tone(440.0f, 0.7f, 2048, sr);
    auto bypassR = bypassL;
    const auto bypassOriginal = bypassL;
    dpx::Settings bypassSettings;
    bypassSettings.bypass = true;
    engine.setSettings(bypassSettings);
    engine.process(bypassL.data(), bypassR.data(), bypassL.size());
    require(bypassL == bypassOriginal && bypassR == bypassOriginal, "bypass is bit-transparent");

    // Full-wet output must stay finite and respect the sample-peak ceiling.
    auto hotL = tone(1000.0f, 1.8f, 8192, sr);
    auto hotR = hotL;
    dpx::Settings limiterSettings;
    limiterSettings.mode = dpx::Mode::mastering;
    limiterSettings.inputGainDb = 12.0f;
    limiterSettings.limiterCeilingDb = -2.0f;
    limiterSettings.dryWet = 1.0f;
    engine.setSettings(limiterSettings);
    engine.process(hotL.data(), hotR.data(), hotL.size());
    const float ceiling = std::pow(10.0f, -2.0f / 20.0f) + 1.0e-5f;
    for (std::size_t i = 0; i < hotL.size(); ++i) {
        require(std::isfinite(hotL[i]) && std::isfinite(hotR[i]), "limiter finite output");
        require(std::abs(hotL[i]) <= ceiling && std::abs(hotR[i]) <= ceiling, "sample peak ceiling");
    }

    engine.reset();
    require(engine.metrics().samples == 0, "reset clears analysis counters");
    const auto invalid = engine.makeSuggestion(dpx::Mode::vocal, dpx::Style::pop);
    require(!invalid.valid, "no suggestion without analysed audio");
    std::cout << "PASS: analysis, metrics, vocal/master suggestions, processing, dry/wet, bypass, sample-peak ceiling, reset\n";
    return 0;
}
