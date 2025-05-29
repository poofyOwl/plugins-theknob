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

const std::array<juce::Colour, 3> COLOURS =
{
    juce::Colours::violet,
    juce::Colours::teal,
    juce::Colours::crimson
};

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MyLookAndFeel(int mode)
    {
        colour = COLOURS[mode];
        setColour (juce::Slider::thumbColourId, colour);
    }
    
    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        layout.sliderBounds = slider.getLocalBounds();
        layout.textBoxBounds = slider.getLocalBounds().withSizeKeepingCentre (100, 100);
        return layout;
    }
    
    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll (label.findColour (juce::Label::backgroundColourId));

        const juce::Font font (juce::FontOptions(33));

        g.setColour (colour.withMultipliedAlpha (0.5f));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, juce::Justification::centred, 1, 1.0f);

        g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (0.0f));

        g.drawRect (label.getLocalBounds());
    }
    
private:
    juce::Colour colour;

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
        knobSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 50);
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
    
    std::array<MyLookAndFeel, 3> buttonLookAndFeel = {MyLookAndFeel(VIOLET), MyLookAndFeel(TEAL), MyLookAndFeel(CRIMSON)};
};
