 #pragma once

#include <JuceHeader.h>
#include <list>
#include <utility>
#include "PluginProcessor.h"

class SplitterAudioProcessorEditor : public juce::AudioProcessorEditor 
{
public:
    SplitterAudioProcessorEditor(SplitterAudioProcessor &);
    ~SplitterAudioProcessorEditor() override;
    
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    void UpdateVolumes();
 
    SplitterAudioProcessor &AudioProcessor;
    juce::Slider vocals_volume_slider;
    juce::Slider drums_volume_slider;
    juce::Slider bass_volume_slider;
    juce::Slider piano_volume_slider;
    juce::Slider other_volume_slider;
    
    std::list<std::pair<std::string, juce::Slider*>> volume_sliders;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplitterAudioProcessorEditor)
};
