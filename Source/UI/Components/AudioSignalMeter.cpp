#include "AudioSignalMeter.h"
#include <cmath>

namespace clickclack {

AudioSignalMeter::AudioSignalMeter() = default;

void AudioSignalMeter::setLevel(float rmsLevel, bool isPresent) {
    if (std::fabs(level_ - rmsLevel) > 0.01f || isPresent_ != isPresent) {
        level_ = rmsLevel;
        isPresent_ = isPresent;
        repaint();
    }
}

void AudioSignalMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Signal text
    auto textBounds = bounds.removeFromLeft(22.0f);
    g.setFont(theme::getMonospaceFont(10.0f, false));
    g.setColour(isPresent_ ? theme::kTextWhite : theme::kTextMuted);
    g.drawText("SIG", textBounds, juce::Justification::centredLeft, false);

    // Mini LED box
    float dotSize = 8.0f;
    float dotY = (bounds.getHeight() - dotSize) * 0.5f;
    juce::Rectangle<float> dotBounds(bounds.getX() + 2.0f, dotY, dotSize, dotSize);

    juce::Colour dotCol = isPresent_ ? theme::kTextWhite : theme::kSignalInactive;
    g.setColour(dotCol);
    g.fillRect(dotBounds);

    g.setColour(isPresent_ ? theme::kBorderBright : theme::kBorder);
    g.drawRect(dotBounds, 1.0f);
}

} // namespace clickclack
