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
                       ),
                    tree(*this, nullptr, "params", {std::make_unique<juce::AudioParameterFloat>(TIME_ID, TIME_NAME, 0.1f, 2000.0f, 500.0f),
                                                    std::make_unique<juce::AudioParameterFloat>(FEEDBACK_ID, FEEDBACK_NAME, 0.0f, 0.7f, 0.1f),
                                                    std::make_unique<juce::AudioParameterFloat>(LEVEL_ID, LEVEL_NAME, 0.0f, 1.0f, 0.8f),
                                                    std::make_unique<juce::AudioParameterFloat>(DRYWET_ID, DRYWET_NAME, 0.0f, 1.0f, 0.5f)})
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
    nDelayBuffer.clear();

    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::sin3dB); // o sinCos para más natural
    dryWetMixer.setWetLatency(0); // importante si el delay genera latencia (este no lo hace)
    dryWetMixer.prepare({ sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels() });

    smoothedMix.reset(sampleRate, 0.05);


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

    smoothedMix.setTargetValue(*tree.getRawParameterValue(DRYWET_ID));
    dryWetMixer.setWetMixProportion(smoothedMix.getNextValue());
    dryWetMixer.pushDrySamples(buffer);  // Guarda copia del dry


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        const float* bufferData = buffer.getReadPointer(channel);
        const float* delayBufferData = nDelayBuffer.getReadPointer(channel);
        const float* wetBufferData = buffer.getReadPointer(channel);

        
        
        fillDelayBuffer(channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        getFromDelayBuffer(buffer, channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        feedbackDelay(channel, bufferLength, delayBufferLength, wetBufferData);



    }

    dryWetMixer.mixWetSamples(buffer);   //mezclo con el buffer procesado

    nWritePosition += bufferLength;
    nWritePosition %= delayBufferLength;   //calculamos el resto para evitar que el nWritter se pase del tamaño del bufferDelay

}

void CircularBufferTutorialAudioProcessor::fillDelayBuffer(int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{

    float levelDelay = *tree.getRawParameterValue(LEVEL_ID);
    //Copiar los datos del main buffer al delay buffer
    if (delayBufferLength > bufferLength + nWritePosition)
    {
        nDelayBuffer.copyFromWithRamp(channel, nWritePosition, bufferData, bufferLength, levelDelay, levelDelay);

    }
    else {
        const int bufferRemaining = delayBufferLength - nWritePosition;

        nDelayBuffer.copyFromWithRamp(channel, nWritePosition, bufferData, bufferRemaining, levelDelay, levelDelay);
        nDelayBuffer.copyFromWithRamp(channel, 0, bufferData + bufferRemaining, bufferLength - bufferRemaining, levelDelay, levelDelay);
    }

    
}

float CircularBufferTutorialAudioProcessor::getWobble()
{
    float wobble = std::sin(2.0f * juce::MathConstants<float>::pi * wobblePhase) * wobbleDepth;
    wobblePhase += wobbleRate / mSampleRate;

    if (wobblePhase >= 1.0f)
        wobblePhase -= 1.0f;
    return wobble;
}

void CircularBufferTutorialAudioProcessor::getFromDelayBuffer(juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    float wobble = getWobble();

    int delayTime = *tree.getRawParameterValue(TIME_ID);
    const int readPosition = static_cast<int> (delayBufferLength + nWritePosition - (mSampleRate * delayTime / 1000) + wobble) % delayBufferLength;

    if (delayBufferLength > bufferLength + readPosition)
    {
        buffer.copyFrom(channel, 0, delayBufferData + readPosition, bufferLength);
    }
    else 
    {
        const int bufferRemaining = delayBufferLength - readPosition;
        buffer.copyFrom(channel, 0, delayBufferData + readPosition, bufferRemaining);
        buffer.copyFrom(channel, bufferRemaining, delayBufferData, bufferLength - bufferRemaining);
    }
}

void CircularBufferTutorialAudioProcessor::feedbackDelay( int channel, const int bufferLength, const int delayBufferLength, const float* wetBuffer)
{
    float feedback = juce::jlimit(0.0f, 0.7f, static_cast<float>(*tree.getRawParameterValue(FEEDBACK_ID)));;

    if (delayBufferLength > bufferLength + nWritePosition)
    {
        nDelayBuffer.addFromWithRamp(channel, nWritePosition, wetBuffer, bufferLength, feedback, feedback);
    }
    else {
        const int bufferReamining = delayBufferLength - nWritePosition;
        nDelayBuffer.addFromWithRamp(channel, bufferReamining, wetBuffer, bufferReamining, feedback, feedback);
        nDelayBuffer.addFromWithRamp(channel, 0, wetBuffer, bufferLength - bufferReamining, feedback, feedback);
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
