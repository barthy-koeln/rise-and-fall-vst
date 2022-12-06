#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "CustomLookAndFeel.h"
#include "SimpleThumbnailComponent.h"
#include "SimplePositionOverlay.h"
#include "NoteLengthSlider.h"
#include "PluginProcessor.h"

class PluginEditor :
  public juce::AudioProcessorEditor,
  public juce::Button::Listener {
  public:
    PluginEditor (PluginProcessor &, GUIParams &);

    ~PluginEditor () override;

    void paint (juce::Graphics &) override;

    void resized () override;

  private:
    PluginProcessor &pluginProcessor;
    GUIParams &parameters;
    CustomLookAndFeel customLookAndFeel;

    std::unique_ptr<juce::Slider> timeOffsetSlider{};
    std::unique_ptr<juce::Slider> riseTimeWarpSlider{};
    std::unique_ptr<juce::Slider> fallTimeWarpSlider{};
    std::unique_ptr<juce::Slider> reverbMixSlider{};
    std::unique_ptr<juce::Slider> delayMixSlider{};
    std::unique_ptr<NoteLengthSlider> delayTimeSlider;
    std::unique_ptr<juce::Slider> delayFeedbackSlider{};
    std::unique_ptr<juce::Slider> filterCutoffSlider{};
    std::unique_ptr<juce::Slider> filterResonanceSlider{};
    std::unique_ptr<juce::ComboBox> reverbImpResComboBox;
    std::unique_ptr<juce::ComboBox> filterTypeComboBox;
    std::unique_ptr<juce::ToggleButton> riseReverseToggleButton;
    std::unique_ptr<juce::ToggleButton> riseReverbToggleButton{};
    std::unique_ptr<juce::ToggleButton> riseDelayToggleButton{};
    std::unique_ptr<juce::ToggleButton> fallReverseToggleButton{};
    std::unique_ptr<juce::ToggleButton> fallReverbToggleButton{};
    std::unique_ptr<juce::ToggleButton> fallDelayToggleButton{};

    std::unique_ptr<juce::FileChooser> fileChooser{};

    juce::TextButton loadFileButton{};
    juce::AudioPluginFormatManager formatManager{};

    const juce::Rectangle<int> thumbnailBounds{};

    SimpleThumbnailComponent thumbnailComp;
    SimplePositionOverlay positionOverlay;

    template<class T>
    std::unique_ptr<T> initSlider (
      juce::String parameterId,
      const juce::String &label,
      const juce::String &suffix,
      juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag
    );

    std::unique_ptr<juce::ComboBox> initComboBox (
      const juce::String &parameterId,
      const juce::String &label
    );

    std::unique_ptr<juce::ToggleButton> initToggleButton (const juce::String &parameterId, const juce::String &label);

    void buttonClicked (juce::Button *button) override;

    void loadFileButtonCLicked ();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
