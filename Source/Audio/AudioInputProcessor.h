#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>
#include "../Utils/LockFreeFifo.h"

namespace clickclack {

class AudioInputProcessor {
public:
    explicit AudioInputProcessor(LockFreeAudioFifo& fifo);
    ~AudioInputProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    /**
     * Real-time audio callback. Wait-free and allocation-free.
     */
    void process(const juce::AudioBuffer<float>& buffer) noexcept;

    float getRmsLevel() const noexcept {
        return rmsLevel_.load(std::memory_order_relaxed);
    }

    float getPeakLevel() const noexcept {
        return peakLevel_.load(std::memory_order_relaxed);
    }

    bool isAudioPresent() const noexcept {
        return isAudioPresent_.load(std::memory_order_relaxed);
    }

    double getSampleRate() const noexcept {
        return sampleRate_;
    }

private:
    LockFreeAudioFifo& fifo_;
    double sampleRate_ = 44100.0;
    
    // Pre-allocated scratch buffer for mono downmix (sized in prepare)
    std::vector<float> monoScratchBuffer_;

    // Fast decay atomic meters
    std::atomic<float> rmsLevel_{0.0f};
    std::atomic<float> peakLevel_{0.0f};
    std::atomic<bool> isAudioPresent_{false};

    // Silence detection threshold (~ -60 dBFS)
    static constexpr float kSilenceThreshold = 0.001f;
};

} // namespace clickclack
