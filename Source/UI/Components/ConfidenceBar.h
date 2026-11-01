#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/BrutalistTheme.h"

namespace clickclack {

class ConfidenceBar : public juce::Component {
public:
    explicit ConfidenceBar(const juce::String& label);
    ~ConfidenceBar() override = default;

    void setConfidence(float confidence); // 0.0 to 1.0
    void paint(juce::Graphics& g) override;

private:
    juce::String label_;
    float confidence_ = 0.0f;
};

} // namespace clickclack
