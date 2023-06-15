/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "spleeter_common/spleeter_common.h"

//==============================================================================
SplitterAudioProcessor::SplitterAudioProcessor() : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ), m_filter(nullptr), m_buffer(nullptr)
{
    addParameter(channel0 = new juce::AudioParameterFloat("0x00", "ch0", 0.0f, 1.0f, 1.0f));
    addParameter(channel1 = new juce::AudioParameterFloat("0x01", "ch1", 0.0f, 1.0f, 1.0f));
    addParameter(channel2 = new juce::AudioParameterFloat("0x02", "ch2", 0.0f, 1.0f, 1.0f));
    addParameter(channel3 = new juce::AudioParameterFloat("0x03", "ch3", 0.0f, 1.0f, 1.0f));
    addParameter(channel4 = new juce::AudioParameterFloat("0x04", "ch4", 0.0f, 1.0f, 1.0f));

    std::error_code err;
    auto models_path = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getParentDirectory().getChildFile("models").getFullPathName();
    
    spleeter::Initialize(models_path.toStdString(), { spleeter::SeparationType::TwoStems }, err);
    m_filter = std::make_shared<spleeter::Filter>(spleeter::SeparationType::TwoStems);
    m_filter->set_extra_frame_latency(10);  // TODO: might be a lot...
    m_filter->Init(err);
    DBG("model path: " << models_path);
    DBG("error msg:  " << err.message());
    DBG("error msg:  " << err.value());
}

SplitterAudioProcessor::~SplitterAudioProcessor() 
{
}

//==============================================================================
const juce::String SplitterAudioProcessor::getName() const 
{
    return JucePlugin_Name;
}

bool SplitterAudioProcessor::acceptsMidi() const 
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SplitterAudioProcessor::producesMidi() const 
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SplitterAudioProcessor::isMidiEffect() const 
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SplitterAudioProcessor::getTailLengthSeconds() const 
{ 
    return 0.0; 
}

int SplitterAudioProcessor::getNumPrograms() 
{
    return 1; 
    // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SplitterAudioProcessor::getCurrentProgram() 
{ 
    return 0; 
}

void SplitterAudioProcessor::setCurrentProgram(int index) 
{
}

const juce::String SplitterAudioProcessor::getProgramName(int index) 
{ 
    return {}; 
}

void SplitterAudioProcessor::changeProgramName(int index, const juce::String& newName) 
{
}

//==============================================================================
void SplitterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // resample to 44100Hz
    m_in_interpolator.clear();
    m_out_interpolator.clear();
    for (auto channel_idx = 0; channel_idx < getTotalNumInputChannels(); channel_idx++) 
    {
        m_in_interpolator.push_back(juce::LagrangeInterpolator());
        m_out_interpolator.push_back(juce::LagrangeInterpolator());
    }
    auto desired_interpolation_ratio = 44100 / sampleRate;
    int block_size = samplesPerBlock * desired_interpolation_ratio;
    m_interpolation_ratio = static_cast<float>(block_size) / samplesPerBlock;

    // Initialize the buffer
    m_filter->set_volume(0, 1.0);
    m_filter->set_volume(1, 1.0);
    //m_filter->set_volume(2, 1.0);
    //m_filter->set_volume(3, 1.0);
    //m_filter->set_volume(4, 1.0);
    m_filter->set_block_size(block_size);
    m_buffer = std::make_shared<rtff::AudioBuffer>(block_size, 2);

    // Latency
    setLatencySamples(m_filter->FrameLatency() * (1.0 / m_interpolation_ratio));
}

void SplitterAudioProcessor::releaseResources() 
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SplitterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const 
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() 
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

void SplitterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) 
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    DBG("Input: " << totalNumInputChannels << " Output: " << totalNumOutputChannels);
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.


    // TODO: factorize !
    if (totalNumInputChannels == 2) 
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel) 
        {
            float* channelData = buffer.getWritePointer(channel);
            m_in_interpolator[channel].process(1.0 / m_interpolation_ratio, channelData, m_buffer->data(channel), m_buffer->frame_count(), buffer.getNumSamples(), 0);
            //memcpy(m_buffer->data(channel), channelData, m_buffer->frame_count() * sizeof(float));
        }
    }
    else 
    {
        // For any other case than stereo, sum in the first channel
        // -- cleanup
        memset(m_buffer->data(0), 0, m_buffer->frame_count() * sizeof(float));
        memset(m_buffer->data(1), 0, m_buffer->frame_count() * sizeof(float));

        for (int channel = 0; channel < totalNumInputChannels; ++channel) 
        {
            // -- interpolate input data in channel 0
            float* channelData = buffer.getWritePointer(channel);
            m_in_interpolator[channel].process(1.0 / m_interpolation_ratio, channelData, m_buffer->data(0), m_buffer->frame_count(), buffer.getNumSamples(), 0);
            // -- store the sum in channel 1
            Eigen::Map<Eigen::VectorXf>(m_buffer->data(1), m_buffer->frame_count()).array() += Eigen::Map<Eigen::VectorXf>(m_buffer->data(0), m_buffer->frame_count()).array();
        }
        // divide by the number of channels
        Eigen::Map<Eigen::VectorXf>(m_buffer->data(1), m_buffer->frame_count()).array() /= totalNumInputChannels;
        // copy channel 1 in others
        Eigen::Map<Eigen::VectorXf>(m_buffer->data(0), m_buffer->frame_count()) - Eigen::Map<Eigen::VectorXf>(m_buffer->data(1), m_buffer->frame_count());
    }

    // convert to stereo
    m_filter->set_volume(0, static_cast<float>(channel0->get()));
    m_filter->set_volume(1, static_cast<float>(channel1->get()));
    //m_filter->set_volume(2, static_cast<float>(channel2->get()));
    //m_filter->set_volume(3, static_cast<float>(channel3->get()));
    //m_filter->set_volume(4, static_cast<float>(channel4->get()));
    m_filter->ProcessBlock(m_buffer.get());

    if (totalNumInputChannels == 2) 
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel) 
        {
            float* channelData = buffer.getWritePointer(channel);
            m_out_interpolator[channel].process(m_interpolation_ratio, m_buffer->data(channel), channelData, buffer.getNumSamples(), m_buffer->frame_count(), 0);
            //memcpy(channelData, m_buffer->data(channel), m_buffer->frame_count() * sizeof(float));
        }
    }
    else 
    {
        // TODO: move to another location
        // For any other case than stereo, sum the result in channel 0 and divide by two
        Eigen::Map<Eigen::VectorXf>(m_buffer->data(0), m_buffer->frame_count()).array() += Eigen::Map<Eigen::VectorXf>(m_buffer->data(1), m_buffer->frame_count()).array();
        Eigen::Map<Eigen::VectorXf>(m_buffer->data(0), m_buffer->frame_count()).array() /= 2;
        // Copy in each channel
        for (int channel = 0; channel < totalNumInputChannels; ++channel) 
        {
            float* channelData = buffer.getWritePointer(channel);
            m_out_interpolator[channel].process(m_interpolation_ratio, m_buffer->data(0), channelData, buffer.getNumSamples(), m_buffer->frame_count(), 0);
        }
    }
}

//==============================================================================
bool SplitterAudioProcessor::hasEditor() const 
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SplitterAudioProcessor::createEditor() 
{
    return new SplitterAudioProcessorEditor(*this);
}

//==============================================================================
void SplitterAudioProcessor::getStateInformation(juce::MemoryBlock& destData) 
{
    juce::MemoryOutputStream(destData, true).writeFloat(*channel0);
    juce::MemoryOutputStream(destData, true).writeFloat(*channel1);
    juce::MemoryOutputStream(destData, true).writeFloat(*channel2);
    juce::MemoryOutputStream(destData, true).writeFloat(*channel3);
    juce::MemoryOutputStream(destData, true).writeFloat(*channel4);
}

void SplitterAudioProcessor::setStateInformation(const void* data, int sizeInBytes) 
{
    channel0->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
    channel1->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
    channel2->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
    channel3->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
    channel4->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() 
{
    return new SplitterAudioProcessor();
}
