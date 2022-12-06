#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <soundtouch/SoundTouch.h>
#include "GUIParams.h"

typedef enum ThreadTypeEnum {
  RISE = 0,
  FALL
} ThreadType;

class SubProcessor {
  public:
    SubProcessor (
      ThreadType threadType,
      juce::AudioBuffer<float> &audioBuffer,
      GUIParams &guiParams
    );

    ~SubProcessor ();

    void process ();

    void prepareToPlay (double sampleRate, double bpm);

    void prepareReverb (const void *sourceData, size_t sourceDataSize);

  private:
    juce::AudioBuffer<float> &bufferIn;
    GUIParams &parameters;
    ThreadType type;
    double sampleRate;
    double bpm;

    const void *lastIRName;

    /**
     * SoundTouch instance for time warping
     */
    soundtouch::SoundTouch soundTouch;

    /**
     * Convolution engine for the reverb effect
     */
    juce::dsp::Convolution convolution;

    /**
     * Warp audio samples to change the speed and pitch
     *
     * @param buffer
     * @param factor
     */
    void applyTimeWarp (int factor);

    /**
     *
     * @param target
     * @param base
     * @param dampen
     * @param delayTimeInSamples
     * @param iteration
     */
    void applyDelay (
      juce::AudioBuffer<float> &base,
      float dampen,
      int delayTimeInSamples,
      int iteration
    );

    /**
     *
     * @param target
     */
    void applyReverb (float mix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubProcessor)
};
