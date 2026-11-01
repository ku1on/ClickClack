#include "BrutalistButton.h"

namespace clickclack {

BrutalistButton::BrutalistButton(const juce::String& buttonText, Style style)
    : juce::Button(buttonText)
    , originalText_(buttonText)
    , style_(style)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void BrutalistButton::setStyle(Style s) {
    if (style_ != s) {
        style_ = s;
        repaint();
    }
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

    if (style_ == Style::CopyIcon) {
        // Icon-only copy button [ ⧉ ]
        juce::Colour bg = isDown ? theme::kTextWhite : (isHighlighted ? theme::kSurfaceActive : theme::kSurface);
        if (showingFeedback_) bg = theme::kBorderBright;
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, 4.0f);

        juce::Colour borderCol = (isHighlighted || isDown || showingFeedback_) ? theme::kTextWhite : theme::kBorder;
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.2f);

        if (showingFeedback_) {
            g.setColour(isDown ? theme::kBackground : theme::kTextWhite);
            g.setFont(theme::getMonospaceFont(10.0f, true));
            g.drawText("OK", bounds, juce::Justification::centred, false);
        } else {
            // Draw overlapping squares icon
            float cx = bounds.getCentreX();
            float cy = bounds.getCentreY();
            float s = 9.0f;

            juce::Colour iconCol = isDown ? theme::kBackground : theme::kTextWhite;
            g.setColour(iconCol);

            // Back square (offset up-left)
            g.drawRect(cx - s * 0.7f, cy - s * 0.7f, s, s, 1.2f);
            // Front square (offset down-right with fill background to occlude overlap)
            g.setColour(bg);
            g.fillRect(cx - s * 0.25f, cy - s * 0.25f, s, s);
            g.setColour(iconCol);
            g.drawRect(cx - s * 0.25f, cy - s * 0.25f, s, s, 1.2f);
        }
        return;
    }

    if (style_ == Style::Solid) {
        // Solid inverted button (e.g. COPY ALL)
        juce::Colour bg = showingFeedback_ ? theme::kBorderLight : (isDown ? theme::kBackground : (isHighlighted ? juce::Colours::white : juce::Colour::fromRGB(240, 240, 240)));
        juce::Colour textCol = isDown ? theme::kTextWhite : theme::kBackground;

        g.setColour(bg);
        g.fillRect(bounds);

        g.setColour(theme::kTextWhite);
        g.drawRect(bounds, 1.5f);

        g.setColour(textCol);
        g.setFont(theme::getDisplayFont(12.5f, true));
        juce::String textToDraw = showingFeedback_ ? feedbackText_ : originalText_;
        g.drawText(textToDraw, getLocalBounds(), juce::Justification::centred, false);
        return;
    }

    // Default Outline Button (RESET, COPY BPM, COPY KEY)
    juce::Colour bg = showingFeedback_ ? theme::kBorderLight : (isDown ? theme::kTextWhite : (isHighlighted ? theme::kSurfaceActive : theme::kBackground));
    g.setColour(bg);
    g.fillRect(bounds);

    juce::Colour borderCol = isDown ? theme::kTextWhite : (isHighlighted ? theme::kBorderBright : theme::kTextWhite);
    g.setColour(borderCol);
    g.drawRect(bounds, 1.2f);

    juce::Colour textCol = isDown ? theme::kBackground : theme::kTextWhite;
    g.setColour(textCol);
    g.setFont(theme::getDisplayFont(12.0f, true));

    juce::String textToDraw = showingFeedback_ ? feedbackText_ : originalText_;
    g.drawText(textToDraw, getLocalBounds(), juce::Justification::centred, false);
}

} // namespace clickclack
