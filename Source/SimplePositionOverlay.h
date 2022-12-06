#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include "PluginProcessor.h"

class SimplePositionOverlay :
  public juce::Component,
  private juce::Timer {
  public:
    SimplePositionOverlay (PluginProcessor &pluginProcessor, CustomLookAndFeel &customLookAndFeel) :
      processor(pluginProcessor),
      lookAndFeel(customLookAndFeel) {
      this->startTimer(40);
    }

    ~SimplePositionOverlay () {
      this->stopTimer();
      this->deleteAllChildren();
    }

    void paint (juce::Graphics &g) override {
      auto numSamples = this->processor.getNumSamples();

      if (numSamples <= 0) {
        return;
      }

      int position = this->processor.getPosition();
      float percentage = (float) position / numSamples;
      float drawPosition = (percentage * 656) + 16;
      g.setColour(this->lookAndFeel.COLOUR_BLACK);
      g.drawLine(
        drawPosition,
        0.0f,
        drawPosition,
        static_cast<float>(getHeight()),
        1.0f
      );
    }

  private:
    void timerCallback () override {
      this->repaint();
    }

    PluginProcessor &processor;
    CustomLookAndFeel &lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimplePositionOverlay)
};
