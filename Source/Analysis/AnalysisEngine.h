#pragma once

#include <memory>
#include "../Utils/LockFreeFifo.h"
#include "../Audio/AudioInputProcessor.h"
#include "AnalysisWorker.h"
#include "AnalysisResult.h"

namespace clickclack {

class AnalysisEngine {
public:
    explicit AnalysisEngine(double sampleRate = 44100.0);
    ~AnalysisEngine();

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Called in audio callback processBlock (wait-free)
    void processAudioBlock(const juce::AudioBuffer<float>& buffer) noexcept;

    // Called by UI timer callback
    AnalysisResult getSnapshot();

    void setManualBpm(float bpm, bool enabled);
    void setManualKey(const std::string& key, bool enabled);
    void setAnalysisEnabled(bool enabled);

private:
    LockFreeAudioFifo fifo_;
    AudioInputProcessor inputProcessor_;
    AnalysisWorker worker_;
};

} // namespace clickclack
