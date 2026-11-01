#pragma once

#include <vector>
#include <atomic>
#include <algorithm>
#include <cstddef>
#include <cstring>

namespace clickclack {

/**
 * Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer for audio streaming.
 * Thread-safe for one audio thread writer and one background worker reader.
 * Guaranteed no allocations and wait-free operations.
 */
class LockFreeAudioFifo {
public:
    explicit LockFreeAudioFifo(size_t capacity = 524288) // ~11.8s at 44.1kHz
        : capacity_(capacity)
        , buffer_(capacity, 0.0f)
        , writeIndex_(0)
        , readIndex_(0)
    {
    }

    void reset() noexcept {
        writeIndex_.store(0, std::memory_order_relaxed);
        readIndex_.store(0, std::memory_order_relaxed);
    }

    size_t getCapacity() const noexcept {
        return capacity_;
    }

    size_t getAvailableForRead() const noexcept {
        const size_t w = writeIndex_.load(std::memory_order_acquire);
        const size_t r = readIndex_.load(std::memory_order_relaxed);
        if (w >= r) {
            return w - r;
        }
        return capacity_ - (r - w);
    }

    size_t getAvailableForWrite() const noexcept {
        return capacity_ - 1 - getAvailableForRead();
    }

    /**
     * Called from Audio Thread (processBlock). Wait-free.
     */
    size_t write(const float* data, size_t numSamples) noexcept {
        if (data == nullptr || numSamples == 0) return 0;

        const size_t w = writeIndex_.load(std::memory_order_relaxed);
        const size_t r = readIndex_.load(std::memory_order_acquire);

        size_t available = (w >= r) ? (capacity_ - 1 - (w - r)) : (r - w - 1);
        size_t toWrite = std::min(numSamples, available);

        if (toWrite == 0) return 0;

        size_t firstChunk = std::min(toWrite, capacity_ - w);
        std::memcpy(&buffer_[w], data, firstChunk * sizeof(float));

        size_t secondChunk = toWrite - firstChunk;
        if (secondChunk > 0) {
            std::memcpy(&buffer_[0], data + firstChunk, secondChunk * sizeof(float));
        }

        size_t newWrite = (w + toWrite) % capacity_;
        writeIndex_.store(newWrite, std::memory_order_release);
        return toWrite;
    }

    /**
     * Called from Background Analysis Worker Thread.
     */
    size_t read(float* dest, size_t numSamples) noexcept {
        if (dest == nullptr || numSamples == 0) return 0;

        const size_t w = writeIndex_.load(std::memory_order_acquire);
        const size_t r = readIndex_.load(std::memory_order_relaxed);

        size_t available = (w >= r) ? (w - r) : (capacity_ - (r - w));
        size_t toRead = std::min(numSamples, available);

        if (toRead == 0) return 0;

        size_t firstChunk = std::min(toRead, capacity_ - r);
        std::memcpy(dest, &buffer_[r], firstChunk * sizeof(float));

        size_t secondChunk = toRead - firstChunk;
        if (secondChunk > 0) {
            std::memcpy(dest + firstChunk, &buffer_[0], secondChunk * sizeof(float));
        }

        size_t newRead = (r + toRead) % capacity_;
        readIndex_.store(newRead, std::memory_order_release);
        return toRead;
    }

private:
    const size_t capacity_;
    std::vector<float> buffer_;
    alignas(64) std::atomic<size_t> writeIndex_;
    alignas(64) std::atomic<size_t> readIndex_;
};

} // namespace clickclack
