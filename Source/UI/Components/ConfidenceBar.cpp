#include "ConfidenceBar.h"
#include <cmath>

namespace clickclack {

ConfidenceBar::ConfidenceBar(const juce::String& label)
    : label_(label)
{
}

void ConfidenceBar::setConfidence(float confidence) {
    float clamped = juce::jlimit(0.0f, 1.0f, confidence);
    if (std::fabs(clamped - confidence_) > 0.005f) {
        confidence_ = clamped;
        repaint();
    }
}

void ConfidenceBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Top text row: Label on left, % value on right
    auto textRow = bounds.removeFromTop(14.0f);
    g.setFont(theme::getMonospaceFont(10.0f, false));
    g.setColour(theme::kTextDim);
    g.drawText(label_, textRow, juce::Justification::centredLeft, false);

    int percent = static_cast<int>(std::round(confidence_ * 100.0f));
    juce::String valStr = (percent > 0) ? (juce::String(percent) + "%") : "--%";
    g.setColour(theme::kTextWhite);
    g.drawText(valStr, textRow, juce::Justification::centredRight, false);

    bounds.removeFromTop(4.0f); // Gap

    // Bar track
    float barHeight = 4.0f;
    auto barBounds = bounds.removeFromTop(barHeight);

    g.setColour(theme::kSurfaceActive);
    g.fillRect(barBounds);

    // Filled indicator
    if (confidence_ > 0.01f) {
        auto filledBounds = barBounds;
        filledBounds.setWidth(barBounds.getWidth() * confidence_);
        g.setColour(theme::kTextWhite);
        g.fillRect(filledBounds);
    }

    // Border around track
    g.setColour(theme::kBorder);
    g.drawRect(barBounds, 1.0f);
}

} // namespace clickclack
