#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class AudioBufferUtils {

  public:

    /**
     * Normalize the buffer
     *
     * @param buffer
     */
    static void normalize (juce::AudioBuffer<float> &buffer) {
      float magnitude = buffer.getMagnitude(0, buffer.getNumSamples());
      buffer.applyGain(1 / magnitude);
    }


    static int getFirstLoudSample (juce::AudioBuffer<float> &buffer, float threshold) {
      int sample = 0;
      int numSamples = buffer.getNumSamples();
      int numChannels = buffer.getNumChannels();

      while (sample < numSamples) {
        for (int channel = 0; channel < numChannels; channel++) {
          if (buffer.getSample(channel, sample) > threshold) {
            return sample;
          }
        }

        sample++;
      }

      return sample;
    }

    static void trim (juce::AudioBuffer<float> &buffer, float threshold = 0.0001) {
      juce::AudioBuffer<float> copy;
      int numSamples = buffer.getNumSamples();
      int numChannels = buffer.getNumChannels();

      copy.makeCopyOf(buffer);
      copy.reverse(0, copy.getNumSamples());

      int firstLoudSample = AudioBufferUtils::getFirstLoudSample(buffer, threshold);
      int firstSilentSample = numSamples - AudioBufferUtils::getFirstLoudSample(copy, threshold);
      int newNumSamples = firstSilentSample - firstLoudSample;

#if DEBUG
      std::cout << "Trim: 0-" << firstLoudSample << " and " << firstSilentSample << "-" << numSamples << std::endl;
#endif

      copy.setSize(numChannels, newNumSamples);
      for (int channel = 0; channel < numChannels; channel++) {
        copy.copyFrom(channel, 0, buffer, channel, firstLoudSample, newNumSamples);
      }

      buffer.makeCopyOf(copy);
    }
};