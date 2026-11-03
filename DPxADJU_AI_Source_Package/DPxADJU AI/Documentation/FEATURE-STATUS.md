# Feature status (2026-11-03)

## Implemented in the current milestone
- C++17 stereo DSP engine, built as a static library.
- Block-based audio analysis: RMS, sample peak, crest factor, coarse low/mid/high energy,
  stereo correlation, heuristic sibilance estimate, and approximate RMS-derived loudness.
- Heuristic parameter suggestions for vocal and mastering modes and several styles.
- DSP: high-pass, presence/air EQ, dynamics compression, simple de-ess heuristic,
  saturation, mid/side width, feedback delay, simple reverb, sample-peak clamp limiter,
  bypass and dry/wet.
- JUCE AudioProcessor/Editor source, APVTS parameter automation and state serialization.
- VST3-only plugin catalog scanner source. It scans metadata with JUCE's plugin scanner;
  it does not instantiate or host Waves effects.
- JUCE CMake target definitions for VST3 and Standalone.
- Automated DSP tests: GCC Release, Clang Release, and GCC ASan/UBSan Debug all pass (1/1).

## Not implemented / not verified
- Windows VST3 bundle and Standalone EXE: not built in this Linux environment.
- FL Studio 2025 load, automation, project recall and audio stability tests: not performed.
- Waves VST2/VST3 scanner coverage: VST2 not implemented; VST3 scanner source not validated
  against a Windows Waves installation. No Waves hosting/routing is implemented.
- Reference audio import/analysis and matching engine.
- Trained local ML inference model.
- Vocal/instrumental source separation and two-stem dynamic balancing.
- Standards-compliant integrated LUFS and oversampled true-peak measurement/limiting.
- Batch mastering and WAV/MP3/FLAC export workflow.
- Finished Windows installer.

## Important accuracy notes
- The current loudness number is a rough RMS-derived estimate, not EBU R128/ITU-R BS.1770
  integrated LUFS.
- The limiter is a sample-peak clamp, not an oversampled true-peak limiter.
- Automatic suggestions are deterministic DSP heuristics, not a trained AI model.
- Plugin scanning is not plugin hosting. A discovered Waves plugin is not automatically
  inserted into this processor's signal chain.
