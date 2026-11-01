#include "ConfidenceEstimator.h"
#include <algorithm>
#include <cmath>

namespace clickclack {

ConfidenceEstimator::ConfidenceEstimator() {
    reset();
}

void ConfidenceEstimator::reset() {
    smoothedBpmConfidence_ = 0.0f;
    smoothedKeyConfidence_ = 0.0f;
    silenceTimer_ = 0.0f;
    lockedHoldTimer_ = 0.0f;
    currentState_ = AnalysisState::Waiting;
}

AnalysisState ConfidenceEstimator::update(bool isAudioPresent,
                                          float bpmConfidence,
                                          bool bpmStable,
                                          float keyConfidence,
                                          bool keyStable,
                                          float dtSeconds,
                                          float accumulatedAudioSeconds)
{
    if (!isAudioPresent) {
        silenceTimer_ += dtSeconds;
        if (silenceTimer_ >= kSilenceTimeout) {
            smoothedBpmConfidence_ = std::max(0.0f, smoothedBpmConfidence_ - dtSeconds * 0.5f);
            smoothedKeyConfidence_ = std::max(0.0f, smoothedKeyConfidence_ - dtSeconds * 0.5f);
            currentState_ = AnalysisState::Waiting;
            return currentState_;
        }
    } else {
        silenceTimer_ = 0.0f;
    }

    // Smooth confidence meters
    const float alpha = std::clamp(dtSeconds * 2.5f, 0.04f, 0.35f);
    smoothedBpmConfidence_ += alpha * (bpmConfidence - smoothedBpmConfidence_);
    smoothedKeyConfidence_ += alpha * (keyConfidence - smoothedKeyConfidence_);

    // Enforce 5 to 10 seconds evaluation requirement before locking
    constexpr float kMinAnalysisSeconds = 5.0f;
    if (accumulatedAudioSeconds < kMinAnalysisSeconds) {
        currentState_ = isAudioPresent ? AnalysisState::Analyzing : AnalysisState::Waiting;
        return currentState_;
    }

    // Determine state
    if (bpmStable && (keyStable || smoothedKeyConfidence_ >= 0.50f)) {
        currentState_ = AnalysisState::Locked;
    } else if (bpmStable && smoothedBpmConfidence_ >= 0.70f) {
        currentState_ = AnalysisState::Locked;
    } else {
        currentState_ = AnalysisState::Analyzing;
    }

    return currentState_;
}

} // namespace clickclack
