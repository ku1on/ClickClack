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

    const int numBars = 12;
    const float barWidth = 4.0f;
    const float gap = 3.0f;
    const float maxBarHeight = bounds.getHeight() - 2.0f;
    const float minBarHeight = 4.0f;

    // Calculate how many bars to light up based on RMS level
    int litCount = 0;
    if (isPresent_) {
        // Logarithmic/perceptual curve for audio level
        float scaled = std::clamp(std::sqrt(level_ * 3.5f), 0.08f, 1.0f);
        litCount = std::clamp(static_cast<int>(std::round(scaled * numBars)), 1, numBars);
    }

    float startX = 0.0f;
    float bottomY = bounds.getBottom();

    for (int i = 0; i < numBars; ++i) {
        float frac = static_cast<float>(i) / static_cast<float>(numBars - 1);
        float h = minBarHeight + frac * (maxBarHeight - minBarHeight);
        float x = startX + i * (barWidth + gap);
        float y = bottomY - h;

        juce::Rectangle<float> barRect(x, y, barWidth, h);

        if (i < litCount) {
            g.setColour(theme::kTextWhite);
        } else {
            g.setColour(theme::kSignalInactive);
        }

        g.fillRect(barRect);
    }
}

} // namespace clickclack
