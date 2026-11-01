#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/Theme/BrutalistTheme.h"
#include "UI/Components/BrutalistButton.h"
#include "UI/Components/ConfidenceBar.h"
#include "UI/Components/StatusBadge.h"
#include "UI/Components/AudioSignalMeter.h"

namespace clickclack {

class ClickClackAudioProcessorEditor : public juce::AudioProcessorEditor
                                      , private juce::Timer {
public:
    explicit ClickClackAudioProcessorEditor(ClickClackAudioProcessor&);
    ~ClickClackAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ClickClackAudioProcessor& processorRef_;

    // UI Components
    StatusBadge statusBadge_;
    AudioSignalMeter signalMeter_;
    ConfidenceBar bpmConfidenceBar_{"BPM"};
    ConfidenceBar keyConfidenceBar_{"KEY"};

    BrutalistButton resetButton_{"RESET", BrutalistButton::Style::Outline};
    BrutalistButton copyBpmButton_{"COPY BPM", BrutalistButton::Style::Outline};
    BrutalistButton copyKeyButton_{"COPY KEY", BrutalistButton::Style::Outline};
    BrutalistButton copyAllButton_{"COPY ALL", BrutalistButton::Style::Solid};

    BrutalistButton copyBpmIconBtn_{"", BrutalistButton::Style::CopyIcon};
    BrutalistButton copyKeyIconBtn_{"", BrutalistButton::Style::CopyIcon};

    // Cached values for display
    AnalysisResult lastResult_;

    void timerCallback() override;
    void copyToClipboard(const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClickClackAudioProcessorEditor)
};

} // namespace clickclack
