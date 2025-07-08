/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define LEVEL_ID "level"
#define LEVEL_NAME "Level"
#define BPM_ID "bpm"
#define BPM_NAME "Bpm"
#define TIME_ID "time"
#define TIME_NAME "Time"
#define FEEDBACK_ID "feedback"
#define FEEDBACK_NAME "Feedback"
#define DRYWET_ID "dryWet"
#define DRYWET_NAME "DryWet"

//==============================================================================
/**
*/
class CircularBufferTutorialAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CircularBufferTutorialAudioProcessor();
    ~CircularBufferTutorialAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void fillDelayBuffer(int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
    void getFromDelayBuffer(juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
    void feedbackDelay(int channel, const int bufferLength, const int delayBufferLength, const float* druByffer);
    float getWobble();
    juce::AudioProcessorValueTreeState tree;
private:
    juce::AudioBuffer<float> nDelayBuffer;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    juce::SmoothedValue<float> smoothedMix;
    
    int nWritePosition{ 0 };
    int mSampleRate{ 44100 };

    float wobblePhase = 0.0f;
    float wobbleRate = 0.5f; // Hz
    float wobbleDepth = 10.0f; // muestras

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBufferTutorialAudioProcessor)
};
