#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <essentia/essentia.h>

namespace clickclack {

static bool sEssentiaInitialized = false;

static void ensureEssentiaInitialized() {
    if (!sEssentiaInitialized) {
        essentia::init();
        sEssentiaInitialized = true;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ClickClackAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"analysisEnabled", 1},
        "Analysis Enabled",
        true
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"manualBpm", 1},
        "Manual BPM Override",
        juce::NormalisableRange<float>(0.0f, 300.0f, 0.1f),
        0.0f
    ));

    return { params.begin(), params.end() };
}

ClickClackAudioProcessor::ClickClackAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts_(*this, nullptr, "Parameters", createParameterLayout())
{
    ensureEssentiaInitialized();
    paramAnalysisEnabled_ = apvts_.getRawParameterValue("analysisEnabled");
    paramManualBpm_ = apvts_.getRawParameterValue("manualBpm");
}

ClickClackAudioProcessor::~ClickClackAudioProcessor() = default;

const juce::String ClickClackAudioProcessor::getName() const {
    return "ClickClack";
}

bool ClickClackAudioProcessor::acceptsMidi() const {
    return false;
}

bool ClickClackAudioProcessor::producesMidi() const {
    return false;
}

bool ClickClackAudioProcessor::isMidiEffect() const {
    return false;
}

double ClickClackAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int ClickClackAudioProcessor::getNumPrograms() {
    return 1;
}

int ClickClackAudioProcessor::getCurrentProgram() {
    return 0;
}

void ClickClackAudioProcessor::setCurrentProgram(int) {}

const juce::String ClickClackAudioProcessor::getProgramName(int) {
    return {};
}

void ClickClackAudioProcessor::changeProgramName(int, const juce::String&) {}

void ClickClackAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    analysisEngine_.prepare(sampleRate, samplesPerBlock);
}

void ClickClackAudioProcessor::releaseResources() {
    analysisEngine_.reset();
}

bool ClickClackAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    // Supports Mono, Stereo, and matching In/Out pass-through
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
        return false;
    }

    return true;
}

void ClickClackAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    // Check parameter state
    bool enabled = (paramAnalysisEnabled_ == nullptr || paramAnalysisEnabled_->load(std::memory_order_relaxed) > 0.5f);
    analysisEngine_.setAnalysisEnabled(enabled);

    if (paramManualBpm_ != nullptr) {
        float mb = paramManualBpm_->load(std::memory_order_relaxed);
        analysisEngine_.setManualBpm(mb, mb > 10.0f);
    }

    // Pass audio through cleanly (transparent monitoring)
    // and tap stream into analysis FIFO (wait-free)
    if (enabled) {
        analysisEngine_.processAudioBlock(buffer);
    }
}

bool ClickClackAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* ClickClackAudioProcessor::createEditor() {
    return new ClickClackAudioProcessorEditor(*this);
}

void ClickClackAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ClickClackAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts_.state.getType())) {
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

} // namespace clickclack

// Plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new clickclack::ClickClackAudioProcessor();
}
