/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>




class PluginEditor : public juce::AudioProcessorEditor
{
public:
    enum
    {
        headerHeight = 50,
        footerHeight = 30
    };

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    PluginEditor (juce::AudioProcessor& parent, juce::AudioProcessorValueTreeState& vts)
        : AudioProcessorEditor (parent),
          valueTreeState (vts)
    {
        setSize (200, 200);
        
        knobSlider.setSliderStyle (juce::Slider::Rotary);
        knobSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knobSlider.setPopupDisplayEnabled (true, true, this);
        addAndMakeVisible (knobSlider);
        
        knobAttachment.reset (new SliderAttachment (valueTreeState, "knob", knobSlider));
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop (headerHeight);
        area.removeFromBottom(footerHeight);
        knobSlider.setBounds (area);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        getLookAndFeel().setColour (juce::Slider::thumbColourId, juce::Colours::red);
        g.setFont (15.0f);
        g.drawFittedText ("The Knob", 0, getHeight() - footerHeight, getWidth(), footerHeight, juce::Justification::centred, 1);
    }

private:
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Label knobLabel;
    juce::Slider knobSlider;
    std::unique_ptr<SliderAttachment> knobAttachment;
};





//==============================================================================
/**
*/
class TheKnobAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TheKnobAudioProcessor();
    ~TheKnobAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
//    juce::AudioProcessorEditor* createEditor() override;
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
    
//    void knobValueChanged (float value);

private:
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* knobParameter  = nullptr;
    
    float previousGain;
    std::atomic<float>* gainParameter  = nullptr;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheKnobAudioProcessor)
};
