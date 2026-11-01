#include "AudioInputProcessor.h"
#include <cmath>
#include <algorithm>

namespace clickclack {

AudioInputProcessor::AudioInputProcessor(LockFreeAudioFifo& fifo)
    : fifo_(fifo)
{
    monoScratchBuffer_.resize(8192, 0.0f);
}

void AudioInputProcessor::prepare(double sampleRate, int maxBlockSize) {
    sampleRate_ = sampleRate;
    size_t requiredSize = static_cast<size_t>(std::max(maxBlockSize, 2048));
    if (monoScratchBuffer_.size() < requiredSize) {
        monoScratchBuffer_.resize(requiredSize, 0.0f);
    }
    reset();
}

void AudioInputProcessor::reset() {
    rmsLevel_.store(0.0f, std::memory_order_relaxed);
    peakLevel_.store(0.0f, std::memory_order_relaxed);
    isAudioPresent_.store(false, std::memory_order_relaxed);
}

void AudioInputProcessor::process(const juce::AudioBuffer<float>& buffer) noexcept {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0) return;

    if (static_cast<size_t>(numSamples) > monoScratchBuffer_.size()) {
        // Fallback safety cap without allocation
        return;
    }

    float* mono = monoScratchBuffer_.data();
    float sumSq = 0.0f;
    float peak = 0.0f;

    if (numChannels == 1) {
        const float* src = buffer.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i) {
            float s = src[i];
            mono[i] = s;
            float absS = std::fabs(s);
            if (absS > peak) peak = absS;
            sumSq += s * s;
        }
    } else {
        const float invChannels = 1.0f / static_cast<float>(numChannels);
        for (int i = 0; i < numSamples; ++i) {
            float s = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                s += buffer.getReadPointer(ch)[i];
            }
            s *= invChannels;
            mono[i] = s;
            float absS = std::fabs(s);
            if (absS > peak) peak = absS;
            sumSq += s * s;
        }
    }

    float rms = std::sqrt(sumSq / static_cast<float>(numSamples));
    
    // Smooth level decay
    float prevRms = rmsLevel_.load(std::memory_order_relaxed);
    float prevPeak = peakLevel_.load(std::memory_order_relaxed);
    
    float newRms = (rms > prevRms) ? rms : (prevRms * 0.85f + rms * 0.15f);
    float newPeak = (peak > prevPeak) ? peak : (prevPeak * 0.90f + peak * 0.10f);

    rmsLevel_.store(newRms, std::memory_order_relaxed);
    peakLevel_.store(newPeak, std::memory_order_relaxed);
    isAudioPresent_.store(newRms > kSilenceThreshold, std::memory_order_relaxed);

    // Push into lock-free audio queue
    fifo_.write(mono, static_cast<size_t>(numSamples));
}

} // namespace clickclack
