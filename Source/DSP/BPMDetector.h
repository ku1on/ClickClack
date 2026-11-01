#pragma once

#include <vector>
#include <deque>
#include <string>
#include <memory>

namespace essentia {
namespace standard {
class Algorithm;
}
}

namespace clickclack {

struct BPMDetectionResult {
    float bpm = 0.0f;
    float confidence = 0.0f;          // 0.0 to 1.0
    bool isStable = false;
    std::vector<float> candidates;    // candidate tempos (e.g. primary, half, double)
};

class BPMDetector {
public:
    BPMDetector();
    ~BPMDetector();

    void reset();

    /**
     * Analyzes an audio frame at 44.1 kHz.
     * Audio should ideally be 4 to 8 seconds of contiguous audio.
     */
    BPMDetectionResult process(const float* audio, size_t numSamples);

    /**
     * Resolves half-time / double-time ambiguities given a candidate tempo.
     */
    static float normalizeTempoRange(float rawBpm, const std::vector<float>& estimates);

private:
    std::unique_ptr<essentia::standard::Algorithm> rhythmExtractor_;

    // Stabilization history
    struct HistoryItem {
        float bpm;
        float confidence;
    };
    std::deque<HistoryItem> history_;
    static constexpr size_t kMaxHistory = 8;
    static constexpr float kMinConfidenceThreshold = 0.25f;

    void initializeAlgorithm();
};

} // namespace clickclack
