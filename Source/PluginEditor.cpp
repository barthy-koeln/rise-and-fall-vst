#define fontSize 16
#define sliderWidth 88
#define sliderHeight 96
#define comboBoxHeight 64
#define toggleButtonHeight 32
#define toggleButtonWidth 188

#include "PluginEditor.h"
#include "BinaryData.h"

template<class T>
std::unique_ptr<T> PluginEditor::initSlider (
  juce::String parameterId,
  [[maybe_unused]] const juce::String &label,
  const juce::String &suffix,
  juce::Slider::SliderStyle style
) {
  auto slider = std::make_unique<T>(parameterId);

  slider->setSliderStyle(style);

  slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  slider->setPopupDisplayEnabled(true, false, this);
  slider->setTextValueSuffix(suffix);

  this->parameters.attach(*slider);
  this->addAndMakeVisible(*slider);

  return slider;
}

std::unique_ptr<juce::ComboBox> PluginEditor::initComboBox (
  const juce::String &parameterId,
  [[maybe_unused]] const juce::String &label
) {
  auto comboBox = std::make_unique<juce::ComboBox>(parameterId);

  this->addAndMakeVisible(*comboBox);
  this->parameters.attach(*comboBox);

  return comboBox;
}

std::unique_ptr<juce::ToggleButton> PluginEditor::initToggleButton (
  const juce::String &parameterId,
  const juce::String &label
) {
  auto button = std::make_unique<juce::ToggleButton>(parameterId);

  button->setButtonText(label);
  this->addAndMakeVisible(*button);
  this->parameters.attach(*button);

  return button;
}

PluginEditor::PluginEditor (
  PluginProcessor &audioProcessor,
  GUIParams &guiParams
) :
  juce::AudioProcessorEditor(&audioProcessor),
  pluginProcessor(audioProcessor),
  parameters(guiParams),
  thumbnailBounds(16, 536, 656, 144),
  thumbnailComp(
    this->pluginProcessor.getThumbnail(),
    this->pluginProcessor.getThumbnailCache(),
    this->customLookAndFeel
  ),
  positionOverlay(audioProcessor, customLookAndFeel) {
  this->timeOffsetSlider = this->initSlider<juce::Slider>(
    TIME_OFFSET_ID,
    "TIME OFFSET",
    this->customLookAndFeel.DIMENSION_MS,
    juce::Slider::LinearHorizontal
  );

  this->riseTimeWarpSlider = this->initSlider<juce::Slider>(
    RISE_TIME_WARP_ID,
    "TIME WARP",
    this->customLookAndFeel.DIMENSION_TIMES
  );

  this->fallTimeWarpSlider = this->initSlider<juce::Slider>(
    FALL_TIME_WARP_ID,
    "TIME WARP",
    this->customLookAndFeel.DIMENSION_TIMES
  );

  this->reverbMixSlider = this->initSlider<juce::Slider>(
    REVERB_MIX_ID,
    "MIX / WET",
    this->customLookAndFeel.DIMENSION_PERCENT
  );

  this->delayMixSlider = this->initSlider<juce::Slider>(
    DELAY_MIX_ID,
    "MIX / WET",
    this->customLookAndFeel.DIMENSION_PERCENT
  );

  this->delayTimeSlider = this->initSlider<NoteLengthSlider>(
    DELAY_TIME_ID,
    "TIME",
    this->customLookAndFeel.DIMENSION_FRACTION
  );

  this->delayFeedbackSlider = this->initSlider<juce::Slider>(
    DELAY_FEEDBACK_ID,
    "FEEDBACK",
    this->customLookAndFeel.DIMENSION_PERCENT
  );

  this->filterCutoffSlider = this->initSlider<juce::Slider>(
    FILTER_CUTOFF_ID,
    "CUTOFF",
    this->customLookAndFeel.DIMENSION_HERTZ
  );

  this->filterResonanceSlider = this->initSlider<juce::Slider>(
    FILTER_RESONANCE_ID,
    "RESONANCE (Q)",
    ""
  );

  this->reverbImpResComboBox = this->initComboBox(IMPULSE_RESPONSE_ID, "IMPULSE RESPONSE");

  this->filterTypeComboBox = this->initComboBox(FILTER_TYPE_ID, "FILTER");

  this->riseReverseToggleButton = this->initToggleButton(RISE_REVERSE_ID, "REVERSE");

  this->riseReverbToggleButton = this->initToggleButton(RISE_REVERB_ID, "REVERB");

  this->riseDelayToggleButton = this->initToggleButton(RISE_DELAY_ID, "DELAY");

  this->fallReverseToggleButton = this->initToggleButton(FALL_REVERSE_ID, "REVERSE");

  this->fallReverbToggleButton = this->initToggleButton(FALL_REVERB_ID, "REVERB");

  this->fallDelayToggleButton = this->initToggleButton(FALL_DELAY_ID, "DELAY");

  this->loadFileButton.setButtonText("LOAD AUDIO FILE");
  this->loadFileButton.addListener(this);
  this->loadFileButton.setColour(
    juce::TextButton::textColourOnId,
    this->customLookAndFeel.COLOUR_BLACK
  );
  this->loadFileButton.setColour(
    juce::TextButton::textColourOffId,
    this->customLookAndFeel.COLOUR_BLACK
  );
  this->loadFileButton.setColour(
    juce::TextButton::textColourOnId,
    this->customLookAndFeel.COLOUR_BLACK
  );
  this->addAndMakeVisible(&loadFileButton);

  this->pluginProcessor.getThumbnail().addChangeListener(&thumbnailComp);
  this->formatManager.addDefaultFormats();

  this->addAndMakeVisible(&thumbnailComp);
  this->addAndMakeVisible(&positionOverlay);

  this->setLookAndFeel(&customLookAndFeel);
  this->setSize(688, 704);
}

