/*
  ==============================================================================

    SofaMetadataView.h
    Created: 2 May 2017 11:59:49am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SofaMetadataView    : public Component, public Button::Listener
{
public:
    SofaMetadataView()
    {
        closeButton.setButtonText("Close");
        closeButton.addListener(this);
        addAndMakeVisible(closeButton);
        
        
        addAndMakeVisible(textBox);
        textBox.setMultiLine (true);
        textBox.setReadOnly(true);
        textBox.setCaretVisible(false);
        textBox.setColour (TextEditor::textColourId, Colours::black);
        textBox.setColour (TextEditor::backgroundColourId, Colours::white);
        
    }
    
    ~SofaMetadataView()
    {
    }

    void paint (Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (Colours::white);
        g.setFont (14.0f);
        g.drawText ("SofaMetadataView", getLocalBounds(),
                    Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        
        textBox.setBounds(getLocalBounds().reduced(20).withTrimmedBottom(20));
        closeButton.setBounds(getLocalBounds().withSizeKeepingCentre(100, 30).withY(getLocalBounds().getBottom() - 35));
        
        

    }
    
    void setMetadata(Array<String> attributeNames, Array<String> attributeValues){
        
        String textBoxText = String();
        for(int i = 0; i < attributeNames.size(); i ++){
            textBoxText += attributeNames[i];
            textBoxText += ":   ";
            textBoxText += attributeValues[i];
            textBoxText += "\n------------------------------------- \n";
            
        }
        
        textBox.setText(textBoxText);
    }
    
    void buttonClicked (Button* button) override{
        
        if(button == &closeButton)
            setVisible(false);
        
    }

private:
    
    TextEditor textBox;
    TextButton closeButton;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaMetadataView)
};
