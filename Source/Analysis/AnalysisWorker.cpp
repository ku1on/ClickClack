#include "AnalysisWorker.h"
#include <chrono>

namespace clickclack {

AnalysisWorker::AnalysisWorker(LockFreeAudioFifo& fifo, double sampleRate)
    : juce::Thread("ClickClackAnalysisWorker")
    , fifo_(fifo)
    , bufferManager_(sampleRate, 12)
{
    fifoReadScratch_.resize(16384, 0.0f);
    bpmWindowScratch_.resize(kBpmWindowSamples, 0.0f);
    keyWindowScratch_.resize(kKeyWindowSamples, 0.0f);
    startThread(juce::Thread::Priority::high);
}

AnalysisWorker::~AnalysisWorker() {
    signalThreadShouldExit();
    stopThread(2000);
}

void AnalysisWorker::setSampleRate(double sampleRate) {
    bufferManager_.setInputSampleRate(sampleRate);
}

void AnalysisWorker::reset() {
    resetRequested_.store(true, std::memory_order_release);
}

void AnalysisWorker::setEnabled(bool enabled) {
    isEnabled_.store(enabled, std::memory_order_relaxed);
}

void AnalysisWorker::setManualBpm(float bpm, bool enabled) {
    std::lock_guard<std::mutex> lock(resultMutex_);
    currentResult_.manualBpm = bpm;
    currentResult_.hasManualBpm = enabled;
}

void AnalysisWorker::setManualKey(const std::string& key, bool enabled) {
    std::lock_guard<std::mutex> lock(resultMutex_);
    currentResult_.manualKey = key;
    currentResult_.hasManualKey = enabled;
}

AnalysisResult AnalysisWorker::getLatestResult() {
    std::lock_guard<std::mutex> lock(resultMutex_);
    return currentResult_;
}

void AnalysisWorker::run() {
    auto lastStepTime = std::chrono::steady_clock::now();

    while (!threadShouldExit()) {
        if (resetRequested_.exchange(false, std::memory_order_acquire)) {
            bufferManager_.reset();
            bpmDetector_.reset();
            keyDetector_.reset();
            confidenceEstimator_.reset();
            accumulatedAudioSeconds_ = 0.0f;

            std::lock_guard<std::mutex> lock(resultMutex_);
            bool mbEnabled = currentResult_.hasManualBpm;
            float mb = currentResult_.manualBpm;
            bool mkEnabled = currentResult_.hasManualKey;
            std::string mk = currentResult_.manualKey;

            currentResult_ = AnalysisResult();
            currentResult_.hasManualBpm = mbEnabled;
            currentResult_.manualBpm = mb;
            currentResult_.hasManualKey = mkEnabled;
            currentResult_.manualKey = mk;
        }

        if (!isEnabled_.load(std::memory_order_relaxed)) {
            wait(50);
            continue;
        }

        // Pull audio from FIFO
        size_t available = fifo_.getAvailableForRead();
        while (available > 0) {
            size_t toRead = std::min(available, fifoReadScratch_.size());
            size_t readCount = fifo_.read(fifoReadScratch_.data(), toRead);
            if (readCount == 0) break;

            bufferManager_.ingestAudio(fifoReadScratch_.data(), readCount);
            available = fifo_.getAvailableForRead();
        }

        // Check if sufficient audio has accumulated for a sliding window analysis step
        if (bufferManager_.hasEnoughNewAudio(kHopSamples)) {
            auto now = std::chrono::steady_clock::now();
            float dtSeconds = std::chrono::duration<float>(now - lastStepTime).count();
            lastStepTime = now;

            performAnalysisStep();
            bufferManager_.markHopConsumed(kHopSamples);
        }

        // Sleep briefly to avoid high CPU usage
        wait(25);
    }
}

void AnalysisWorker::performAnalysisStep() {
    bool hasBpmWindow = bufferManager_.getLatestWindow(bpmWindowScratch_, kBpmWindowSamples);
    bool hasKeyWindow = bufferManager_.getLatestWindow(keyWindowScratch_, kKeyWindowSamples);

    if (!hasBpmWindow || !hasKeyWindow) {
        return;
    }

    // Check if window is silent
    float sumSq = 0.0f;
    for (size_t i = 0; i < kBpmWindowSamples; i += 64) {
        sumSq += bpmWindowScratch_[i] * bpmWindowScratch_[i];
    }
    float rms = std::sqrt(sumSq / static_cast<float>(kBpmWindowSamples / 64));
    bool isAudioPresent = (rms > 0.001f);

    if (isAudioPresent) {
        accumulatedAudioSeconds_ += 0.5f;
    } else {
        accumulatedAudioSeconds_ = std::max(0.0f, accumulatedAudioSeconds_ - 0.5f);
    }

    BPMDetectionResult bpmRes;
    KeyDetectionResult keyRes;

    if (isAudioPresent) {
        bpmRes = bpmDetector_.process(bpmWindowScratch_.data(), kBpmWindowSamples);
        keyRes = keyDetector_.process(keyWindowScratch_.data(), kKeyWindowSamples);
    }

    AnalysisState newState = confidenceEstimator_.update(
        isAudioPresent,
        bpmRes.confidence,
        bpmRes.isStable,
        keyRes.confidence,
        keyRes.isStable,
        0.5f, // Hop duration
        accumulatedAudioSeconds_
    );

    // Update shared result
    std::lock_guard<std::mutex> lock(resultMutex_);
    currentResult_.state = newState;
    currentResult_.isAudioPresent = isAudioPresent;
    currentResult_.accumulatedSeconds = accumulatedAudioSeconds_;
    currentResult_.analysisProgress = std::clamp(accumulatedAudioSeconds_ / 5.5f, 0.0f, 1.0f);

    if (isAudioPresent) {
        currentResult_.bpmConfidence = confidenceEstimator_.getSmoothedBpmConfidence();
        currentResult_.keyConfidence = confidenceEstimator_.getSmoothedKeyConfidence();
        currentResult_.bpmLocked = bpmRes.isStable;
        currentResult_.keyLocked = keyRes.isStable;

        // When locked, publish the stable results
        if (newState == AnalysisState::Locked) {
            if (bpmRes.bpm > 0.0f) {
                currentResult_.bpm = bpmRes.bpm;
            }
            if (!keyRes.rootNote.empty()) {
                currentResult_.rootNote = keyRes.rootNote;
                currentResult_.scale = keyRes.scale;
                currentResult_.camelot = keyRes.camelot;
            }
        } else if (newState == AnalysisState::Analyzing) {
            // During initial warm-up accumulation (first 5-8 seconds),
            // do not emit fluctuating raw preliminary numbers if not locked yet
            if (!currentResult_.bpmLocked) {
                currentResult_.bpm = 0.0f;
                currentResult_.rootNote = "";
                currentResult_.scale = "";
                currentResult_.camelot = "--";
            }
        }
    } else if (newState == AnalysisState::Waiting) {
        currentResult_.bpmConfidence = 0.0f;
        currentResult_.keyConfidence = 0.0f;
        currentResult_.bpmLocked = false;
        currentResult_.keyLocked = false;
        currentResult_.accumulatedSeconds = 0.0f;
        currentResult_.analysisProgress = 0.0f;
    }
}

} // namespace clickclack
