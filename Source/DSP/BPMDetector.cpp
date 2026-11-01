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
            "maxTempo", 215.0,
            "minTempo", 45.0,
            "method", "multifeature"));
    } catch (...) {
        rhythmExtractor_.reset();
    }
}

void BPMDetector::reset() {
    history_.clear();
    if (rhythmExtractor_) {
        rhythmExtractor_->reset();
    }
}

float BPMDetector::normalizeTempoRange(float rawBpm, const std::vector<float>& estimates) {
    if (rawBpm <= 0.0f) return 0.0f;

    // Check if an alternate candidate is available in the preferred music tempo range (70 - 175)
    float chosen = rawBpm;

    // If detected BPM is very slow (< 70) and double-time exists in estimates, prefer double-time
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
    }
    // If detected BPM is extremely fast (> 180) and half-time exists, prefer half-time
    else if (chosen > 180.0f) {
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

    // Stability score: high count in history with low variance
    bool isStable = (history_.size() >= 3 && maxDiff < 1.5f && conf >= 0.45f);

    result.bpm = finalBpm;
    // Boost confidence if consecutive windows agree
    float stabilityBonus = (maxDiff < 1.0f && history_.size() >= 3) ? 0.25f : 0.0f;
    result.confidence = std::clamp(conf * 0.75f + stabilityBonus, 0.0f, 1.0f);
    result.isStable = isStable;

    return result;
}

} // namespace clickclack
