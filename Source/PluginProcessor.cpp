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
void TheKnobAudioProcessor::initialiseGraph()
{
    mainProcessor->clear();
    
    // I/O nodes & processors
    audioInputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioOutputNode));
    
    // FX nodes & processors
    filterNode = mainProcessor->addNode (std::make_unique<FilterProcessor>(knobParameter, modeParameter));
    eqNode = mainProcessor->addNode (std::make_unique<EQProcessor>(knobParameter, modeParameter));
    specialEqNode = mainProcessor->addNode (std::make_unique<SpecialEQProcessor>(knobParameter, modeParameter));
    reverbNode = mainProcessor->addNode (std::make_unique<ReverbProcessor>(knobParameter, modeParameter));
    delayNode = mainProcessor->addNode (std::make_unique<DelayProcessor>(knobParameter, modeParameter));
    distortionNode = mainProcessor->addNode (std::make_unique<DistortionProcessor>(knobParameter, modeParameter));
    
    connectGraph();
}

void TheKnobAudioProcessor::connectGraph()
{
    for (auto& conn : mainProcessor->getConnections())
    {
        mainProcessor->removeConnection (conn);
    }
    
    switch((int)*modeParameter)
    {
        case VIOLET:
            for (int channel = 0; channel < 2; ++channel)
            {
                // order: filter -> distortion -> reverb -> delay -> EQ
                mainProcessor->addConnection ({ { audioInputNode->nodeID, channel }, { filterNode->nodeID, channel } });
                mainProcessor->addConnection ({ { filterNode->nodeID, channel }, { distortionNode->nodeID, channel } });
                mainProcessor->addConnection ({ { distortionNode->nodeID, channel }, { reverbNode->nodeID, channel } });
                mainProcessor->addConnection ({ { reverbNode->nodeID, channel }, { delayNode->nodeID, channel } });
                mainProcessor->addConnection ({ { delayNode->nodeID, channel }, { eqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { eqNode->nodeID, channel }, { specialEqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { specialEqNode->nodeID, channel }, { audioOutputNode->nodeID, channel } });
            }
            break;
            
        case TEAL:
            for (int channel = 0; channel < 2; ++channel)
            {
                // order: filter -> EQ -> distortion -> delay -> reverb
                mainProcessor->addConnection ({ { audioInputNode->nodeID, channel }, { filterNode->nodeID, channel } });
                mainProcessor->addConnection ({ { filterNode->nodeID, channel }, { eqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { eqNode->nodeID, channel }, { distortionNode->nodeID, channel } });
                mainProcessor->addConnection ({ { distortionNode->nodeID, channel }, { delayNode->nodeID, channel } });
                mainProcessor->addConnection ({ { delayNode->nodeID, channel }, { reverbNode->nodeID, channel } });
                mainProcessor->addConnection ({ { reverbNode->nodeID, channel }, { specialEqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { specialEqNode->nodeID, channel }, { audioOutputNode->nodeID, channel } });
            }
            break;
        
        case CRIMSON:
            for (int channel = 0; channel < 2; ++channel)
            {
                // order: filter -> EQ -> delay -> reverb -> distortion
                mainProcessor->addConnection ({ { audioInputNode->nodeID, channel }, { filterNode->nodeID, channel } });
                mainProcessor->addConnection ({ { filterNode->nodeID, channel }, { eqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { eqNode->nodeID, channel }, { delayNode->nodeID, channel } });
                mainProcessor->addConnection ({ { delayNode->nodeID, channel }, { reverbNode->nodeID, channel } });
                mainProcessor->addConnection ({ { reverbNode->nodeID, channel }, { distortionNode->nodeID, channel } });
                mainProcessor->addConnection ({ { distortionNode->nodeID, channel }, { specialEqNode->nodeID, channel } });
                mainProcessor->addConnection ({ { specialEqNode->nodeID, channel }, { audioOutputNode->nodeID, channel } });
            }
            break;
    }
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheKnobAudioProcessor();
}
