#include "StatusBadge.h"

namespace clickclack {

StatusBadge::StatusBadge() = default;

void StatusBadge::setState(AnalysisState state) {
    if (state_ != state) {
        state_ = state;
        repaint();
    }
}

void StatusBadge::setProgress(float progress) {
    if (std::fabs(progress_ - progress) > 0.02f) {
        progress_ = progress;
        if (state_ == AnalysisState::Analyzing) {
            repaint();
        }
    }
}

void StatusBadge::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    juce::Colour bg = theme::kBackground;
    juce::Colour border = theme::kBorder;
    juce::Colour textCol = theme::kTextDim;
    juce::String str = "WAITING";

    switch (state_) {
        case AnalysisState::Waiting:
            bg = theme::kBackground;
            border = theme::kBorder;
            textCol = theme::kTextMuted;
            str = "WAITING";
            break;
        case AnalysisState::Analyzing:
            bg = theme::kBackground;
            border = theme::kTextWhite;
            textCol = theme::kTextWhite;
            if (progress_ > 0.05f) {
                int pct = static_cast<int>(std::round(progress_ * 100.0f));
                str = "ANALYZING " + juce::String(pct) + "%";
            } else {
                str = "ANALYZING";
            }
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
    g.drawRect(bounds, 1.2f);

    g.setColour(textCol);
    g.setFont(theme::getDisplayFont(12.0f, true));
    g.drawText(str, getLocalBounds(), juce::Justification::centred, false);
}

} // namespace clickclack
