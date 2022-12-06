#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "SubProcessor.h"
#include "GUIParams.h"

class PluginProcessor :
  public juce::AudioProcessor,
  public juce::AudioProcessorListener {
  public:
    PluginProcessor ();

    ~PluginProcessor () override;

    void prepareToPlay (double sampleRateIn, int maximumExpectedSamplesPerBlock) override;

    void releaseResources () override;

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout &layouts) const override;

    void processBlock (juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor () override;

    bool hasEditor () const override;

    const juce::String getName () const override;

    juce::StringArray getAlternateDisplayNames () const override;

    bool acceptsMidi () const override;

    bool producesMidi () const override;

    bool isMidiEffect () const override;

    double getTailLengthSeconds () const override;

    int getNumPrograms () override;

    int getCurrentProgram () override;

    void setCurrentProgram (int index) override;

    const juce::String getProgramName (int index) override;

    void changeProgramName (int index, const juce::String &newName) override;

    void getStateInformation (juce::MemoryBlock &destData) override;

    void setStateInformation (const void *data, int sizeInBytes) override;

    /**
     * Get the thumbnail
     *
     * @return A reference to the thumbnail
     */
    juce::AudioThumbnail &getThumbnail ();

    juce::AudioThumbnailCache &getThumbnailCache ();

    int getPosition () const;

    int getNumSamples ();

    /**
     * Reset the position and set the new number of samples and channels
     */
    void newSampleLoaded ();

    void loadNewImpulseResponse (int id);

    /**
     * Load an audio sample from a file
     *
     * @param file
     */
    void loadSampleFromFile (juce::File &file);

    /**
     * Cascade the multiple audio processing algorithms
     */
    void processSample ();

  private:
    /**
     * Buffer containing the samples of the original audio file
     */
    juce::AudioBuffer<float> originalSampleBuffer;

    /**
     * Buffer containing the final processed output audio
     */
    juce::AudioBuffer<float> processedSampleBuffer;

    /**
     * Buffer containing the final processed output audio
     */
    juce::AudioBuffer<float> riseSampleBuffer;

    /**
     * Buffer containing the final processed output audio
     */
    juce::AudioBuffer<float> fallSampleBuffer;

    SubProcessor riseProcessor;
    SubProcessor fallProcessor;

    /**
     * Sample rate for the current block
     */
    double sampleRate;

    double bpm{};

    /**
     * Number of samples in the current block
     */
    int samplesPerBlock;

    /**
     * Current position in the processing of sample blocks
     */
    int position;

    /**
     * Handles basic audio formats (wav, aiff)
     */
    juce::AudioFormatManager formatManager;

    /**
     * Cache containing thumbnail previews
     */
    juce::AudioThumbnailCache thumbnailCache;

    /**
     * Thumbnail of the audio waveform
     */
    juce::AudioThumbnail thumbnail;

    /**
     * Stores all the parameters
     */
    GUIParams guiParams;

    /**
     * The loaded sample's file path
     */
    juce::String filePath = "";

    /**
     * Block processing of the sample if it is already in process
     */
    bool processing;

    /**
     * Whether the plugin should start playback or not
     */
    bool play{};

    /**
     * Array of filters (one for each channel)
     */
    juce::OwnedArray<juce::IIRFilter> filters;

    /**
     * Infinite Impulse Response Filter Coefficients
     */
    juce::IIRCoefficients iirCoefficients;

    /**
     * Clone the processed audio, reverse it and finally prepend it to the
     * processed audio buffer
     */
    void concatenate ();

    /**
     * Update the thumbnail image
     */
    void updateThumbnail ();

    void audioProcessorChanged (
      juce::AudioProcessor *processor,
      const juce::AudioProcessorListener::ChangeDetails &details
    ) override;

    /**
     * Act when a parameter changes
     *
     * @param processor
     * @param parameterIndex
     * @param newValue
     */
    void audioProcessorParameterChanged (
      juce::AudioProcessor *processor,
      int parameterIndex,
      float newValue
    ) override;

    /**
     * Act when a gesture changing a parameter ends
     *
     * @param processor
     * @param parameterIndex
     */
    void audioProcessorParameterChangeGestureEnd (
      juce::AudioProcessor *processor,
      int parameterIndex
    ) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
