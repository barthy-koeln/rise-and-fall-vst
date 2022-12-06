#define AVOID_REALLOCATING false
#define PLAY_LOOP true // FOR DEBUG MODE ONLY

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AudioBufferUtils.h"
#include "BinaryData.h"

PluginProcessor::PluginProcessor () :
  juce::AudioProcessor(
    juce::AudioProcessor::BusesProperties()
      .withOutput("Output", juce::AudioChannelSet::stereo(), true)
  ),
  sampleRate(-1),
  bpm(120),
  samplesPerBlock(0),
  position(0),
  thumbnailCache(5),
  thumbnail(32, formatManager, thumbnailCache),
  guiParams(*this),
  processing(false),
  riseProcessor(
    ThreadType::RISE,
    this->riseSampleBuffer,
    this->guiParams
  ),
  fallProcessor(
    ThreadType::FALL,
    this->fallSampleBuffer,
    this->guiParams
  ),
  play(false) {
  this->formatManager.registerBasicFormats();

  this->addListener(this);
}

PluginProcessor::~PluginProcessor () = default;

const juce::String PluginProcessor::getName () const {
  return JucePlugin_Name;
}

juce::StringArray PluginProcessor::getAlternateDisplayNames () const {
  return {
    juce::CharPointer_UTF8("R&F"),
    juce::CharPointer_UTF8("RiFa"),
    juce::CharPointer_UTF8("Rise&Fall"),
  };
}

bool PluginProcessor::acceptsMidi () const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::producesMidi () const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::isMidiEffect () const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PluginProcessor::getTailLengthSeconds () const {
  return 0.0;
}

int PluginProcessor::getNumPrograms () {
  return 1;
}

int PluginProcessor::getCurrentProgram () { return 0; }

void PluginProcessor::setCurrentProgram ([[maybe_unused]] int index) {}

const juce::String PluginProcessor::getProgramName ([[maybe_unused]] int index) { return {}; }

void PluginProcessor::changeProgramName (
  [[maybe_unused]] int index,
  [[maybe_unused]] const juce::String &name
) {}

//==============================================================================
void PluginProcessor::prepareToPlay (
  double sampleRateIn,
  int maximumExpectedSamplesPerBlock
) {
  this->sampleRate = sampleRateIn;
  this->samplesPerBlock = maximumExpectedSamplesPerBlock;

  juce::AudioPlayHead::CurrentPositionInfo result{};
  juce::AudioPlayHead *const head = this->getPlayHead();

  this->bpm = head && head->getPosition() ? result.bpm : 120;

  this->riseProcessor.prepareToPlay(this->sampleRate, this->bpm);
  this->fallProcessor.prepareToPlay(this->sampleRate, this->bpm);

  if (this->sampleRate > 0) {
    processSample();
  }
}

