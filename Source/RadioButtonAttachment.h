//
//  RadioButtonAttachment.h
//  TheKnob
//
//  Created by Vou Theophanous on 2025-05-23.
//  Copyright © 2025 VOU. All rights reserved.
//

#pragma once
#include <JuceHeader.h>

/*To implement a new attachment type, create a new class which includes an instance of this class as a data member. Your class should pass a function to the constructor of the ParameterAttachment, which will then be called on the message thread when the parameter changes. You can use this function to update the state of the UI control. Your class should also register as a listener of the UI control and respond to changes in the UI element by calling either setValueAsCompleteGesture or beginGesture, setValueAsPartOfGesture and endGesture.

Make sure to call sendInitialUpdate at the end of your new attachment's constructor, so that the UI immediately reflects the state of the parameter.*/

class RadioButtonAttachment : private juce::Button::Listener
{
public:
//     Creates a connection between a plug-in parameter and some radio buttons.
    RadioButtonAttachment (juce::RangedAudioParameter& parameter, juce::Array<juce::Button*>& _buttons, juce::String componentID, int radioGroupID = 0, juce::UndoManager* um = nullptr);
    ~RadioButtonAttachment();
        
    juce::Button* getButton(int index) { return buttons.getUnchecked(index); }
    
    int numButtons() { return buttons.size(); }
    
    void setBounds(int x, int y, int width, int height, int margin);

private:
    void setValue (float newValue);
    float value;

    void buttonClicked (juce::Button* b) override;
    void buttonStateChanged(juce::Button* b) override;

    juce::RangedAudioParameter& storedParameter;
    juce::ParameterAttachment attachment;
    
    juce::Array<juce::Component::SafePointer<juce::Button> > buttons;
    bool ignoreCallbacks = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadioButtonAttachment)
};
