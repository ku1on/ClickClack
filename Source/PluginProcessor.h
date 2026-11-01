#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Analysis/AnalysisEngine.h"

namespace clickclack {

class ClickClackAudioProcessor : public juce::AudioProcessor {
public:
    ClickClackAudioProcessor();
    ~ClickClackAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    AnalysisEngine& getAnalysisEngine() noexcept { return analysisEngine_; }
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }

private:
    AnalysisEngine analysisEngine_;
    juce::AudioProcessorValueTreeState apvts_;

    std::atomic<float>* paramAnalysisEnabled_ = nullptr;
    std::atomic<float>* paramManualBpm_ = nullptr;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClickClackAudioProcessor)
};

} // namespace clickclack