void PluginProcessor::releaseResources () {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  this->position = 0;
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool PluginProcessor::isBusesLayoutSupported (
  const juce::AudioProcessor::BusesLayout &layouts
) const {
#if JucePlugin_IsMidiEffect
  ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
    return false;
  }

// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

#endif


void PluginProcessor::processBlock (
  juce::AudioBuffer<float> &buffer,
  juce::MidiBuffer &midiMessages
) {
  if (!midiMessages.isEmpty()) {
    for (const juce::MidiMessageMetadata metadata: midiMessages) {
      auto message = metadata.getMessage();

      if (message.isNoteOn(false)) {
        this->position = 0;
        this->play = true;
      }

      if (message.isNoteOff(true)) {
        this->play = false;
        this->position = 0;
      }
    }
  }

#if !PLAY_LOOP
  if (play) {
#endif
  buffer.clear();

  midiMessages.clear();

  if (this->processedSampleBuffer.getNumChannels() > 0 && !this->processing) {
    auto bufferSamplesRemaining = this->processedSampleBuffer.getNumSamples() - this->position;
    int samplesThisTime = juce::jmin(this->samplesPerBlock, bufferSamplesRemaining);

    for (int channel = 0; channel < this->processedSampleBuffer.getNumChannels(); channel++) {
      buffer.addFrom(
        channel,
        0,
        this->processedSampleBuffer,
        channel,
        this->position,
        samplesThisTime,
        0.9f
      );

      this->filters[channel]->processSamples(
        buffer.getWritePointer(channel),
        samplesThisTime
      );
    }

    this->position += samplesThisTime;
    if (this->position >= this->processedSampleBuffer.getNumSamples()) {
#if !PLAY_LOOP
      this->play = false;
#endif
      this->position = 0;
    }
  }
#if !PLAY_LOOP
  }
#endif
}

//==============================================================================
bool PluginProcessor::hasEditor () const {
  return true; // (change this to false if you choose to not supply an editor)
}

void PluginProcessor::getStateInformation (juce::MemoryBlock &destData) {
  auto state = this->guiParams.copyState();
  auto xml = state.createXml();
  juce::AudioProcessor::copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void *data, int sizeInBytes) {
  auto xmlState = juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes);

  if (xmlState == nullptr) {
    return;
  }

  if (!xmlState->hasTagName(this->guiParams.state.getType())) {
    return;
  }

  this->guiParams.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor *PluginProcessor::createEditor () {
  return new PluginEditor(*this, this->guiParams);
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter () {
  return new PluginProcessor();
}

juce::AudioThumbnail &PluginProcessor::getThumbnail () {
  return this->thumbnail;
}

juce::AudioThumbnailCache &PluginProcessor::getThumbnailCache () {
  return this->thumbnailCache;
}

void PluginProcessor::concatenate () {
  // TIME OFFSET
  auto timeOffset = (float) this->guiParams.getParameterAsValue(TIME_OFFSET_ID).getValue();
  int offsetNumSamples = (int) ceil((timeOffset / 1000) * this->sampleRate);
  int numSamples = juce::jmax(
    1,
    this->riseSampleBuffer.getNumSamples() + this->fallSampleBuffer.getNumSamples() + offsetNumSamples
  );

  this->processedSampleBuffer.setSize(
    this->originalSampleBuffer.getNumChannels(),
    numSamples,
    false,
    true,
    AVOID_REALLOCATING
  );

  int overlapStart = this->riseSampleBuffer.getNumSamples() + offsetNumSamples;
  int overlapStop = overlapStart + abs(juce::jmin(offsetNumSamples, 0));
  int overlapLength = overlapStop - overlapStart;

  for (int i = 0; i < processedSampleBuffer.getNumChannels(); i++) {
    for (int j = 0; j < overlapStart && j < this->riseSampleBuffer.getNumSamples(); j++) {
      float value = this->riseSampleBuffer.getSample(i, j);
      this->processedSampleBuffer.setSample(i, j, value);
    }

    for (int j = 0; j < overlapLength; j++) {
      float value = this->fallSampleBuffer.getSample(i, j) + this->riseSampleBuffer.getSample(i, overlapStart + j);
      this->processedSampleBuffer.setSample(i, overlapStart + j, value);
    }

    for (int j = 0; j < this->fallSampleBuffer.getNumSamples() - overlapLength; j++) {
      float value = this->fallSampleBuffer.getSample(i, overlapLength + j);
      this->processedSampleBuffer.setSample(i, overlapStop + j, value);
    }
  }
}

void PluginProcessor::updateThumbnail () {
  int numChannels = this->processedSampleBuffer.getNumChannels();
  int numSamples = this->processedSampleBuffer.getNumSamples();

  this->thumbnail.reset(
    numChannels,
    this->sampleRate,
    numSamples
  );

  this->thumbnail.addBlock(
    0,
    this->processedSampleBuffer,
    0,
    numSamples
  );
}

void PluginProcessor::processSample () {
  if (this->processing) {
    return;
  }

  if (this->originalSampleBuffer.getNumChannels() <= 0) {
    return;
  }

  if (this->sampleRate <= 0) {
    return;
  }

  this->processing = true;

#if DEBUG
  const clock_t start = clock();
#endif

  this->processedSampleBuffer.makeCopyOf(this->originalSampleBuffer);

  this->riseSampleBuffer.makeCopyOf(this->originalSampleBuffer);
  this->fallSampleBuffer.makeCopyOf(this->originalSampleBuffer);

  this->riseProcessor.prepareToPlay(this->sampleRate, this->bpm);
  this->fallProcessor.prepareToPlay(this->sampleRate, this->bpm);

  this->riseProcessor.process();
  this->fallProcessor.process();

  AudioBufferUtils::trim(this->riseSampleBuffer);
  AudioBufferUtils::trim(this->fallSampleBuffer);

  AudioBufferUtils::normalize(this->riseSampleBuffer);
  AudioBufferUtils::normalize(this->fallSampleBuffer);

  concatenate();

  AudioBufferUtils::normalize(this->processedSampleBuffer);

  int numSamples = this->processedSampleBuffer.getNumSamples();
  int fades = (int) (numSamples * 0.1);
  this->processedSampleBuffer.applyGainRamp(0, fades, 0, 1);
  this->processedSampleBuffer.applyGainRamp(
    numSamples - fades,
    fades,
    1,
    0
  );

  this->position = 0;

#if DEBUG
  std::cout << "Processed: " << float((clock() - start)) / CLOCKS_PER_SEC << " s, "
            << this->processedSampleBuffer.getNumChannels() << " Channels, "
            << this->processedSampleBuffer.getNumSamples() << " Samples" << std::endl;
#endif

  this->processing = false;
  updateThumbnail();
}

void PluginProcessor::newSampleLoaded () {
  this->filters.clear();
  for (int i = 0; i < this->originalSampleBuffer.getNumChannels(); i++) {
    this->filters.add(new juce::IIRFilter());
  }

  AudioBufferUtils::normalize(this->originalSampleBuffer);
  AudioBufferUtils::trim(this->originalSampleBuffer);

  this->processSample();
}

void PluginProcessor::loadSampleFromFile (juce::File &file) {
  this->filePath = file.getFullPathName();
  auto reader = this->formatManager.createReaderFor(file);

  if (reader == nullptr) {
    std::cout << "FILE DOES NOT EXIST" << std::endl;
    return;
  }

  auto length = static_cast<int>(reader->lengthInSamples);

  this->originalSampleBuffer.setSize(static_cast<int>(reader->numChannels), length);

  reader->read(
    &this->originalSampleBuffer,
    0,
    length,
    0,
    true,
    true
  );

  this->newSampleLoaded();
}

void PluginProcessor::audioProcessorParameterChanged (
  [[maybe_unused]] juce::AudioProcessor *processor,
  int parameterIndex,
  [[maybe_unused]] float newValue
) {
  if (this->sampleRate <= 0) {
    return;
  }

  if (
    parameterIndex == FILTER_RESONANCE ||
    parameterIndex == FILTER_CUTOFF ||
    parameterIndex == FILTER_TYPE
    ) {
    auto resonanceParam = (juce::AudioParameterFloat *) this->guiParams.getParameter(FILTER_RESONANCE_ID);
    auto cutoffParam = (juce::AudioParameterInt *) this->guiParams.getParameter(FILTER_CUTOFF_ID);
    auto typeParam = (juce::AudioParameterChoice *) this->guiParams.getParameter(FILTER_TYPE_ID);

    int filterType = typeParam->getIndex();
    int cutoff = cutoffParam->get();
    float resonance = resonanceParam->get();

    switch (filterType) {
      case 1:
        this->iirCoefficients = juce::IIRCoefficients::makeLowPass(this->sampleRate, cutoff, resonance);
        break;
      case 2:
        this->iirCoefficients = juce::IIRCoefficients::makeHighPass(this->sampleRate, cutoff, resonance);
        break;
      default:
        break;
    }

    for (int i = 0; i < this->processedSampleBuffer.getNumChannels(); i++) {
      this->filters[i]->setCoefficients(this->iirCoefficients);
    }

    return;
  }

  if (parameterIndex == IMPULSE_RESPONSE) {
    auto impulseResponseParam = (juce::AudioParameterChoice *) this->guiParams.getParameter(IMPULSE_RESPONSE_ID);
    this->loadNewImpulseResponse(impulseResponseParam->getIndex());

    return;
  }
}

void PluginProcessor::audioProcessorParameterChangeGestureEnd (juce::AudioProcessor *processor, int parameterIndex) {
  if (
    parameterIndex == FILTER_RESONANCE ||
    parameterIndex == FILTER_CUTOFF ||
    parameterIndex == FILTER_TYPE ||
    parameterIndex == IMPULSE_RESPONSE
    ) {
    return;
  }

  if (parameterIndex == TIME_OFFSET) {
    this->processing = true;
    this->concatenate();
    this->position = 0;
    this->processing = false;
    this->updateThumbnail();
    return;
  }

  this->processSample();
}

int PluginProcessor::getPosition () const { return this->position; }

int PluginProcessor::getNumSamples () {
  return this->processedSampleBuffer.getNumSamples();
}

void PluginProcessor::loadNewImpulseResponse (int id) {
  const char *resourceName;
  int resourceSize;

  switch (id) {
    case 5:
      resourceName = BinaryData::university_of_york_stairwell48khznormtrim_wav;
      resourceSize = BinaryData::university_of_york_stairwell48khznormtrim_wavSize;
      break;
    case 4:
      resourceName = BinaryData::empty_apartment_bedroom48khznormtrim_wav;
      resourceSize = BinaryData::empty_apartment_bedroom48khznormtrim_wavSize;
      break;
    case 3:
      resourceName = BinaryData::st_georges48khznormtrim_wav;
      resourceSize = BinaryData::st_georges48khznormtrim_wavSize;
      break;
    case 2:
      resourceName = BinaryData::nuclear_reactor_hall48khznormtrim_wav;
      resourceSize = BinaryData::nuclear_reactor_hall48khznormtrim_wavSize;
      break;
    case 1:
      resourceName = BinaryData::york_minster48khznormtrim_wav;
      resourceSize = BinaryData::york_minster48khznormtrim_wavSize;
      break;
    case 0:
    default:
      resourceName = BinaryData::warehouse48khznormtrim_wav;
      resourceSize = BinaryData::warehouse48khznormtrim_wavSize;
  }

  this->riseProcessor.prepareReverb(resourceName, static_cast<size_t>(resourceSize));
  this->fallProcessor.prepareReverb(resourceName, static_cast<size_t>(resourceSize));

  this->processSample();
}

void PluginProcessor::audioProcessorChanged (
  [[maybe_unused]] juce::AudioProcessor *processor,
  [[maybe_unused]] const juce::AudioProcessorListener::ChangeDetails &details
) {

}
