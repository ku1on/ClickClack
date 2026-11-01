#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/BrutalistTheme.h"

namespace clickclack {

class BrutalistButton : public juce::Button {
public:
    enum class Style {
        Outline,
        Solid,
        CopyIcon
    };

    explicit BrutalistButton(const juce::String& buttonText, Style style = Style::Outline);
    ~BrutalistButton() override = default;

    void setStyle(Style s);
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    // Shows temporary feedback e.g. "COPIED"
    void triggerFeedback(const juce::String& tempText, int durationMs = 1200);

private:
    juce::String originalText_;
    juce::String feedbackText_;
    Style style_ = Style::Outline;
    bool showingFeedback_ = false;

    void resetFeedback();
};

} // namespace clickclack
