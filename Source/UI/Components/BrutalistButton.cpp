#include "BrutalistButton.h"

namespace clickclack {

BrutalistButton::BrutalistButton(const juce::String& buttonText)
    : juce::Button(buttonText)
    , originalText_(buttonText)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void BrutalistButton::triggerFeedback(const juce::String& tempText, int durationMs) {
    feedbackText_ = tempText;
    showingFeedback_ = true;
    repaint();

    juce::Timer::callAfterDelay(durationMs, [safeThis = juce::Component::SafePointer<BrutalistButton>(this)] {
        if (safeThis != nullptr) {
            safeThis->resetFeedback();
        }
    });
}

void BrutalistButton::resetFeedback() {
    showingFeedback_ = false;
    repaint();
}

void BrutalistButton::paintButton(juce::Graphics& g, bool isHighlighted, bool isDown) {
    auto bounds = getLocalBounds().toFloat();

    // Background fill
    juce::Colour bg = theme::kSurface;
    if (showingFeedback_) {
        bg = theme::kBorderLight;
    } else if (isDown) {
        bg = theme::kTextWhite;
    } else if (isHighlighted) {
        bg = theme::kSurfaceActive;
    }
    g.setColour(bg);
    g.fillRect(bounds);

    // Border
    juce::Colour borderCol = isDown ? theme::kTextWhite : (isHighlighted ? theme::kBorderBright : theme::kBorder);
    g.setColour(borderCol);
    g.drawRect(bounds, 1.0f);

    // Text
    juce::Colour textCol = isDown ? theme::kBackground : theme::kTextWhite;
    g.setColour(textCol);
    g.setFont(theme::getMonospaceFont(11.0f, true));

    juce::String textToDraw = showingFeedback_ ? feedbackText_ : originalText_;
    g.drawText(textToDraw, getLocalBounds(), juce::Justification::centred, false);
}

} // namespace clickclack
