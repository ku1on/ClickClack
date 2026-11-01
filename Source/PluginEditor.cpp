#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/CamelotMapper.h"
#include <iomanip>
#include <sstream>

namespace clickclack {

ClickClackAudioProcessorEditor::ClickClackAudioProcessorEditor(ClickClackAudioProcessor& p)
    : juce::AudioProcessorEditor(&p)
    , processorRef_(p)
{
    setSize(440, 480);

    addAndMakeVisible(statusBadge_);
    addAndMakeVisible(signalMeter_);
    addAndMakeVisible(bpmConfidenceBar_);
    addAndMakeVisible(keyConfidenceBar_);

    addAndMakeVisible(resetButton_);
    addAndMakeVisible(copyBpmButton_);
    addAndMakeVisible(copyKeyButton_);
    addAndMakeVisible(copyAllButton_);

    resetButton_.onClick = [this] {
        processorRef_.getAnalysisEngine().reset();
        resetButton_.triggerFeedback("RESET DONE", 800);
    };

    copyBpmButton_.onClick = [this] {
        if (lastResult_.bpm > 0.0f) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << lastResult_.effectiveBpm();
            copyToClipboard(oss.str());
            copyBpmButton_.triggerFeedback("COPIED");
        } else {
            copyBpmButton_.triggerFeedback("NO BPM");
        }
    };

    copyKeyButton_.onClick = [this] {
        std::string keyStr = lastResult_.effectiveKeyDisplay();
        if (keyStr != "---") {
            copyToClipboard(keyStr);
            copyKeyButton_.triggerFeedback("COPIED");
        } else {
            copyKeyButton_.triggerFeedback("NO KEY");
        }
    };

    copyAllButton_.onClick = [this] {
        if (lastResult_.bpm > 0.0f) {
            std::string formatted = CamelotMapper::formatAll(
                lastResult_.effectiveBpm(),
                lastResult_.rootNote,
                lastResult_.scale
            );
            copyToClipboard(formatted);
            copyAllButton_.triggerFeedback("COPIED ALL");
        } else {
            copyAllButton_.triggerFeedback("NO DATA");
        }
    };

    startTimerHz(30);
}

ClickClackAudioProcessorEditor::~ClickClackAudioProcessorEditor() {
    stopTimer();
}

void ClickClackAudioProcessorEditor::copyToClipboard(const juce::String& text) {
    juce::SystemClipboard::copyTextToClipboard(text);
}

void ClickClackAudioProcessorEditor::timerCallback() {
    lastResult_ = processorRef_.getAnalysisEngine().getSnapshot();

    statusBadge_.setState(lastResult_.state);
    signalMeter_.setLevel(lastResult_.inputLevelRms, lastResult_.isAudioPresent);
    bpmConfidenceBar_.setConfidence(lastResult_.bpmConfidence);
    keyConfidenceBar_.setConfidence(lastResult_.keyConfidence);

    repaint();
}

