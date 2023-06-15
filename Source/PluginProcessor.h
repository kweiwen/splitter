#pragma once

#include <JuceHeader.h>
#include "spleeter_filter/filter.h"

class SplitterAudioProcessor : public juce::AudioProcessor 
{
public:
	SplitterAudioProcessor();
	~SplitterAudioProcessor();

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

private:
	//==============================================================================
	std::shared_ptr<spleeter::Filter> m_filter;
	std::shared_ptr<rtff::AudioBuffer> m_buffer;
	juce::AudioParameterFloat* channel0;
	juce::AudioParameterFloat* channel1;
	juce::AudioParameterFloat* channel2;
	juce::AudioParameterFloat* channel3;
	juce::AudioParameterFloat* channel4;

	std::vector<juce::LagrangeInterpolator> m_in_interpolator;
	std::vector<juce::LagrangeInterpolator> m_out_interpolator;
	double m_interpolation_ratio;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplitterAudioProcessor)
};
