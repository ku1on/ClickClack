#include "KeyDetector.h"
#include "CamelotMapper.h"
#include <essentia/essentia.h>
#include <essentia/algorithmfactory.h>
#include <algorithm>
#include <cmath>

namespace clickclack {

KeyDetector::KeyDetector() {
    initializeAlgorithm();
}

KeyDetector::~KeyDetector() = default;

void KeyDetector::initializeAlgorithm() {
    try {
        keyExtractor_.reset(essentia::standard::AlgorithmFactory::create("KeyExtractor",
            "profileType", "edma",
            "sampleRate", 44100.0,
            "averageDetuningCorrection", true,
            "hpcpSize", 12,
            "weightType", "cosine"));
    } catch (...) {
        keyExtractor_.reset();
    }
}

void KeyDetector::reset() {
    history_.clear();
    if (keyExtractor_) {
        keyExtractor_->reset();
    }
}

KeyDetectionResult KeyDetector::process(const float* audio, size_t numSamples) {
    KeyDetectionResult result;
    if (audio == nullptr || numSamples < 44100 * 2) {
        // Need at least 2 seconds for minimal tonal HPCP analysis
        return result;
    }

    if (!keyExtractor_) {
        initializeAlgorithm();
        if (!keyExtractor_) return result;
    }

    std::vector<essentia::Real> signal(audio, audio + numSamples);

    std::string detectedKey = "";
    std::string detectedScale = "";
    essentia::Real detectedStrength = 0.0;

    try {
        keyExtractor_->input("audio").set(signal);
        keyExtractor_->output("key").set(detectedKey);
        keyExtractor_->output("scale").set(detectedScale);
        keyExtractor_->output("strength").set(detectedStrength);

        keyExtractor_->compute();
    } catch (...) {
        return result;
    }

    float strength = std::clamp(static_cast<float>(detectedStrength), 0.0f, 1.0f);
    std::string normRoot = CamelotMapper::normalizeRootNote(detectedKey);
    std::string normScale = CamelotMapper::normalizeScale(detectedScale);

    if (normRoot.empty() || strength < kMinStrengthThreshold) {
        // Unreliable or percussive audio without prominent tonal content
        if (!history_.empty()) {
            // Decay history confidence slightly
            for (auto& item : history_) {
                item.confidence *= 0.85f;
            }
        }
        return result;
    }

    // Add to history
    history_.push_back({normRoot, normScale, strength});
    if (history_.size() > kMaxHistory) {
        history_.pop_front();
    }

    // Accumulate weighted votes for each detected key candidate
    std::unordered_map<std::string, float> keyScores;
    float totalWeight = 0.0f;

    for (const auto& item : history_) {
        std::string fullKey = item.rootNote + " " + item.scale;
        float w = item.confidence * item.confidence;
        keyScores[fullKey] += w;
        totalWeight += w;
    }

    std::string bestKeyCombo = normRoot + " " + normScale;
    float bestScore = 0.0f;
    for (const auto& pair : keyScores) {
        if (pair.second > bestScore) {
            bestScore = pair.second;
            bestKeyCombo = pair.first;
        }
    }

    // Parse best root and scale
    auto spacePos = bestKeyCombo.find(' ');
    std::string finalRoot = (spacePos != std::string::npos) ? bestKeyCombo.substr(0, spacePos) : normRoot;
    std::string finalScale = (spacePos != std::string::npos) ? bestKeyCombo.substr(spacePos + 1) : normScale;

    float consensusRatio = (totalWeight > 0.0001f) ? (bestScore / totalWeight) : 0.0f;
    bool isStable = (history_.size() >= 3 && consensusRatio >= 0.65f && strength >= 0.40f);

    result.rootNote = finalRoot;
    result.scale = finalScale;
    result.camelot = CamelotMapper::toCamelot(finalRoot, finalScale);
    result.confidence = std::clamp(strength * 0.7f + consensusRatio * 0.3f, 0.0f, 1.0f);
    result.isStable = isStable;

    return result;
}

} // namespace clickclack
