#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class SimpleThumbnailComponent :
  public juce::Component,
  public juce::ChangeListener {
  public:
    SimpleThumbnailComponent (
      juce::AudioThumbnail &t,
      [[maybe_unused]] juce::AudioThumbnailCache &cache,
      CustomLookAndFeel &laf
    ) :
      thumbnail(t),
      lookAndFeel(laf) {
      this->thumbnail.addChangeListener(this);
    }

    void paint (juce::Graphics &g) override {
      if (this->thumbnail.getNumChannels() == 0) {
        this->paintIfNoFileLoaded(g);
        return;
      }

      this->paintIfFileLoaded(g);
    }

    void paintIfNoFileLoaded (juce::Graphics &g) {
      g.fillAll(this->lookAndFeel.COLOUR_WHITE);
      g.setColour(this->lookAndFeel.COLOUR_BLACK);
      g.drawFittedText(
        "No File Loaded",
        this->getLocalBounds(),
        juce::Justification::centred,
        1
      );
    }

    void paintIfFileLoaded (juce::Graphics &g) {
      g.fillAll(this->lookAndFeel.COLOUR_WHITE);
      g.setColour(this->lookAndFeel.COLOUR_RED);
      this->thumbnail.drawChannels(
        g,
        getLocalBounds(),
        0.0,
        this->thumbnail.getTotalLength(),
        1.0f
      );
    }

    void changeListenerCallback (juce::ChangeBroadcaster *source) override {
      if (source == &this->thumbnail) {
        this->thumbnailChanged();
      }
    }

  private:
    void thumbnailChanged () {
      this->repaint();
    }

    juce::AudioThumbnail &thumbnail;
    CustomLookAndFeel &lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleThumbnailComponent)
};
