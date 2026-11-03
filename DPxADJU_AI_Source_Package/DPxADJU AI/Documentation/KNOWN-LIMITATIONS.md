# Known limitations — milestone 0.1

- This is an early development build, not release-ready mastering software.
- The JUCE wrapper has not been compiled because JUCE and the Windows toolchain are not installed in the available Linux build environment.
- No Windows VST3, Standalone EXE or installer binary is present.
- Waves VST3 scanning is a catalog operation only. No third-party plug-in is instantiated, wrapped, or routed in the audio graph.
- VST2 scanning is intentionally absent. JUCE's current supported plug-in hosting workflow is VST3; legacy VST2 hosting has separate SDK/licensing constraints.
- Artist reference import, audio feature matching and artist-specific presets are not implemented.
- No vocal/instrument stem separation or two-stem automatic balancing is implemented.
- The DSP engine is a compact starter chain. It does not yet contain multiband compression, oversampled true-peak limiting, convolution reverb, high-quality noise reduction, or a full de-esser detector.
- LUFS shown by the engine is an RMS-derived estimate, not EBU R128/ITU-R BS.1770 integrated loudness. Do not use it as a delivery compliance measurement.
- Export, batch processing and preset-file loading are not implemented in the UI.
- Plugin scan is currently synchronous and should be moved to a worker thread before production release.
- The editor's reference and balance pages explicitly disclose that their processing is not active.
