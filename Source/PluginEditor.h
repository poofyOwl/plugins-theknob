//
//  PluginEditor.h
//  TheKnob
//
//  Created by Vou Theophanous on 2025-05-23.
//  Copyright © 2025 VOU. All rights reserved.
//

//==============================================================================

#include "RadioButtonAttachment.h"


enum MODE
{
    VIOLET,
    TEAL,
    CRIMSON
};

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    enum
    {
        windowWidth = 300,
        windowHeight = 200,
        footerHeight = 30,
        knobAreaWidth = 200
    };
    
    enum RadioButtonIds {
        ModeButtons = 1001
    };

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    PluginEditor (juce::AudioProcessor& parent, juce::AudioProcessorValueTreeState& vts)
        : AudioProcessorEditor (parent),
          valueTreeState (vts)
    {
        setSize (windowWidth, windowHeight);
        
        // the knob label
        addAndMakeVisible (knobLabel);
        knobLabel.setFont (juce::Font (juce::FontOptions(16.0f, juce::Font::bold)));
        knobLabel.setText ("The Knob", juce::dontSendNotification);
        knobLabel.setJustificationType (juce::Justification::centred);
        
        // the knob
        addAndMakeVisible (knobSlider);
        knobSlider.setSliderStyle (juce::Slider::Rotary);
        knobSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knobSlider.setPopupDisplayEnabled (false, false, this);
        
        // the mode label
        addAndMakeVisible (modeLabel);
        modeLabel.setFont (juce::Font (juce::FontOptions(16.0f, juce::Font::italic)));
        modeLabel.setText ("The Mode", juce::dontSendNotification);
        modeLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
        
        // the mode buttons
        addAndMakeVisible (button1);
        addAndMakeVisible (button2);
        addAndMakeVisible (button3);
        button1.onClick = [this] { updateButtons(VIOLET); };
        button2.onClick = [this] { updateButtons(TEAL); };
        button3.onClick = [this] { updateButtons(CRIMSON); };
        button1.setRadioGroupId (ModeButtons);
        button2.setRadioGroupId (ModeButtons);
        button3.setRadioGroupId (ModeButtons);
        modeButtons.add(&button1);
        modeButtons.add(&button2);
        modeButtons.add(&button3);
        
        // looks and feels
        buttonLookAndFeel[VIOLET].setColour (juce::Slider::thumbColourId, COLOURS[VIOLET]);
        buttonLookAndFeel[TEAL].setColour (juce::Slider::thumbColourId, COLOURS[TEAL]);
        buttonLookAndFeel[CRIMSON].setColour (juce::Slider::thumbColourId, COLOURS[CRIMSON]);
        
        knobAttachment.reset (new SliderAttachment (valueTreeState, "knob", knobSlider));
        modeAttachment = std::make_unique<RadioButtonAttachment>(*valueTreeState.getParameter("mode"), modeButtons, "mode", ModeButtons);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        
        auto footerArea = area.removeFromBottom(footerHeight);
        auto knobLabelArea = footerArea.removeFromLeft(knobAreaWidth);
        auto modeLabelArea = footerArea;
        
        auto knobArea = area.removeFromLeft(knobAreaWidth);
        auto modeArea = area;
        
        modeArea.removeFromTop(footerHeight);
        auto button1Area = modeArea.removeFromTop(modeArea.getHeight()/3);
        auto button2Area = modeArea.removeFromTop(modeArea.getHeight()/2);
        auto button3Area = modeArea;
        
        knobSlider.setBounds (knobArea);
        knobLabel.setBounds (knobLabelArea);
        modeLabel.setBounds (modeLabelArea);
        button1.setBounds (button1Area);
        button2.setBounds (button2Area);
        button3.setBounds (button3Area);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }
    
    void updateButtons (MODE mode)
    {
        knobLabel.setColour (juce::Label::textColourId, COLOURS[mode]);
        knobSlider.setLookAndFeel(&buttonLookAndFeel[mode]);
    }

private:
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Label knobLabel;
    juce::Slider knobSlider;
    std::unique_ptr<SliderAttachment> knobAttachment;
    
    juce::Label modeLabel;
    juce::Array<juce::Button*> modeButtons;
    juce::ToggleButton button1 { "Violet" };
    juce::ToggleButton button2 { "Teal" };
    juce::ToggleButton button3 { "Crimson" };
    std::unique_ptr<RadioButtonAttachment> modeAttachment;
    
    std::array<juce::LookAndFeel_V4, 3> buttonLookAndFeel;
    
    std::array<juce::Colour, 3> COLOURS = { juce::Colours::violet, juce::Colours::teal, juce::Colours::crimson };
};
