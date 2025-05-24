/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"


//==============================================================================
TheKnobAudioProcessor::TheKnobAudioProcessor()
    :   AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true).withOutput ("Output", juce::AudioChannelSet::stereo(), true)),

        mainProcessor (new juce::AudioProcessorGraph()),

        parameters (*this, nullptr, juce::Identifier ("TheKnob"), {
            std::make_unique<juce::AudioParameterInt> ("knob",
                                                       "TheKnob",
                                                       KNOB_MIN_VALUE,
                                                       KNOB_MAX_VALUE,
                                                       KNOB_DEFAULT_VALUE),
            std::make_unique<juce::AudioParameterInt> ("mode",
                                                       "Mode",
                                                       VIOLET,
                                                       CRIMSON,
                                                       VIOLET)
              })
{
    knobParameter = parameters.getRawParameterValue("knob");
    modeParameter = parameters.getRawParameterValue("mode");
    
    initialiseGraph();
}

TheKnobAudioProcessor::~TheKnobAudioProcessor(){}

//==============================================================================
void TheKnobAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mainProcessor->setPlayConfigDetails (getMainBusNumInputChannels(),
                                         getMainBusNumOutputChannels(),
                                         sampleRate, samplesPerBlock);
    mainProcessor->prepareToPlay (sampleRate, samplesPerBlock);
    initialiseGraph();
}

void TheKnobAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    
    if (prevMode != *modeParameter)
    {
        connectGraph();
        prevMode = *modeParameter;
    }
    
    mainProcessor->processBlock (buffer, midiMessages);
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
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheKnobAudioProcessor();
}
