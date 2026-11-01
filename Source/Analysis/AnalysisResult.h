#pragma once

#include <string>
#include <cstdint>

namespace clickclack {

enum class AnalysisState : uint8_t {
    Waiting = 0,
    Analyzing = 1,
    Locked = 2
};

inline const char* stateToString(AnalysisState state) {
    switch (state) {
        case AnalysisState::Waiting:   return "WAITING";
        case AnalysisState::Analyzing: return "ANALYZING";
        case AnalysisState::Locked:    return "LOCKED";
    }
    return "WAITING";
}

struct AnalysisResult {
    AnalysisState state = AnalysisState::Waiting;
    
    // BPM
    float bpm = 0.0f;
    float bpmConfidence = 0.0f;       // 0.0 to 1.0
    bool bpmLocked = false;
    
    // Key & Camelot
    std::string rootNote = "";        // e.g. "C", "F#", "Eb"
    std::string scale = "";           // "major" or "minor"
    std::string camelot = "--";       // e.g. "8B", "5A"
    float keyConfidence = 0.0f;       // 0.0 to 1.0
    bool keyLocked = false;

    // Real-time audio input
    float inputLevelRms = 0.0f;       // 0.0 to 1.0
    float inputLevelPeak = 0.0f;      // 0.0 to 1.0
    bool isAudioPresent = false;
    float accumulatedSeconds = 0.0f;
    float analysisProgress = 0.0f;    // 0.0 to 1.0 during warm-up

    // Manual overrides
    float manualBpm = 0.0f;
    bool hasManualBpm = false;
    std::string manualKey = "";
    bool hasManualKey = false;

    // Helper methods
    float effectiveBpm() const {
        return hasManualBpm ? manualBpm : bpm;
    }

    std::string effectiveKeyDisplay() const {
        if (hasManualKey && !manualKey.empty()) return manualKey;
        if (rootNote.empty()) return "---";
        std::string sc = (scale == "minor" ? "Minor" : "Major");
        return rootNote + " " + sc;
    }

    std::string effectiveCamelot() const {
        return camelot.empty() ? "--" : camelot;
    }
};

} // namespace clickclack
