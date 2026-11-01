#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/BrutalistTheme.h"

namespace clickclack {

class AudioSignalMeter : public juce::Component {
public:
    AudioSignalMeter();
    ~AudioSignalMeter() override = default;

    void setLevel(float rmsLevel, bool isPresent);
    void paint(juce::Graphics& g) override;

private:
    float level_ = 0.0f;
    bool isPresent_ = false;
};

} // namespace clickclack
