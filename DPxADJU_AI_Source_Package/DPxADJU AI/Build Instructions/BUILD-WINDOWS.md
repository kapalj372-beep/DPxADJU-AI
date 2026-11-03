# Windows 10/11 x64 build

## Required tools
- Visual Studio 2022 with **Desktop development with C++** workload and Windows 10/11 SDK.
- CMake 3.22 or newer.
- Git (optional if JUCE is downloaded as a ZIP).
- JUCE 7 or JUCE 8 source tree. This project does not vendor JUCE; point `JUCE_SOURCE_DIR` at the extracted JUCE directory.
- Optional: NSIS if you want a self-extracting Windows installer executable.

## Build
Open **x64 Native Tools Command Prompt for VS 2022** or PowerShell with the VS C++ environment available, then run:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DDPX_BUILD_JUCE=ON `
  -DDPX_BUILD_TESTS=ON `
  -DJUCE_SOURCE_DIR="C:/SDK/JUCE"
cmake --build build --config Release --target DPxADJU_AI_VST3 DPxADJU_AI_Standalone
ctest --test-dir build -C Release --output-on-failure
```

JUCE output artefacts are normally placed under `build/DPxADJU_AI_artefacts/Release/` (VST3 and Standalone subfolders). The post-build step also copies the Standalone executable to `DPxADJU AI Studio.exe` beside the generated Standalone binary.

## FL Studio
1. Copy the generated `.vst3` bundle to `%CommonProgramFiles%/VST3/` (or install it there).
2. Open FL Studio 2025, run **Plugin Manager > Find installed plugins**.
3. Add `DPxADJU AI` to a Mixer Insert and verify audio, state recall and automation.
4. Do not run the plug-in scanner while a DAW is loading plug-ins.

## Important implementation status
The current build provides the in-process DSP core, VST3/Standalone JUCE adapter source, parameter/state plumbing, a basic editor, and VST3 plug-in description scanning. The scanner only catalogs plug-ins; it does not host or route Waves plug-ins. Reference-file matching, source separation, two-stem vocal balancing, batch mastering, MP3/FLAC export, true-peak oversampling and a trained ML model are not implemented in this milestone. The displayed LUFS is a rough RMS-derived estimate, not standards-compliant integrated LUFS.
