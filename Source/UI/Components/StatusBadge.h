#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Analysis/AnalysisResult.h"
#include "../Theme/BrutalistTheme.h"

namespace clickclack {

class StatusBadge : public juce::Component {
public:
    StatusBadge();
    ~StatusBadge() override = default;

    void setState(AnalysisState state);
    void setProgress(float progress);
    void paint(juce::Graphics& g) override;

private:
    AnalysisState state_ = AnalysisState::Waiting;
    float progress_ = 0.0f;
};

} // namespace clickclack
