#pragma once

#include <vector>
#include <cstddef>
#include <samplerate.h>

namespace clickclack {

/**
 * Manages audio accumulation and resampling in the background analysis thread.
 * Converts input stream (e.g. 48kHz, 96kHz) to fixed 44.1kHz Essentia target rate
 * and maintains a continuous sliding window for BPM and Key analyzers.
 */
class AudioBufferManager {
public:
    static constexpr double kTargetSampleRate = 44100.0;

    explicit AudioBufferManager(double inputSampleRate = 44100.0,
                                size_t maxWindowSeconds = 12);
    ~AudioBufferManager();

    void setInputSampleRate(double inputSampleRate);
    void reset();

    /**
     * Ingest raw audio from the FIFO, downsamples/resamples to 44.1kHz if needed,
     * and appends to the circular analysis buffer.
     * Returns number of resampled 44.1kHz samples added.
     */
    size_t ingestAudio(const float* inputSamples, size_t numInputSamples);

    /**
     * Checks if enough new audio has arrived to trigger a sliding analysis window.
     */
    bool hasEnoughNewAudio(size_t hopSamples) const noexcept {
        return newSamplesAccumulated_ >= hopSamples;
    }

    void markHopConsumed(size_t hopSamples) noexcept {
        if (newSamplesAccumulated_ >= hopSamples) {
            newSamplesAccumulated_ -= hopSamples;
        } else {
            newSamplesAccumulated_ = 0;
        }
    }

    /**
     * Total resampled samples available in circular buffer (up to capacity).
     */
    size_t getAvailableSamples() const noexcept {
        return totalSamplesInBuffer_;
    }

    /**
     * Extracts the most recent `numSamples` from the 44.1kHz sliding window.
     * Returns true if enough samples were available.
     */
    bool getLatestWindow(std::vector<float>& dest, size_t numSamples) const;

private:
    double inputSampleRate_ = 44100.0;
    const size_t maxWindowCapacity_;
    std::vector<float> circularBuffer_;
    size_t writeHead_ = 0;
    size_t totalSamplesInBuffer_ = 0;
    size_t newSamplesAccumulated_ = 0;

    // Resampler state
    SRC_STATE* srcState_ = nullptr;
    std::vector<float> resampleOutputScratch_;
    std::vector<float> resampleInputScratch_;

    void setupResampler();
    void teardownResampler();
};

} // namespace clickclack
