/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularBufferTutorialAudioProcessor::CircularBufferTutorialAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

CircularBufferTutorialAudioProcessor::~CircularBufferTutorialAudioProcessor()
{
}

//==============================================================================
const juce::String CircularBufferTutorialAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CircularBufferTutorialAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CircularBufferTutorialAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CircularBufferTutorialAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CircularBufferTutorialAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CircularBufferTutorialAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CircularBufferTutorialAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CircularBufferTutorialAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CircularBufferTutorialAudioProcessor::getProgramName (int index)
{
    return {};
}

void CircularBufferTutorialAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CircularBufferTutorialAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numInputChannels = getNumInputChannels();
    const int delayBufferSize = 2 * (sampleRate + samplesPerBlock);
    mSampleRate = sampleRate;
    nDelayBuffer.setSize(numInputChannels, delayBufferSize);
}

void CircularBufferTutorialAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CircularBufferTutorialAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CircularBufferTutorialAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = nDelayBuffer.getNumSamples();


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        const float* bufferData = buffer.getReadPointer(channel);
        const float* delayBufferData = nDelayBuffer.getReadPointer(channel);

        fillDelayBuffer(channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        getFromDelayBuffer(buffer, channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
    }

    nWritePosition += bufferLength;
    nWritePosition %= delayBufferLength;   //calculamos el resto para evitar que el nWritter se pase del tamaño del bufferDelay

}

void CircularBufferTutorialAudioProcessor::fillDelayBuffer(int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    //Copiar los datos del main buffer al delay buffer
    if (delayBufferLength > bufferLength + nWritePosition)
    {
        nDelayBuffer.copyFromWithRamp(channel, nWritePosition, bufferData, bufferLength, 0.8, 0.8);

    }
    else {
        const int bufferRemaining = delayBufferLength - nWritePosition;

        nDelayBuffer.copyFromWithRamp(channel, nWritePosition, bufferData, bufferRemaining, 0.8, 0.8);
        nDelayBuffer.copyFromWithRamp(channel, 0, bufferData, bufferLength - bufferRemaining, 0.8, 0.8);
    }

    
}

void CircularBufferTutorialAudioProcessor::getFromDelayBuffer(juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    int delayTime = 500;
    const int readPosition = static_cast<int> (delayBufferLength + nWritePosition - (mSampleRate * delayTime / 1000)) % delayBufferLength;

    if (delayBufferLength > bufferLength + readPosition)
    {
        buffer.addFrom(channel, 0, delayBufferData + readPosition, bufferLength);
    }
    else 
    {
        const int bufferRemaining = delayBufferLength - readPosition;
        buffer.addFrom(channel, 0, delayBufferData + readPosition, bufferRemaining);
        buffer.addFrom(channel, bufferRemaining, delayBufferData, bufferLength - bufferRemaining);
    }
}
//==============================================================================
bool CircularBufferTutorialAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CircularBufferTutorialAudioProcessor::createEditor()
{
    return new CircularBufferTutorialAudioProcessorEditor (*this);
}

//==============================================================================
void CircularBufferTutorialAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CircularBufferTutorialAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CircularBufferTutorialAudioProcessor();
}
