#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <cstdlib>
#include <cmath>

class NoteLengthSlider :
  public juce::Slider {
  public:
    explicit NoteLengthSlider (const juce::String &name) :
      juce::Slider(name) {
    }

    juce::String getTextFromValue (double value) override {
      juce::String textValue = juce::String(pow(2, (int) std::abs(value)));

      if (value < 0) {
        return "1/" + textValue;
      }

      return textValue;
    }
};
