#pragma once

#include "AnalysisResult.h"
#include <chrono>

namespace clickclack {

class ConfidenceEstimator {
public:
    ConfidenceEstimator();
    ~ConfidenceEstimator() = default;

    void reset();

    /**
     * Updates the overall state given raw detection metrics and audio presence.
     */
    AnalysisState update(bool isAudioPresent,
                         float bpmConfidence,
                         bool bpmStable,
                         float keyConfidence,
                         bool keyStable,
                         float dtSeconds);

    float getSmoothedBpmConfidence() const noexcept { return smoothedBpmConfidence_; }
    float getSmoothedKeyConfidence() const noexcept { return smoothedKeyConfidence_; }

private:
    float smoothedBpmConfidence_ = 0.0f;
    float smoothedKeyConfidence_ = 0.0f;
    float silenceTimer_ = 0.0f;
    float lockedHoldTimer_ = 0.0f;
    AnalysisState currentState_ = AnalysisState::Waiting;

    static constexpr float kSilenceTimeout = 1.2f; // Seconds of silence before reverting to WAITING
};

} // namespace clickclack
