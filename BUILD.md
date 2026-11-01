# ClickClack — Build & Architecture Documentation

This document describes the compilation, dependencies, real-time audio architecture, and testing procedures for **ClickClack** on macOS Apple Silicon (ARM64).

---

## 1. Prerequisites

ClickClack is compiled natively for **Apple Silicon (ARM64)** on macOS Sonoma (14+) or later.

Required tools:
- **Xcode Command Line Tools** / `clang++` supporting C++17 or C++20.
- **CMake** 3.22+.
- **Homebrew** (located at `/opt/homebrew`).

Required dependencies (installed via Homebrew):
```bash
brew install eigen fftw libsamplerate libyaml taglib
```

---

## 2. Dependencies Architecture

1. **JUCE 8.0.4**
   - Vendored locally in `deps/JUCE` to guarantee 100% offline, self-contained builds without external network calls.
   - Provides audio processor lifecycle, audio format hosting (VST3, AU, Standalone), parameter state management (APVTS), and custom graphics.

2. **Essentia (Music Information Retrieval & DSP)**
   - Built as an optimized static library (`libessentia.a`) for ARM64 using Apple Accelerate framework and FFTW.
   - Core algorithms used:
     - `RhythmExtractor2013` (multifeature beat tracking & candidate estimation).
     - `KeyExtractor` (HPCP harmonic pitch class profile extraction & polyphonic key correlation).
     - `libsamplerate` (ultra-clean sinc sample rate conversion from any host rate to 44.1kHz).

---

## 3. Real-Time Processing Architecture

ClickClack adheres strictly to pro-audio real-time safety constraints:

```text
Ableton Live Audio Callback (processBlock)
                │
                ▼ (Wait-Free Mono Downmix & Peak/RMS Calculation)
     LockFreeAudioFifo (524,288 float capacity)
                │
                ▼ (Read without blocking Audio Thread)
    AudioBufferManager (44.1 kHz Resampling & Circular Sliding Window)
                │
                ▼ (Sliding Window: 6s Hop: 0.5s)
    AnalysisWorker Thread (High Priority Background juce::Thread)
         ├── BPMDetector (Essentia RhythmExtractor2013)
         ├── KeyDetector (Essentia KeyExtractor & HPCP)
         └── ConfidenceEstimator (State Machine & History Consensus)
                │
                ▼ (Mutex-protected atomic snapshot)
    ClickClackAudioProcessorEditor (Brutalist GUI at 30 Hz)
```

### Safety Guarantees in `processBlock`:
- **Zero dynamic memory allocations** (`new`, `malloc`, `std::vector::resize`).
- **Zero blocking mutex locks** (SPSC lock-free FIFO with `std::atomic` acquire/release semantics).
- **Zero complex FFT or MIR operations on the audio thread**.
- **Transparent audio pass-through** with zero latency introduction.

---

## 4. Building ClickClack

To compile the entire plugin suite (VST3, AU, and Standalone):

```bash
./Scripts/build.sh
```

Or manually with CMake:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64"
cmake --build . --config Release -j8
```

Compiled bundles will be generated in:
`build/ClickClack_artefacts/Release/`
- `VST3/ClickClack.vst3`
- `AU/ClickClack.component`
- `Standalone/ClickClack.app`

---

## 5. Installing into macOS & Ableton Live

Run the installation script:
```bash
./Scripts/install.sh
```

Standard destinations:
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/ClickClack.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/ClickClack.component`

Then in **Ableton Live 11/12**:
1. Open Preferences -> **Plug-Ins**.
2. Ensure **Use VST3 Plug-In System Folders** and **Use Audio Units** are turned ON.
3. Click **Rescan** (or hold `Option` while clicking Rescan for a deep re-index).
4. Look for **ClickClack** under `Audio Units -> ClickClack Audio` or `VST3 -> ClickClack Audio`.

---

## 6. Running Unit & Integration Tests

Run the test suite:
```bash
clang++ -std=c++17 -I./Source Tests/CamelotTest.cpp Source/DSP/CamelotMapper.cpp -o /tmp/camelot_test && /tmp/camelot_test
clang++ -std=c++17 -I./Source -I/opt/homebrew/Cellar/libsamplerate/0.2.2/include Tests/DSPTest.cpp Source/Audio/AudioBufferManager.cpp Source/Analysis/ConfidenceEstimator.cpp -L/opt/homebrew/Cellar/libsamplerate/0.2.2/lib -lsamplerate -o /tmp/dsp_test && /tmp/dsp_test
```
