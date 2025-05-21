/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
//#include "PluginEditor.h"

//==============================================================================
TheKnobAudioProcessor::TheKnobAudioProcessor()
    : parameters (*this, nullptr, juce::Identifier ("TheKnob"),
                  {
        std::make_unique<juce::AudioParameterFloat> ("gain",            // parameterID
                                                     "Gain",            // parameter name
                                                     0.0f,              // minimum value
                                                     1.0f,              // maximum value
                                                     0.5f),             // default value
        std::make_unique<juce::AudioParameterFloat> ("knob",            // parameterID
                                                     "Knob",            // parameter name
                                                     0.0f,              // minimum value
                                                     100.0f,              // maximum value
                                                     50.0f)              // default value
              })
{
    gainParameter  = parameters.getRawParameterValue ("gain");
    knobParameter = parameters.getRawParameterValue("knob");
}

TheKnobAudioProcessor::~TheKnobAudioProcessor(){}

//==============================================================================
void TheKnobAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    previousGain = *gainParameter;
}

void TheKnobAudioProcessor::releaseResources(){}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TheKnobAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TheKnobAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // First update all params based on knob value
    *gainParameter = *knobParameter / 100;
    
    // Apply smooth gain change
    float currentGain = *gainParameter;
    if (juce::approximatelyEqual (currentGain, previousGain))
    {
        buffer.applyGain (currentGain);
    }
    else
    {
        buffer.applyGainRamp (0, buffer.getNumSamples(), previousGain, currentGain);
        previousGain = currentGain;
    }
}

//==============================================================================
void TheKnobAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TheKnobAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
}

//void TheKnobAudioProcessor::knobValueChanged (float value)
//{
//    *gainParameter = value/100;
//}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheKnobAudioProcessor();
}