PluginEditor::~PluginEditor () {
  this->setLookAndFeel(nullptr);
  this->parameters.detachAll();
}

void PluginEditor::paint (juce::Graphics &g) {
  juce::Image background = juce::ImageCache::getFromMemory(
    BinaryData::background_png,
    BinaryData::background_pngSize
  );
  g.drawImageAt(background, 0, 0);
}

void PluginEditor::resized () {
  this->timeOffsetSlider->setBounds(238, 464, 418, 46);

  this->riseTimeWarpSlider->setBounds(232, 320, sliderWidth, sliderHeight);
  this->fallTimeWarpSlider->setBounds(568, 320, sliderWidth, sliderHeight);

  this->reverbMixSlider->setBounds(232, 176, sliderWidth, sliderHeight);

  this->delayTimeSlider->setBounds(368, 176, sliderWidth, sliderHeight);
  this->delayFeedbackSlider->setBounds(468, 176, sliderWidth, sliderHeight);
  this->delayMixSlider->setBounds(568, 176, sliderWidth, sliderHeight);

  this->filterCutoffSlider->setBounds(468, 32, sliderWidth, sliderHeight);
  this->filterResonanceSlider->setBounds(568, 32, sliderWidth, sliderHeight);

  this->reverbImpResComboBox->setBounds(32, 208, 188, comboBoxHeight);

  this->filterTypeComboBox->setBounds(368, 64, 88, comboBoxHeight);

  this->riseReverseToggleButton->setBounds(32, 320, toggleButtonWidth, toggleButtonHeight);
  this->riseReverbToggleButton->setBounds(32, 352, toggleButtonWidth, toggleButtonHeight);
  this->riseDelayToggleButton->setBounds(32, 384, toggleButtonWidth, toggleButtonHeight);

  this->fallReverseToggleButton->setBounds(368, 320, toggleButtonWidth, toggleButtonHeight);
  this->fallReverbToggleButton->setBounds(368, 352, toggleButtonWidth, toggleButtonHeight);
  this->fallDelayToggleButton->setBounds(368, 384, toggleButtonWidth, toggleButtonHeight);

  this->loadFileButton.setBounds(32, 464, 188, 32);

  this->thumbnailComp.setBounds(this->thumbnailBounds);
  this->positionOverlay.setBounds(this->thumbnailBounds.expanded(16, 16));
}

void PluginEditor::loadFileButtonCLicked () {
  std::cout << "load file button clicked" << std::endl;

  if (!this->fileChooser) {
    std::cout << "create chooser" << std::endl;
    this->fileChooser = std::make_unique<juce::FileChooser>(
      "Select a Wave file",
      juce::File(),
      "*.wav",
      true,
      false,
      nullptr
    );
  }

  this->fileChooser->launchAsync(
    juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
    [this] (const juce::FileChooser &chooserReference) {
      juce::File file(chooserReference.getResult());
      this->pluginProcessor.loadSampleFromFile(file);
    }
  );
}

void PluginEditor::buttonClicked (juce::Button *button) {
  if (button == &this->loadFileButton) {
    this->loadFileButtonCLicked();
  }
}
