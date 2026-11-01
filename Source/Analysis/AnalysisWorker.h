#pragma once

#include <juce_core/juce_core.h>
#include <mutex>
#include "../Utils/LockFreeFifo.h"
#include "../Audio/AudioBufferManager.h"
#include "../DSP/BPMDetector.h"
#include "../DSP/KeyDetector.h"
#include "ConfidenceEstimator.h"
#include "AnalysisResult.h"

namespace clickclack {

class AnalysisWorker : public juce::Thread {
public:
    AnalysisWorker(LockFreeAudioFifo& fifo, double sampleRate);
    ~AnalysisWorker() override;

    void setSampleRate(double sampleRate);
    void reset();

    AnalysisResult getLatestResult();

    void setManualBpm(float bpm, bool enabled);
    void setManualKey(const std::string& key, bool enabled);
    void setEnabled(bool enabled);

    void run() override;

private:
    LockFreeAudioFifo& fifo_;
    AudioBufferManager bufferManager_;
    BPMDetector bpmDetector_;
    KeyDetector keyDetector_;
    ConfidenceEstimator confidenceEstimator_;

    std::atomic<bool> isEnabled_{true};
    std::atomic<bool> resetRequested_{false};

    // Shared state between worker and UI
    std::mutex resultMutex_;
    AnalysisResult currentResult_;

    // Analysis configuration
    static constexpr size_t kBpmWindowSamples = 44100 * 6; // 6 seconds
    static constexpr size_t kKeyWindowSamples = 44100 * 6; // 6 seconds
    static constexpr size_t kHopSamples       = 44100 / 2; // 0.5s hop interval

    // Scratch buffers
    std::vector<float> fifoReadScratch_;
    std::vector<float> bpmWindowScratch_;
    std::vector<float> keyWindowScratch_;

    void performAnalysisStep();
};

} // namespace clickclack
