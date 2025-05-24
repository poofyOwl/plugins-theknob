/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RadioButtonAttachment.h"
#include "PluginEditor.h"
#include "FXProcessors.h"


//==============================================================================
/**
*/
class TheKnobAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
    using Node = juce::AudioProcessorGraph::Node;
    
    //==============================================================================
    TheKnobAudioProcessor();
    ~TheKnobAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    void releaseResources() override
    {
        mainProcessor->releaseResources();
    }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
            return false;
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;
        return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
    }

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override          { return new PluginEditor (*this, parameters); }
    bool hasEditor() const override                              { return true; }

    //==============================================================================
    const juce::String getName() const override                  { return "The Knob"; }
    bool acceptsMidi() const override                            { return false; }
    bool producesMidi() const override                           { return false; }
    bool isMidiEffect() const override                           { return false; }
    double getTailLengthSeconds() const override                 { return 0; }

    //==============================================================================
    int getNumPrograms() override                                { return 1; }
    int getCurrentProgram() override                             { return 0; }
    void setCurrentProgram (int) override                        {}
    const juce::String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const juce::String&) override   {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    void initialiseGraph();
    void connectGraph();
    
    //==============================================================================

    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;

    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;

    Node::Ptr filterNode;
    Node::Ptr eqNode;
    Node::Ptr reverbNode;
    Node::Ptr delayNode;
    Node::Ptr distortionNode;
    
    //==============================================================================
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* knobParameter  = nullptr;
    std::atomic<float>* modeParameter  = nullptr;
    
    float prevMode;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheKnobAudioProcessor)
};
