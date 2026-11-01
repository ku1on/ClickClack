#include "StatusBadge.h"

namespace clickclack {

StatusBadge::StatusBadge() = default;

void StatusBadge::setState(AnalysisState state) {
    if (state_ != state) {
        state_ = state;
        repaint();
    }
}

void StatusBadge::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    juce::Colour bg = theme::kSurface;
    juce::Colour border = theme::kBorder;
    juce::Colour textCol = theme::kTextDim;
    const char* str = "WAITING";

    switch (state_) {
        case AnalysisState::Waiting:
            bg = theme::kSurface;
            border = theme::kBorder;
            textCol = theme::kTextMuted;
            str = "WAITING";
            break;
        case AnalysisState::Analyzing:
            bg = theme::kSurfaceActive;
            border = theme::kBorderLight;
            textCol = theme::kTextWhite;
            str = "ANALYZING";
            break;
        case AnalysisState::Locked:
            bg = theme::kTextWhite;
            border = theme::kTextWhite;
            textCol = theme::kBackground;
            str = "LOCKED";
            break;
    }

    g.setColour(bg);
    g.fillRect(bounds);

    g.setColour(border);
    g.drawRect(bounds, 1.0f);

    g.setColour(textCol);
    g.setFont(theme::getMonospaceFont(10.5f, true));
    g.drawText(str, getLocalBounds(), juce::Justification::centred, false);
}

} // namespace clickclack
