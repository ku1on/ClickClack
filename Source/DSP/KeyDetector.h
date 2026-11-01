#pragma once

#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <unordered_map>

namespace essentia {
namespace standard {
class Algorithm;
}
}

namespace clickclack {

struct KeyDetectionResult {
    std::string rootNote = "";        // e.g. "C", "F#", "Eb"
    std::string scale = "";           // "major" or "minor"
    std::string camelot = "--";       // e.g. "8B", "5A"
    float confidence = 0.0f;          // 0.0 to 1.0
    bool isStable = false;
};

class KeyDetector {
public:
    KeyDetector();
    ~KeyDetector();

    void reset();

    /**
     * Analyzes an audio window at 44.1 kHz.
     * Expects 3 to 8 seconds of audio for reliable harmonic profile.
     */
    KeyDetectionResult process(const float* audio, size_t numSamples);

private:
    std::unique_ptr<essentia::standard::Algorithm> keyExtractor_;

    struct KeyHistoryItem {
        std::string rootNote;
        std::string scale;
        float confidence;
    };
    std::deque<KeyHistoryItem> history_;
    static constexpr size_t kMaxHistory = 14;
    static constexpr float kMinStrengthThreshold = 0.25f;

    std::string lockedRoot_ = "";
    std::string lockedScale_ = "";
    std::string lockedCamelot_ = "";
    bool isLocked_ = false;
    size_t consecutiveDisagreements_ = 0;

    void initializeAlgorithm();
};

} // namespace clickclack
