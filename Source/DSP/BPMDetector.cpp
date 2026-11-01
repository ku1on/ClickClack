#include "BPMDetector.h"
#include <essentia/essentia.h>
#include <essentia/algorithmfactory.h>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace clickclack {

BPMDetector::BPMDetector() {
    initializeAlgorithm();
}

BPMDetector::~BPMDetector() = default;

void BPMDetector::initializeAlgorithm() {
    try {
        rhythmExtractor_.reset(essentia::standard::AlgorithmFactory::create("RhythmExtractor2013",
            "maxTempo", 215,
            "minTempo", 45,
            "method", "multifeature"));
    } catch (...) {
        rhythmExtractor_.reset();
    }
}

void BPMDetector::reset() {
    history_.clear();
    lockedBpm_ = 0.0f;
    isLocked_ = false;
    consecutiveDisagreements_ = 0;
    if (rhythmExtractor_) {
        rhythmExtractor_->reset();
    }
}

float BPMDetector::snapToMusicalTempoGrid(float bpm, float /*confidence*/) {
    if (bpm <= 0.0f) return 0.0f;

    // Check distance to nearest integer (e.g. 144.6 -> 145.0)
    float roundedInt = std::round(bpm);
    float diffInt = std::fabs(bpm - roundedInt);

    // In modern music production, 99.5% of tracks are authored on integer BPMs.
    // If estimated BPM is within 0.42 BPM of an integer, snap cleanly!
    if (diffInt <= 0.42f) {
        return roundedInt;
    }

    // Check distance to nearest half-integer (e.g. 124.5, 87.5)
    float halfInt = std::floor(bpm) + 0.5f;
    float diffHalf = std::fabs(bpm - halfInt);
    if (diffHalf <= 0.18f) {
        return halfInt;
    }

    // Otherwise round to 1 decimal place
    return std::round(bpm * 10.0f) / 10.0f;
}

float BPMDetector::normalizeTempoRange(float rawBpm, const std::vector<float>& estimates) {
    if (rawBpm <= 0.0f) return 0.0f;

    float chosen = rawBpm;

    if (chosen < 70.0f) {
        float doubleTime = chosen * 2.0f;
        for (float est : estimates) {
            if (std::fabs(est - doubleTime) < 3.0f) {
                chosen = est;
                break;
            }
        }
        if (chosen < 70.0f && doubleTime <= 180.0f) {
            chosen = doubleTime;
        }
    } else if (chosen > 180.0f) {
        float halfTime = chosen * 0.5f;
        for (float est : estimates) {
            if (std::fabs(est - halfTime) < 3.0f) {
                chosen = est;
                break;
            }
        }
        if (chosen > 180.0f && halfTime >= 70.0f) {
            chosen = halfTime;
        }
    }

    return chosen;
}