void ClickClackAudioProcessorEditor::paint(juce::Graphics& g) {
    // Solid background
    g.fillAll(theme::kBackground);

    auto bounds = getLocalBounds().toFloat();

    // Outer border
    g.setColour(theme::kBorder);
    g.drawRect(bounds, 1.0f);

    // Padding
    float pad = 24.0f;
    auto content = bounds.reduced(pad);

    // 1. Header Row
    auto headerRow = content.removeFromTop(32.0f);
    g.setFont(theme::getDisplayFont(20.0f, true));
    g.setColour(theme::kTextWhite);
    g.drawText("CLICKCLACK", headerRow, juce::Justification::centredLeft, false);

    content.removeFromTop(20.0f);

    // Divider line
    g.setColour(theme::kBorder);
    g.drawHorizontalLine(static_cast<int>(headerRow.getBottom() + 10.0f), pad, bounds.getRight() - pad);

    // 2. BPM Block
    auto bpmBlock = content.removeFromTop(120.0f);
    g.setColour(theme::kSurface);
    g.fillRect(bpmBlock);
    g.setColour(theme::kBorder);
    g.drawRect(bpmBlock, 1.0f);

    auto bpmInner = bpmBlock.reduced(14.0f);
    auto bpmLabelRow = bpmInner.removeFromTop(14.0f);
    g.setFont(theme::getMonospaceFont(11.0f, true));
    g.setColour(theme::kTextDim);
    g.drawText("BPM", bpmLabelRow, juce::Justification::centredLeft, false);

    if (lastResult_.hasManualBpm) {
        g.setColour(theme::kTextDim);
        g.drawText("[MANUAL]", bpmLabelRow, juce::Justification::centredRight, false);
    }

    auto bpmValueRow = bpmInner.removeFromTop(58.0f);
    g.setFont(theme::getDisplayFont(46.0f, true));
    g.setColour(theme::kTextWhite);

    juce::String bpmStr = "---.-";
    if (lastResult_.effectiveBpm() > 0.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << lastResult_.effectiveBpm();
        bpmStr = oss.str();
    }
    g.drawText(bpmStr, bpmValueRow, juce::Justification::centredLeft, false);

    content.removeFromTop(16.0f);

    // 3. KEY & CAMELOT Block
    auto keyCamelotBlock = content.removeFromTop(128.0f);
    
    // Left: KEY (62% width), Right: CAMELOT (38% width)
    float gap = 12.0f;
    float keyWidth = (keyCamelotBlock.getWidth() - gap) * 0.62f;
    float camelotWidth = keyCamelotBlock.getWidth() - gap - keyWidth;

    auto keyBox = keyCamelotBlock.removeFromLeft(keyWidth);
    keyCamelotBlock.removeFromLeft(gap);
    auto camelotBox = keyCamelotBlock;

    // KEY Box Paint
    g.setColour(theme::kSurface);
    g.fillRect(keyBox);
    g.setColour(theme::kBorder);
    g.drawRect(keyBox, 1.0f);

    auto keyInner = keyBox.reduced(14.0f);
    auto keyLabelRow = keyInner.removeFromTop(14.0f);
    g.setFont(theme::getMonospaceFont(11.0f, true));
    g.setColour(theme::kTextDim);
    g.drawText("KEY", keyLabelRow, juce::Justification::centredLeft, false);

    auto keyValueRow = keyInner.removeFromTop(54.0f);
    g.setFont(theme::getDisplayFont(24.0f, true));
    g.setColour(theme::kTextWhite);
    
    juce::String keyStr = "---";
    if (!lastResult_.rootNote.empty()) {
        keyStr = juce::String(lastResult_.effectiveKeyDisplay()).toUpperCase();
    }
    g.drawText(keyStr, keyValueRow, juce::Justification::centredLeft, false);

    // CAMELOT Box Paint
    g.setColour(theme::kSurface);
    g.fillRect(camelotBox);
    g.setColour(theme::kBorder);
    g.drawRect(camelotBox, 1.0f);

    auto camelotInner = camelotBox.reduced(14.0f);
    auto camelotLabelRow = camelotInner.removeFromTop(14.0f);
    g.setFont(theme::getMonospaceFont(11.0f, true));
    g.setColour(theme::kTextDim);
    g.drawText("CAMELOT", camelotLabelRow, juce::Justification::centredLeft, false);

    auto camelotValBounds = camelotInner.removeFromTop(54.0f);
    g.setFont(theme::getDisplayFont(32.0f, true));
    g.setColour(theme::kTextWhite);
    juce::String camelotStr = lastResult_.effectiveCamelot();
    g.drawText(camelotStr, camelotValBounds, juce::Justification::centred, false);
}

void ClickClackAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    int pad = 24;

    // Header Right: Signal Meter + Status Badge
    int headerY = 24;
    int badgeWidth = 92;
    int badgeHeight = 26;
    int signalWidth = 48;

    statusBadge_.setBounds(bounds.getRight() - pad - badgeWidth, headerY, badgeWidth, badgeHeight);
    signalMeter_.setBounds(statusBadge_.getX() - signalWidth - 10, headerY + 4, signalWidth, 18);

    // Confidence bars placed inside their respective blocks
    // BPM Block starts at y = 76, height = 120
    int bpmBlockY = 76;
    bpmConfidenceBar_.setBounds(pad + 14, bpmBlockY + 84, bounds.getWidth() - (pad * 2) - 28, 22);

    // Key Block starts at y = 212, height = 128
    int keyBlockY = 212;
    int keyWidth = static_cast<int>((bounds.getWidth() - (pad * 2) - 12) * 0.62f);
    keyConfidenceBar_.setBounds(pad + 14, keyBlockY + 92, keyWidth - 28, 22);

    // Action Buttons Row at bottom
    int buttonRowY = bounds.getHeight() - pad - 36;
    int totalBtnWidth = bounds.getWidth() - (pad * 2);
    int btnGap = 8;
    int numButtons = 4;
    int btnWidth = (totalBtnWidth - (btnGap * (numButtons - 1))) / numButtons;
    int btnHeight = 36;

    resetButton_.setBounds(pad, buttonRowY, btnWidth, btnHeight);
    copyBpmButton_.setBounds(resetButton_.getRight() + btnGap, buttonRowY, btnWidth, btnHeight);
    copyKeyButton_.setBounds(copyBpmButton_.getRight() + btnGap, buttonRowY, btnWidth, btnHeight);
    copyAllButton_.setBounds(copyKeyButton_.getRight() + btnGap, buttonRowY, btnWidth, btnHeight);
}

} // namespace clickclack
