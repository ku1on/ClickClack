# CLICKCLACK

> **Professional Real-Time BPM & Musical Key Detector for macOS Apple Silicon (ARM64)**  
> Engineered specifically for **Ableton Live 11 & 12**, Audio Units, and Standalone.

---

## Overview

**ClickClack** is a zero-latency audio plugin designed to detect tempo (BPM) and musical key (Key & Camelot notation) in real time directly inside your DAW.

Powered by MTG's industry-standard **Essentia MIR** framework combined with **JUCE 8**, ClickClack operates entirely locally with no network calls, accounts, or telemetry.

```text
┌──────────────────────────────────────────────┐
│ CLICKCLACK                        REAL-TIME  │
│                                              │
│  BPM                 KEY          CAMELOT    │
│  124.5               C MINOR      5A         │
│  CONFIDENCE 94%      CONFIDENCE 89%          │
│                                              │
│  [●] SIGNAL ACTIVE         STATUS: LOCKED    │
│                                              │
│  [ RESET ]  [ COPY BPM ]  [ COPY KEY ]       │
│  [            COPY ALL (124.5 | C Min | 5A) ]│
└──────────────────────────────────────────────┘
```

---

## Features

- **Real-Time BPM Detection (Tenths Precision)**:
  - Powered by Essentia `RhythmExtractor2013`.
  - Half-time / double-time candidate normalization.
  - Multi-frame rolling median stabilization.
  - Dynamic tempo change detection with confidence decay.
- **Harmonic Key Detection (Camelot System)**:
  - Powered by Essentia `KeyExtractor` (HPCP Harmonic Pitch Class Profiles + polyphonic correlation).
  - Full circle of 24 keys (12 Major + 12 Minor) with enharmonic resolution (`C#` / `Db`, `F#` / `Gb`).
  - Standard Camelot notation mapping (`8B` = C Major, `5A` = C Minor, `8A` = A Minor, etc.).
- **Real-Time Safety & DSP Architecture**:
  - **Wait-Free Lock-Free FIFO**: Audio thread never allocates dynamic memory and never takes mutex locks.
  - **Sliding Analysis Window**: 6-second circular rolling analysis buffer with 0.5-second hop intervals.
  - **Native Sample Rate Conversion**: High-quality SINC resampling via `libsamplerate` to ensure 44.1 kHz input for Essentia algorithms regardless of host session sample rate (44.1k, 48k, 88.2k, 96k, 192k).
- **Strict Monochromatic Brutalist Design**:
  - High-contrast industrial palette (pure black `#0A0A0A`, off-white typography, surgical gray borders).
  - Apple SF Pro typographic hierarchy and monospaced technical figures.
  - Responsive visual states: `WAITING` (silence), `ANALYZING` (gathering frames), `LOCKED` (high confidence).
  - One-click clipboard actions: `RESET`, `COPY BPM`, `COPY KEY`, and `COPY ALL` (format: `124.5 BPM | C Minor | 5A`).

---

## Plugin Formats & Installation

ClickClack builds natively as **Mach-O 64-bit arm64** for macOS 12+ (Apple Silicon M1/M2/M3/M4):

| Format | Target Path | Ableton Live Compatibility |
| :--- | :--- | :--- |
| **VST3** | `~/Library/Audio/Plug-Ins/VST3/ClickClack.vst3` | Ableton Live 11 & 12 |
| **Audio Unit (AU)** | `~/Library/Audio/Plug-Ins/Components/ClickClack.component` | Ableton Live 11 & 12, Logic Pro |
| **Standalone App** | `/Applications/ClickClack.app` | Standalone audio testing |

### Quick Install

```bash
cd ClickClack
./Scripts/install.sh
```

---

## Ableton Live Workflow

1. Open **Ableton Live 11** or **Ableton Live 12**.
2. Navigate to **Plugins** in the browser sidebar (`Plug-Ins` -> `VST3` or `Audio Units`).
3. Drag **ClickClack** onto any audio track or the Master bus.
4. Press Play in Ableton.
5. ClickClack immediately tracks incoming audio:
   - Within 0.5 – 1.0 second: status transitions from `WAITING` to `ANALYZING`.
   - Within 3 – 5 seconds: tempo and key reach high confidence, badge locks to `LOCKED`.
