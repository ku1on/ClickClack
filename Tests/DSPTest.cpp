#include "../Source/Utils/LockFreeFifo.h"
#include "../Source/Audio/AudioBufferManager.h"
#include "../Source/Analysis/ConfidenceEstimator.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

using namespace clickclack;

void testLockFreeFifo() {
    LockFreeAudioFifo fifo(1024);
    assert(fifo.getCapacity() == 1024);
    assert(fifo.getAvailableForRead() == 0);

    std::vector<float> inputData(500);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = static_cast<float>(i);
    }

    size_t written = fifo.write(inputData.data(), inputData.size());
    assert(written == 500);
    assert(fifo.getAvailableForRead() == 500);

    std::vector<float> readData(500, 0.0f);
    size_t read = fifo.read(readData.data(), readData.size());
    assert(read == 500);
    assert(fifo.getAvailableForRead() == 0);

    for (size_t i = 0; i < 500; ++i) {
        assert(readData[i] == inputData[i]);
    }

    // Test wrap-around
    std::vector<float> wrapInput(800, 42.0f);
    written = fifo.write(wrapInput.data(), wrapInput.size());
    assert(written == 800);

    std::vector<float> wrapOutput(800, 0.0f);
    read = fifo.read(wrapOutput.data(), wrapOutput.size());
    assert(read == 800);
    for (size_t i = 0; i < 800; ++i) {
        assert(wrapOutput[i] == 42.0f);
    }

    std::cout << "[PASS] LockFreeAudioFifo test passed.\n";
}

void testAudioBufferManager() {
    AudioBufferManager mgr(44100.0, 6);
    assert(mgr.getAvailableSamples() == 0);

    std::vector<float> chunk(44100, 0.5f);
    size_t ingested = mgr.ingestAudio(chunk.data(), chunk.size());
    assert(ingested == 44100);
    assert(mgr.getAvailableSamples() == 44100);
    assert(mgr.hasEnoughNewAudio(22050) == true);

    std::vector<float> window;
    bool success = mgr.getLatestWindow(window, 22050);
    assert(success);
    assert(window.size() == 22050);
    assert(window[0] == 0.5f);

    mgr.markHopConsumed(22050);
    assert(mgr.hasEnoughNewAudio(22050) == true); // 44100 - 22050 = 22050 remaining
    mgr.markHopConsumed(22050);
    assert(mgr.hasEnoughNewAudio(22050) == false); // Now 0 remaining

    std::cout << "[PASS] AudioBufferManager sliding window test passed.\n";
}

void testConfidenceEstimator() {
    ConfidenceEstimator estimator;
    assert(estimator.getSmoothedBpmConfidence() == 0.0f);

    // When audio is present but confidence is low
    AnalysisState state = estimator.update(true, 0.2f, false, 0.1f, false, 0.5f);
    assert(state == AnalysisState::Analyzing);

    // When BPM and Key stabilize
    for (int i = 0; i < 5; ++i) {
        state = estimator.update(true, 0.85f, true, 0.75f, true, 0.5f);
    }
    assert(state == AnalysisState::Locked);
    assert(estimator.getSmoothedBpmConfidence() > 0.6f);

    // When silence occurs
    state = estimator.update(false, 0.0f, false, 0.0f, false, 1.5f);
    assert(state == AnalysisState::Waiting);

    std::cout << "[PASS] ConfidenceEstimator state machine test passed.\n";
}

int main() {
    std::cout << "Running DSP & Architecture Unit Tests...\n";
    testLockFreeFifo();
    testAudioBufferManager();
    testConfidenceEstimator();
    std::cout << "All DSP & Architecture tests passed successfully!\n";
    return 0;
}