BPMDetectionResult BPMDetector::process(const float* audio, size_t numSamples) {
    BPMDetectionResult result;
    if (audio == nullptr || numSamples < 44100 * 2) {
        // Need at least 2 seconds for minimal rhythm estimation
        return result;
    }

    if (!rhythmExtractor_) {
        initializeAlgorithm();
        if (!rhythmExtractor_) return result;
    }

    // Convert input audio to Essentia Real vector
    std::vector<essentia::Real> signal(audio, audio + numSamples);

    essentia::Real rawBpm = 0.0;
    std::vector<essentia::Real> ticks;
    essentia::Real rawConfidence = 0.0;
    std::vector<essentia::Real> rawEstimates;
    std::vector<essentia::Real> bpmIntervals;

    try {
        rhythmExtractor_->input("signal").set(signal);
        rhythmExtractor_->output("bpm").set(rawBpm);
        rhythmExtractor_->output("ticks").set(ticks);
        rhythmExtractor_->output("confidence").set(rawConfidence);
        rhythmExtractor_->output("estimates").set(rawEstimates);
        rhythmExtractor_->output("bpmIntervals").set(bpmIntervals);

        rhythmExtractor_->compute();
    } catch (...) {
        return result;
    }

    if (rawBpm <= 0.0) {
        return result;
    }

    std::vector<float> estimatesFloat;
    for (auto est : rawEstimates) {
        estimatesFloat.push_back(static_cast<float>(est));
    }
    result.candidates = estimatesFloat;

    float normalizedBpm = normalizeTempoRange(static_cast<float>(rawBpm), estimatesFloat);
    float conf = std::clamp(static_cast<float>(rawConfidence), 0.0f, 1.0f);

    // Filter low confidence noise
    if (conf >= kMinConfidenceThreshold) {
        history_.push_back({normalizedBpm, conf});
        if (history_.size() > kMaxHistory) {
            history_.pop_front();
        }
    }

    if (history_.empty()) {
        result.bpm = normalizedBpm;
        result.confidence = conf;
        result.isStable = false;
        return result;
    }

    // Consensus and median smoothing over history
    std::vector<float> sortedBpm;
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;

    for (const auto& item : history_) {
        sortedBpm.push_back(item.bpm);
        float weight = item.confidence * item.confidence;
        weightedSum += item.bpm * weight;
        totalWeight += weight;
    }

    std::sort(sortedBpm.begin(), sortedBpm.end());
    float medianBpm = sortedBpm[sortedBpm.size() / 2];

    // Compute variance from median
    float maxDiff = 0.0f;
    for (float b : sortedBpm) {
        float diff = std::fabs(b - medianBpm);
        if (diff > maxDiff) maxDiff = diff;
    }

    float finalBpm = (totalWeight > 0.001f) ? (weightedSum / totalWeight) : medianBpm;

    // Detect tempo shift: if the latest 3 values are closely clustered together
    // but far from old history, prune older history
    if (history_.size() >= 4) {
        size_t n = history_.size();
        float recent1 = history_[n - 1].bpm;
        float recent2 = history_[n - 2].bpm;
        float recent3 = history_[n - 3].bpm;
        if (std::fabs(recent1 - recent2) < 1.0f && std::fabs(recent2 - recent3) < 1.0f) {
            float recentAvg = (recent1 + recent2 + recent3) / 3.0f;
            if (std::fabs(recentAvg - medianBpm) > 4.0f) {
                // Tempo modulation! Clear older history and switch
                history_.erase(history_.begin(), history_.end() - 3);
                finalBpm = recentAvg;
                maxDiff = 0.5f;
            }
        }
    }

    // Stability score: requires at least 5 frames (approx 5-6 seconds of playback)
    // with tight variance (< 1.4 BPM)
    bool isStable = (history_.size() >= 5 && maxDiff < 1.4f && conf >= 0.40f);

    if (isStable) {
        float candidateBpm = snapToMusicalTempoGrid(medianBpm, conf);
        if (!isLocked_) {
            lockedBpm_ = candidateBpm;
            isLocked_ = true;
            consecutiveDisagreements_ = 0;
        } else {
            // Check if new measurement disagrees significantly
            if (std::fabs(candidateBpm - lockedBpm_) > 2.5f) {
                consecutiveDisagreements_++;
                if (consecutiveDisagreements_ >= 5) {
                    // Confirmed tempo modulation
                    lockedBpm_ = candidateBpm;
                    consecutiveDisagreements_ = 0;
                }
            } else {
                consecutiveDisagreements_ = 0;
            }
        }
    }

    if (isLocked_) {
        result.bpm = lockedBpm_;
        result.isStable = true;
        result.confidence = std::clamp(conf * 0.4f + 0.58f, 0.85f, 0.99f);
    } else {
        result.bpm = snapToMusicalTempoGrid(finalBpm, conf);
        result.isStable = false;
        result.confidence = std::clamp(conf * 0.75f, 0.0f, 0.80f);
    }

    return result;
}

} // namespace clickclack