6. Click `COPY ALL` to paste directly into Ableton track titles, clip notes, or DJ metadata.

---

## Build from Source

### Prerequisites
- macOS Apple Silicon (ARM64)
- Xcode Command Line Tools (`clang`, `make`)
- CMake 3.22+
- Homebrew dependencies: `libsamplerate`, `fftw`, `libyaml`, `taglib`, `eigen`, `ffmpeg`

```bash
# 1. Clone repository
git clone https://github.com/clickclack/clickclack.git
cd clickclack

# 2. Build plugin bundles
./Scripts/build.sh

# 3. Install to macOS audio directories
./Scripts/install.sh
```

---

## Project Structure

```text
ClickClack/
├── CMakeLists.txt                # Unified CMake build configuration
├── LICENSE                       # AGPLv3 + Commercial licensing notice
├── BUILD.md                      # Detailed technical build guide
├── README.md                     # Product documentation
├── Scripts/
│   ├── build.sh                  # One-click build script
│   └── install.sh                # Installation and AU cache refresh
├── Source/
│   ├── Analysis/                 # Worker thread and confidence state machine
│   │   ├── AnalysisEngine.cpp / .h
│   │   ├── AnalysisResult.h
│   │   ├── AnalysisWorker.cpp / .h
│   │   └── ConfidenceEstimator.cpp / .h
│   ├── Audio/                    # Audio input downmix, meters, and circular buffer
│   │   ├── AudioBufferManager.cpp / .h
│   │   └── AudioInputProcessor.cpp / .h
│   ├── DSP/                      # Essentia algorithm wrappers & Camelot wheel
│   │   ├── BPMDetector.cpp / .h
│   │   ├── KeyDetector.cpp / .h
│   │   └── CamelotMapper.cpp / .h
│   ├── UI/                       # Brutalist design system components
│   │   ├── Components/
│   │   │   ├── AudioSignalMeter.cpp / .h
│   │   │   ├── BrutalistButton.cpp / .h
│   │   │   ├── ConfidenceBar.cpp / .h
│   │   │   └── StatusBadge.cpp / .h
│   │   └── Theme/
│   │       └── BrutalistTheme.h
│   ├── Utils/
│   │   └── LockFreeFifo.h        # SPSC atomic lock-free ring buffer
│   ├── PluginProcessor.cpp / .h  # JUCE AudioProcessor lifecycle
│   └── PluginEditor.cpp / .h     # JUCE AudioProcessorEditor
├── Tests/
│   ├── CamelotTest.cpp           # Key & Camelot unit tests
│   └── DSPTest.cpp               # Audio FIFO & buffer manager tests
└── deps/
    ├── JUCE/                     # JUCE 8 framework
    └── essentia/                 # Essentia static library (ARM64) & headers
```

---

## Testing

Unit tests run independently and verify the DSP pipeline:

```bash
# Run Camelot circle and enharmonics verification
clang++ -std=c++17 Tests/CamelotTest.cpp Source/DSP/CamelotMapper.cpp -I Source -o /tmp/camelot_test && /tmp/camelot_test

# Run Lock-Free FIFO, Sliding Window, and Confidence Estimator verification
clang++ -std=c++17 Tests/DSPTest.cpp \
    Source/Audio/AudioBufferManager.cpp \
    Source/Analysis/ConfidenceEstimator.cpp \
    Source/DSP/CamelotMapper.cpp \
    -I Source \
    $(pkg-config --cflags --libs samplerate) \
    -o /tmp/dsp_test && /tmp/dsp_test
```

---

## License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPLv3)** in compliance with the MTG Essentia open-source license.
For proprietary commercial licensing options without AGPLv3 obligations, contact the Music Technology Group (MTG) at Universitat Pompeu Fabra (https://essentia.upf.edu/).
