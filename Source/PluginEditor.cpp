/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularBufferTutorialAudioProcessorEditor::CircularBufferTutorialAudioProcessorEditor (CircularBufferTutorialAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    delaySlider.setSliderStyle(juce::Slider::Rotary);
    delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    delaySlider.setRange(0.1f, 2000.0f, 500.0f);
    addAndMakeVisible(delaySlider);

    feedbackSlider.setSliderStyle(juce::Slider::Rotary);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    feedbackSlider.setRange(0.0f, 0.7f, 0.1f);
    addAndMakeVisible(feedbackSlider);

    volumeSlider.setSliderStyle(juce::Slider::Rotary);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    volumeSlider.setRange(0.0f, 1.0f, 0.8f);
    addAndMakeVisible(volumeSlider);

    dryWetSlider.setSliderStyle(juce::Slider::Rotary);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    dryWetSlider.setRange(0.0f, 1.0f, 0.5f);
    addAndMakeVisible(dryWetSlider);

    timeknob = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "time", delaySlider);
    feedbackKnob = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "feedback", feedbackSlider);
    volumeKnob = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "level", volumeSlider);
    dryWetKnob = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "dryWet", dryWetSlider);
}

CircularBufferTutorialAudioProcessorEditor::~CircularBufferTutorialAudioProcessorEditor()
{
}

//==============================================================================
void CircularBufferTutorialAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.drawText("Time", 10, 5, (getWidth() / 4) - 10, 100, juce::Justification::centred, true);
    g.drawText("Feedback", (getWidth() / 4) + 10, 5, (getWidth() / 4) - 10, 100, juce::Justification::centred, true);
    g.drawText("Volume", 2 * (getWidth() / 4) + 10, 5, (getWidth() / 4) - 10, 100, juce::Justification::centred, true);
    g.drawText("DryWet", 3 * (getWidth() / 4) + 10, 5, (getWidth() / 4) - 10, 100, juce::Justification::centred, true);
    
   
}

void CircularBufferTutorialAudioProcessorEditor::resized()
{
    delaySlider.setBounds(10, 10, (getWidth() / 4) - 10, 100);
    feedbackSlider.setBounds((getWidth() / 4) + 10, 10, (getWidth() / 4) - 10, 100);
    volumeSlider.setBounds(2*(getWidth() / 4) + 10, 10, (getWidth() / 4) - 10, 100);
    dryWetSlider.setBounds(3 * (getWidth() / 4) + 10, 10, (getWidth() / 4) - 10, 100);
}
