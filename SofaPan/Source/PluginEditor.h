/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "customLookAndFeel.h"
#include "LogoHexData.h"

//==============================================================================
/**
*/
class SofaPanAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Button::Listener, private Timer
{
public:
    SofaPanAudioProcessorEditor (SofaPanAudioProcessor&);
    ~SofaPanAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    void sliderValueChanged(Slider* slider) override;
    void buttonClicked (Button* button) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SofaPanAudioProcessor& processor;
    
    Slider panner;
    TextButton loadSOFAButton;
    ToggleButton bypassButton;
    Label showSOFADimensions_numMeasurements_label;
    Label showSOFADimensions_numMeasurements;
    Label showSOFADimensions_numSamples;
    Label showSOFADimensions_numSamples_label;
    
    CustomLookAndFeel customLookAndFeel;
    
    AudioProcessorParameter* getParameter (const String& paramId);
    float getParameterValue (const String& paramId);
    void setParameterValue (const String& paramId, float value);
    
    void sliderDragStarted(Slider* slider);
    void sliderDragEnded(Slider* slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
