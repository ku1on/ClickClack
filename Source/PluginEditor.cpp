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
    // Aspect ratio matching reference design: 740 x 480
    setSize(740, 480);

    addAndMakeVisible(statusBadge_);
    addAndMakeVisible(signalMeter_);
    addAndMakeVisible(bpmConfidenceBar_);
    addAndMakeVisible(keyConfidenceBar_);

    addAndMakeVisible(resetButton_);
    addAndMakeVisible(copyBpmButton_);
    addAndMakeVisible(copyKeyButton_);
    addAndMakeVisible(copyAllButton_);

    addAndMakeVisible(copyBpmIconBtn_);
    addAndMakeVisible(copyKeyIconBtn_);

    // Button actions
    resetButton_.onClick = [this] {
        processorRef_.getAnalysisEngine().reset();
        resetButton_.triggerFeedback("RESET DONE", 900);
    };

    auto handleCopyBpm = [this] {
        if (lastResult_.bpm > 0.0f) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << lastResult_.effectiveBpm();
            copyToClipboard(oss.str());
            copyBpmButton_.triggerFeedback("COPIED");
            copyBpmIconBtn_.triggerFeedback("OK");
        } else {
            copyBpmButton_.triggerFeedback("NO BPM");
        }
    };

    auto handleCopyKey = [this] {
        std::string keyStr = lastResult_.effectiveKeyDisplay();
        if (keyStr != "---") {
            copyToClipboard(keyStr);
            copyKeyButton_.triggerFeedback("COPIED");
            copyKeyIconBtn_.triggerFeedback("OK");
        } else {
            copyKeyButton_.triggerFeedback("NO KEY");
        }
    };

    copyBpmButton_.onClick = handleCopyBpm;
    copyBpmIconBtn_.onClick = handleCopyBpm;

    copyKeyButton_.onClick = handleCopyKey;
    copyKeyIconBtn_.onClick = handleCopyKey;

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
    statusBadge_.setProgress(lastResult_.analysisProgress);
    signalMeter_.setLevel(lastResult_.inputLevelRms, lastResult_.isAudioPresent);
    bpmConfidenceBar_.setConfidence(lastResult_.bpmConfidence);
    keyConfidenceBar_.setConfidence(lastResult_.keyConfidence);

    repaint();
}

