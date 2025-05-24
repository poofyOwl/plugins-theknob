//
//  RadioButtonAttachment.cpp
//  TheKnob
//
//  Created by Vou Theophanous on 2025-05-23.
//  Copyright © 2025 VOU. All rights reserved.
//

#include "RadioButtonAttachment.h"


RadioButtonAttachment::RadioButtonAttachment(juce::RangedAudioParameter& param, juce::Array<juce::Button*>& _buttons,
                                             juce::String componentID, int radioGroupID, juce::UndoManager* um) :
storedParameter(param),
attachment(param, [this] (float newValue) { setValue(newValue); }, um)
{
    for (int i=0; i < _buttons.size(); ++i) {
        juce::Button* b = _buttons.getUnchecked (i);
        if (! buttons.contains (b)) {
            if (radioGroupID > 0) {
                b->setRadioGroupId(radioGroupID);
            }
            b->setComponentID(componentID);
            b->setClickingTogglesState(true);
            buttons.add (b);
            b->addListener (this);
        }
    }
    attachment.sendInitialUpdate();
}

RadioButtonAttachment::~RadioButtonAttachment()
{
    for (int i=0; i < buttons.size(); ++i) {
        juce::Button* b = buttons.getUnchecked (i);
        b->removeListener(this);
    }
}

// place all buttons in a row
void RadioButtonAttachment::setBounds(int x, int y, int width, int height, int margin)
{
    for (int i=0; i<buttons.size(); i++) {
        buttons.getUnchecked(i)->setBounds(x+margin*i, y, width, height);
    }
}

// FIXME: automation no worky
void RadioButtonAttachment::buttonClicked (juce::Button* b)
{
    if (ignoreCallbacks)
        return;
    
    for (int i=0; i<buttons.size(); i++) {
        if (b == buttons.getUnchecked(i) && b->getToggleState()) {
            attachment.setValueAsCompleteGesture(i);
        }
    }
}

void RadioButtonAttachment::buttonStateChanged(juce::Button* b)
{
    if (ignoreCallbacks) {
        return;
    }
    
    for (int i=0; i<buttons.size(); i++) {
        if (b == buttons.getUnchecked(i) && b->getToggleState()) {
            attachment.setValueAsCompleteGesture(i);
        }
    }
}

void RadioButtonAttachment::setValue (float newValue)
{
    value = newValue;
    const juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    buttons[value]->setToggleState(true, juce::sendNotification);
}
