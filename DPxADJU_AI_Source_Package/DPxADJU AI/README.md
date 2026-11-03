# DPxADJU AI Mastering & Vocal Mixer

An early engineering milestone for a Windows JUCE VST3/Standalone audio processor.
The C++17 DSP core is implemented and tested; this is not yet a finished commercial
mastering suite and does not yet include a Windows plugin binary.

## Verified DSP build

```sh
cmake -S . -B build -DDPX_BUILD_TESTS=ON -DDPX_BUILD_JUCE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

Verified on 2026-11-03 with GCC Release, Clang Release, and GCC Debug + ASan/UBSan.
See `Build Evidence/Linux/BUILD-REPORT.txt`.

## Windows JUCE build

See `Build Instructions/BUILD-WINDOWS.md`. JUCE 7/8 and Visual Studio 2022 are external
requirements. This project does not bundle JUCE or any Waves binary.

## Accuracy and limitations

Automatic settings are DSP heuristics, not a trained ML model. Loudness is currently an
RMS-derived estimate, and the limiter is a sample-peak clamp, not a true-peak limiter.
The VST3 scanner catalogs metadata only; Waves hosting and VST2 scanning are not implemented.
See `Documentation/FEATURE-STATUS.md` and `Documentation/KNOWN-LIMITATIONS.md`.
