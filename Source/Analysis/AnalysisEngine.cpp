#include "AnalysisEngine.h"

namespace clickclack {

AnalysisEngine::AnalysisEngine(double sampleRate)
    : fifo_(524288) // ~11.8s capacity
    , inputProcessor_(fifo_)
    , worker_(fifo_, sampleRate)
{
}

AnalysisEngine::~AnalysisEngine() = default;

void AnalysisEngine::prepare(double sampleRate, int maxBlockSize) {
    inputProcessor_.prepare(sampleRate, maxBlockSize);
    worker_.setSampleRate(sampleRate);
}

void AnalysisEngine::reset() {
    fifo_.reset();
    inputProcessor_.reset();
    worker_.reset();
}

void AnalysisEngine::processAudioBlock(const juce::AudioBuffer<float>& buffer) noexcept {
    inputProcessor_.process(buffer);
}

AnalysisResult AnalysisEngine::getSnapshot() {
    AnalysisResult res = worker_.getLatestResult();
    res.inputLevelRms = inputProcessor_.getRmsLevel();
    res.inputLevelPeak = inputProcessor_.getPeakLevel();
    return res;
}

void AnalysisEngine::setManualBpm(float bpm, bool enabled) {
    worker_.setManualBpm(bpm, enabled);
}

void AnalysisEngine::setManualKey(const std::string& key, bool enabled) {
    worker_.setManualKey(key, enabled);
}

void AnalysisEngine::setAnalysisEnabled(bool enabled) {
    worker_.setEnabled(enabled);
}

} // namespace clickclack
