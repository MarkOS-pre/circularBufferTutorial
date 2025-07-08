/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CircularBufferTutorialAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CircularBufferTutorialAudioProcessorEditor (CircularBufferTutorialAudioProcessor&);
    ~CircularBufferTutorialAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CircularBufferTutorialAudioProcessor& audioProcessor;

    juce::Slider delaySlider;
    juce::Slider feedbackSlider;
    juce::Slider volumeSlider;
    juce::Slider dryWetSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeknob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBufferTutorialAudioProcessorEditor)
};
