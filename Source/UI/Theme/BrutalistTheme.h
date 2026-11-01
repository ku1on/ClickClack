#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace clickclack {
namespace theme {

// Brutalist monochromatic color palette
static const juce::Colour kBackground     = juce::Colour::fromRGB(12, 12, 12);
static const juce::Colour kSurface        = juce::Colour::fromRGB(20, 20, 20);
static const juce::Colour kSurfaceActive  = juce::Colour::fromRGB(32, 32, 32);
static const juce::Colour kBorder         = juce::Colour::fromRGB(45, 45, 45);
static const juce::Colour kBorderLight    = juce::Colour::fromRGB(80, 80, 80);
static const juce::Colour kBorderBright   = juce::Colour::fromRGB(240, 240, 240);

static const juce::Colour kTextWhite      = juce::Colour::fromRGB(250, 250, 250);
static const juce::Colour kTextDim        = juce::Colour::fromRGB(160, 160, 160);
static const juce::Colour kTextMuted      = juce::Colour::fromRGB(100, 100, 100);

static const juce::Colour kStatusWaiting  = juce::Colour::fromRGB(110, 110, 110);
static const juce::Colour kStatusAnalyze  = juce::Colour::fromRGB(230, 230, 230);
static const juce::Colour kStatusLocked   = juce::Colour::fromRGB(255, 255, 255);

static const juce::Colour kSignalActive   = juce::Colour::fromRGB(255, 255, 255);
static const juce::Colour kSignalInactive = juce::Colour::fromRGB(40, 40, 40);

inline juce::Font getDisplayFont(float height, bool bold = true) {
    juce::Font font{ juce::FontOptions{ height } };
    return font.withStyle(bold ? juce::Font::bold : juce::Font::plain);
}

inline juce::Font getMonospaceFont(float height, bool bold = false) {
    juce::Font font{ juce::FontOptions{ juce::Font::getDefaultMonospacedFontName(), height, bold ? juce::Font::bold : juce::Font::plain } };
    return font;
}

} // namespace theme
} // namespace clickclack