void ClickClackAudioProcessorEditor::paint(juce::Graphics& g) {
    // Pure black background
    g.fillAll(theme::kBackground);

    auto totalBounds = getLocalBounds().toFloat();
    float margin = 14.0f;
    auto content = totalBounds.reduced(margin);

    // Outer frame border
    g.setColour(theme::kBorder);
    g.drawRect(content, 1.2f);

    // ========================================================
    // 1. HEADER SECTION
    // ========================================================
    float headerHeight = 52.0f;
    auto headerRect = content.removeFromTop(headerHeight);

    // Draw 4 diagonal slashes logo "///"
    float slashX = headerRect.getX() + 16.0f;
    float slashY = headerRect.getY() + 14.0f;
    float slashH = 24.0f;
    float slashW = 5.0f;
    float slashSlant = 8.0f;
    float slashGap = 4.0f;

    g.setColour(theme::kTextWhite);
    for (int i = 0; i < 4; ++i) {
        float xOffset = i * (slashW + slashGap);
        juce::Path p;
        p.startNewSubPath(slashX + xOffset + slashSlant, slashY);
        p.lineTo(slashX + xOffset + slashSlant + slashW, slashY);
        p.lineTo(slashX + xOffset + slashW, slashY + slashH);
        p.lineTo(slashX + xOffset, slashY + slashH);
        p.closeSubPath();
        g.fillPath(p);
    }

    // Title text "CLICKCLACK"
    float titleX = slashX + 4 * (slashW + slashGap) + 12.0f;
    g.setFont(theme::getDisplayFont(26.0f, true));
    g.setColour(theme::kTextWhite);
    g.drawText("CLICKCLACK", titleX, headerRect.getY() + 8.0f, 240.0f, 36.0f, juce::Justification::centredLeft, false);

    // Header Right: "[ REAL-TIME ANALYSIS ]"
    g.setFont(theme::getMonospaceFont(12.0f, false));
    g.setColour(theme::kTextWhite);
    g.drawText("[ REAL-TIME ANALYSIS ]", headerRect.getRight() - 210.0f, headerRect.getY() + 14.0f, 195.0f, 24.0f, juce::Justification::centredRight, false);

    // Horizontal divider below header
    g.setColour(theme::kBorder);
    g.drawHorizontalLine(static_cast<int>(headerRect.getBottom()), content.getX(), content.getRight());

    // ========================================================
    // 2. MAIN GRID & SECTIONS
    // ========================================================
    float bottomRowHeight = 56.0f;
    auto mainGrid = content.removeFromTop(content.getHeight() - bottomRowHeight);

    float leftColWidth = 185.0f;
    auto leftCol = mainGrid.removeFromLeft(leftColWidth);

    // Vertical divider line separating left column from right area
    g.setColour(theme::kBorder);
    g.drawVerticalLine(static_cast<int>(leftCol.getRight()), mainGrid.getY(), mainGrid.getBottom());

    // Paint Left Column Headings
    g.setFont(theme::getMonospaceFont(11.0f, true));
    g.setColour(theme::kTextDim);

    float leftPad = leftCol.getX() + 16.0f;
    g.drawText("STATUS", leftPad, leftCol.getY() + 14.0f, 150.0f, 16.0f, juce::Justification::centredLeft, false);
    g.drawText("CONFIDENCE", leftPad, leftCol.getY() + 86.0f, 150.0f, 16.0f, juce::Justification::centredLeft, false);
    g.drawText("INPUT", leftPad, leftCol.getY() + 204.0f, 150.0f, 16.0f, juce::Justification::centredLeft, false);

    // Right Area: Split into Top (BPM) and Bottom (KEY / CAMELOT)
    float bpmRowHeight = 200.0f;
    auto bpmArea = mainGrid.removeFromTop(bpmRowHeight);
    auto keyCamelotArea = mainGrid;

    // Horizontal divider between BPM and KEY/CAMELOT
    g.setColour(theme::kBorder);
    g.drawHorizontalLine(static_cast<int>(bpmArea.getBottom()), mainGrid.getX(), mainGrid.getRight());

    // --- Paint BPM Area ---
    float rightPad = bpmArea.getX() + 24.0f;
    g.setFont(theme::getDisplayFont(12.5f, true));
    g.setColour(theme::kTextWhite);
    g.drawText("BPM", rightPad, bpmArea.getY() + 14.0f, 120.0f, 18.0f, juce::Justification::centredLeft, false);

    if (lastResult_.hasManualBpm) {
        g.setFont(theme::getMonospaceFont(10.5f, true));
        g.setColour(theme::kTextDim);
        g.drawText("[MANUAL]", rightPad + 40.0f, bpmArea.getY() + 14.0f, 80.0f, 18.0f, juce::Justification::centredLeft, false);
    }

    // Hero BPM Value
    juce::String bpmStr = "---.-";
    if (lastResult_.effectiveBpm() > 0.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << lastResult_.effectiveBpm();
        bpmStr = oss.str();
    }

    g.setFont(theme::getDisplayFont(108.0f, true));
    g.setColour(theme::kTextWhite);
    g.drawText(bpmStr, rightPad - 4.0f, bpmArea.getY() + 32.0f, bpmArea.getWidth() - 80.0f, 155.0f, juce::Justification::centredLeft, false);

    // --- Paint Bottom Area (KEY & CAMELOT) ---
    float keyWidth = (keyCamelotArea.getWidth()) * 0.65f;
    auto keyArea = keyCamelotArea.removeFromLeft(keyWidth);
    auto camelotArea = keyCamelotArea;

    // Vertical divider between KEY and CAMELOT
    g.setColour(theme::kBorder);
    g.drawVerticalLine(static_cast<int>(keyArea.getRight()), keyArea.getY(), keyArea.getBottom());

    // Paint KEY Cell
    float keyPadX = keyArea.getX() + 24.0f;
    g.setFont(theme::getDisplayFont(12.5f, true));
    g.setColour(theme::kTextWhite);
    g.drawText("KEY", keyPadX, keyArea.getY() + 12.0f, 120.0f, 18.0f, juce::Justification::centredLeft, false);

    juce::String keyStr = "---";
    if (!lastResult_.rootNote.empty()) {
        keyStr = juce::String(lastResult_.effectiveKeyDisplay()).toUpperCase();
    }

    g.setFont(theme::getDisplayFont(46.0f, true));
    g.setColour(theme::kTextWhite);
    g.drawText(keyStr, keyPadX, keyArea.getY() + 36.0f, keyArea.getWidth() - 80.0f, 64.0f, juce::Justification::centredLeft, false);

    // Paint CAMELOT Cell
    float camelotPadX = camelotArea.getX() + 24.0f;
    g.setFont(theme::getDisplayFont(12.5f, true));
    g.setColour(theme::kTextWhite);
    g.drawText("CAMELOT", camelotPadX, camelotArea.getY() + 12.0f, 120.0f, 18.0f, juce::Justification::centredLeft, false);

    juce::String camelotStr = lastResult_.effectiveCamelot();
    g.setFont(theme::getDisplayFont(68.0f, true));
    g.setColour(theme::kTextWhite);
    g.drawText(camelotStr, camelotArea.getX(), camelotArea.getY() + 24.0f, camelotArea.getWidth() - 16.0f, 80.0f, juce::Justification::centred, false);

    // ========================================================
    // 3. BOTTOM DIVIDER & BUTTONS
    // ========================================================
    g.setColour(theme::kBorder);
    g.drawHorizontalLine(static_cast<int>(content.getY()), content.getX(), content.getRight());
}

