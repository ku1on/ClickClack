#include "AudioBufferManager.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace clickclack {

AudioBufferManager::AudioBufferManager(double inputSampleRate, size_t maxWindowSeconds)
    : inputSampleRate_(inputSampleRate)
    , maxWindowCapacity_(static_cast<size_t>(kTargetSampleRate * maxWindowSeconds))
    , circularBuffer_(maxWindowCapacity_, 0.0f)
{
    resampleOutputScratch_.resize(65536, 0.0f);
    resampleInputScratch_.resize(65536, 0.0f);
    setupResampler();
}

AudioBufferManager::~AudioBufferManager() {
    teardownResampler();
}

void AudioBufferManager::setupResampler() {
    teardownResampler();
    if (std::fabs(inputSampleRate_ - kTargetSampleRate) > 1.0) {
        int error = 0;
        srcState_ = src_new(SRC_SINC_FASTEST, 1, &error);
    }
}

void AudioBufferManager::teardownResampler() {
    if (srcState_ != nullptr) {
        src_delete(srcState_);
        srcState_ = nullptr;
    }
}

void AudioBufferManager::setInputSampleRate(double inputSampleRate) {
    if (std::fabs(inputSampleRate_ - inputSampleRate) > 1.0) {
        inputSampleRate_ = inputSampleRate;
        setupResampler();
        reset();
    }
}

void AudioBufferManager::reset() {
    if (srcState_ != nullptr) {
        src_reset(srcState_);
    }
    std::fill(circularBuffer_.begin(), circularBuffer_.end(), 0.0f);
    writeHead_ = 0;
    totalSamplesInBuffer_ = 0;
    newSamplesAccumulated_ = 0;
}

size_t AudioBufferManager::ingestAudio(const float* inputSamples, size_t numInputSamples) {
    if (inputSamples == nullptr || numInputSamples == 0) return 0;

    const float* samplesToPush = inputSamples;
    size_t numSamplesToPush = numInputSamples;

    // Resample if input sample rate is different from 44.1kHz
    if (srcState_ != nullptr) {
        double ratio = kTargetSampleRate / inputSampleRate_;
        size_t maxOutputExpected = static_cast<size_t>(std::ceil(numInputSamples * ratio)) + 256;
        if (resampleOutputScratch_.size() < maxOutputExpected) {
            resampleOutputScratch_.resize(maxOutputExpected, 0.0f);
        }

        SRC_DATA srcData;
        srcData.data_in = inputSamples;
        srcData.input_frames = static_cast<long>(numInputSamples);
        srcData.data_out = resampleOutputScratch_.data();
        srcData.output_frames = static_cast<long>(resampleOutputScratch_.size());
        srcData.src_ratio = ratio;
        srcData.end_of_input = 0;

        int err = src_process(srcState_, &srcData);
        if (err == 0 && srcData.output_frames_gen > 0) {
            samplesToPush = resampleOutputScratch_.data();
            numSamplesToPush = static_cast<size_t>(srcData.output_frames_gen);
        } else {
            return 0;
        }
    }

    // Append resampled samples to circular analysis buffer
    for (size_t i = 0; i < numSamplesToPush; ++i) {
        circularBuffer_[writeHead_] = samplesToPush[i];
        writeHead_ = (writeHead_ + 1) % maxWindowCapacity_;
    }

    totalSamplesInBuffer_ = std::min(totalSamplesInBuffer_ + numSamplesToPush, maxWindowCapacity_);
    newSamplesAccumulated_ += numSamplesToPush;

    return numSamplesToPush;
}

bool AudioBufferManager::getLatestWindow(std::vector<float>& dest, size_t numSamples) const {
    if (totalSamplesInBuffer_ < numSamples || numSamples > maxWindowCapacity_) {
        return false;
    }

    if (dest.size() != numSamples) {
        dest.resize(numSamples);
    }

    // The oldest sample in the window is at:
    // startIdx = (writeHead_ + maxWindowCapacity_ - numSamples) % maxWindowCapacity_
    size_t startIdx = (writeHead_ + maxWindowCapacity_ - numSamples) % maxWindowCapacity_;

    size_t firstChunk = std::min(numSamples, maxWindowCapacity_ - startIdx);
    std::memcpy(dest.data(), &circularBuffer_[startIdx], firstChunk * sizeof(float));

    size_t secondChunk = numSamples - firstChunk;
    if (secondChunk > 0) {
        std::memcpy(dest.data() + firstChunk, &circularBuffer_[0], secondChunk * sizeof(float));
    }

    return true;
}

} // namespace clickclack
