#define AVOID_REALLOCATING false

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <SoundTouch.h>
#include "GUIParams.h"
#include "SubProcessor.h"

SubProcessor::SubProcessor (
  ThreadType threadType,
  juce::AudioBuffer<float> &audioBuffer,
  GUIParams &guiParams
) :
  bufferIn(audioBuffer),
  parameters(guiParams),
  type(threadType),
  sampleRate(-1),
  bpm(0),
  lastIRName(nullptr) {
  this->soundTouch.setChannels(1); // always iterate over single channels
  this->soundTouch.setSampleRate(static_cast<uint>(this->sampleRate));
}

void SubProcessor::applyTimeWarp (int factor) {
  float realFactor = factor < 0 ? (1.0f / abs(factor)) : (1.0f * factor);

  juce::AudioBuffer<float> copy;
  copy.makeCopyOf(this->bufferIn);

  this->soundTouch.setTempo(realFactor);

  double ratio = this->soundTouch.getInputOutputSampleRatio();

  this->bufferIn.setSize(
    this->bufferIn.getNumChannels(),
    static_cast<int>(ceil(this->bufferIn.getNumSamples() * ratio)),
    false, true, AVOID_REALLOCATING
  );

  for (int channel = 0; channel < this->bufferIn.getNumChannels(); channel++) {
    this->soundTouch.putSamples(
      copy.getReadPointer(channel),
      static_cast<uint>(copy.getNumSamples())
    );

    this->soundTouch.receiveSamples(
      bufferIn.getWritePointer(channel),
      static_cast<uint>(this->bufferIn.getNumSamples())
    );

    this->soundTouch.clear();
  }
}

void SubProcessor::applyDelay (
  juce::AudioBuffer<float> &base,
  const float dampen,
  const int delayTimeInSamples,
  int iteration
) {
  base.applyGain(dampen);
  bool continueIteration = base.getMagnitude(0, base.getNumSamples()) > 0.001f;
  int currentDelayPosition = delayTimeInSamples * iteration;
  int length = this->bufferIn.getNumSamples() + base.getNumSamples() + delayTimeInSamples;

  this->bufferIn.setSize(
    this->bufferIn.getNumChannels(),
    length,
    true,
    true,
    AVOID_REALLOCATING
  );

  for (int channel = 0; channel < this->bufferIn.getNumChannels(); channel++) {
    for (int i = 0; i < base.getNumSamples(); i++) {
      this->bufferIn.addSample(
        channel, i + currentDelayPosition,
        base.getSample(channel, i));
    }
  }

  if (continueIteration) {
    applyDelay(base, dampen, delayTimeInSamples, iteration + 1);
  }
}

void SubProcessor::applyReverb (float mix) {
  int irSize = this->convolution.getCurrentIRSize();

#if DEBUG
  std::cout << "Reverb Params: IR size " << irSize << ", Mix " << mix << std::endl;
#endif

  if (irSize <= 0) {
    return;
  }

  juce::AudioBuffer<float> copy;
  copy.makeCopyOf(this->bufferIn);

  const int processedSize = irSize + copy.getNumSamples() - 1;
  this->bufferIn.setSize(
    this->bufferIn.getNumChannels(),
    processedSize,
    false,
    true,
    AVOID_REALLOCATING
  );

  auto audioBlockIn = juce::dsp::AudioBlock<float>(copy);
  auto audioBlockOut = juce::dsp::AudioBlock<float>(this->bufferIn);
  auto processContext = juce::dsp::ProcessContextNonReplacing<float>(audioBlockIn, audioBlockOut);

  this->convolution.process(processContext);

  bufferIn.applyGain(mix);

  for (int channel = 0; channel < copy.getNumChannels(); channel++) {
    this->bufferIn.addFrom(
      channel,
      0,
      copy,
      channel,
      0,
      copy.getNumSamples(),
      1 - mix
    );
  }
}

void SubProcessor::prepareToPlay (double sampleRateIn, double bpmIn) {
  auto numChannels = this->bufferIn.getNumChannels();

  this->sampleRate = sampleRateIn;
  this->bpm = bpmIn;
  this->soundTouch.setSampleRate(static_cast<uint>(this->sampleRate));

  this->convolution.prepare(
    {
      this->sampleRate,
      static_cast<juce::uint32>(288000),
      static_cast<juce::uint32>(numChannels)
    }
  );
}

void SubProcessor::prepareReverb (const void *sourceData, size_t sourceDataSize) {
  auto numChannels = this->bufferIn.getNumChannels();

  if (this->lastIRName == sourceData) {
    return;
  }

  this->lastIRName = sourceData;

  this->convolution.loadImpulseResponse(
    sourceData,
    sourceDataSize,
    numChannels > 1 ? juce::dsp::Convolution::Stereo::yes : juce::dsp::Convolution::Stereo::no,
    juce::dsp::Convolution::Trim::yes,
    0,
    juce::dsp::Convolution::Normalise::yes
  );
}

void SubProcessor::process () {
  auto delayMix = (float) this->parameters.getParameterAsValue(DELAY_MIX_ID).getValue() / 100.0f;
  auto reverbMix = (float) this->parameters.getParameterAsValue(REVERB_MIX_ID).getValue() / 100.0f;

  juce::String reverbId = (this->type == RISE) ? RISE_REVERB_ID : FALL_REVERB_ID;
  juce::String delayId = (this->type == RISE) ? RISE_DELAY_ID : FALL_DELAY_ID;
  juce::String timeWarpId = (this->type == RISE) ? RISE_TIME_WARP_ID : FALL_TIME_WARP_ID;
  juce::String reverseId = (this->type == RISE) ? RISE_REVERSE_ID : FALL_REVERSE_ID;

  auto reverbEnabled = (bool) this->parameters.getParameterAsValue(reverbId).getValue();
  auto delayEnabled = (bool) this->parameters.getParameterAsValue(delayId).getValue();
  auto timeWarp = (int) this->parameters.getParameterAsValue(timeWarpId).getValue();
  auto reverse = (bool) this->parameters.getParameterAsValue(reverseId).getValue();

  if (timeWarp != 0) {
    applyTimeWarp(timeWarp);
  }

  if (reverbEnabled && reverbMix > 0) {
    applyReverb(reverbMix);
  }

  if (delayEnabled && delayMix > 0) {
    juce::AudioBuffer<float> delayBaseBuffer;
    auto delayFeedbackNormalized = (float) this->parameters.getParameterAsValue(DELAY_FEEDBACK_ID).getValue() / 100.0f;
    auto delayNoteIndex = (float) this->parameters.getParameterAsValue(DELAY_TIME_ID).getValue();
    int samplesPerBeat = (int) ceil((60.0f / this->bpm) * this->sampleRate);
    int delayTimeInSamples;

    double delayNote = pow(2, abs(delayNoteIndex));

    if (delayNoteIndex > 0) {
      delayTimeInSamples = (int) (delayNote * 4 * samplesPerBeat);
    } else {
      delayTimeInSamples = (int) ceil(samplesPerBeat / (abs(delayNote) * 4));
    }

    delayBaseBuffer.makeCopyOf(this->bufferIn);
    delayBaseBuffer.applyGain(delayMix);
    applyDelay(delayBaseBuffer, delayFeedbackNormalized, delayTimeInSamples, 1);
  }

  if (reverse) {
    this->bufferIn.reverse(0, this->bufferIn.getNumSamples());
  }
}

SubProcessor::~SubProcessor () = default;