void ClickClackAudioProcessorEditor::resized() {
    auto totalBounds = getLocalBounds();
    int margin = 14;
    auto content = totalBounds.reduced(margin);

    int headerHeight = 52;
    content.removeFromTop(headerHeight);

    int bottomRowHeight = 56;
    auto mainGrid = content.removeFromTop(content.getHeight() - bottomRowHeight);
    auto bottomArea = content;

    int leftColWidth = 185;
    auto leftCol = mainGrid.removeFromLeft(leftColWidth);

    // Layout Left Column Components
    int leftPad = leftCol.getX() + 16;
    int colInnerWidth = leftColWidth - 32;

    statusBadge_.setBounds(leftPad, leftCol.getY() + 36, colInnerWidth, 32);

    bpmConfidenceBar_.setBounds(leftPad, leftCol.getY() + 110, colInnerWidth, 34);
    keyConfidenceBar_.setBounds(leftPad, leftCol.getY() + 152, colInnerWidth, 34);

    signalMeter_.setBounds(leftPad, leftCol.getY() + 228, colInnerWidth, 28);

    // Right Area (BPM & Key/Camelot)
    int bpmRowHeight = 200;
    auto bpmArea = mainGrid.removeFromTop(bpmRowHeight);
    auto keyCamelotArea = mainGrid;

    // BPM Copy Icon Button (far right)
    copyBpmIconBtn_.setBounds(bpmArea.getRight() - 24 - 36, bpmArea.getY() + 70, 36, 36);

    // Key Copy Icon Button
    int keyWidth = static_cast<int>(keyCamelotArea.getWidth() * 0.65f);
    auto keyArea = keyCamelotArea.removeFromLeft(keyWidth);

    copyKeyIconBtn_.setBounds(keyArea.getRight() - 20 - 32, keyArea.getY() + 52, 32, 32);

    // Bottom Action Buttons (RESET, COPY BPM, COPY KEY, COPY ALL)
    int btnMarginY = bottomArea.getY() + 9;
    int btnHeight = 38;
    int btnGap = 10;
    int totalBtnsWidth = bottomArea.getWidth() - 28; // 14px padding each side

    // 4 buttons with copy all slightly wider or equal
    int btnWidth = (totalBtnsWidth - (btnGap * 3)) / 4;
    int startX = bottomArea.getX() + 14;

    resetButton_.setBounds(startX, btnMarginY, btnWidth, btnHeight);
    copyBpmButton_.setBounds(resetButton_.getRight() + btnGap, btnMarginY, btnWidth, btnHeight);
    copyKeyButton_.setBounds(copyBpmButton_.getRight() + btnGap, btnMarginY, btnWidth, btnHeight);
    copyAllButton_.setBounds(copyKeyButton_.getRight() + btnGap, btnMarginY, btnWidth, btnHeight);
}

} // namespace clickclack
