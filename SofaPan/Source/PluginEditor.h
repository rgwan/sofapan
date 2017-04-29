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
#include "elevationSliderLookAndFeel.h"
#include "LogoHexData.h"
#include "HeadTopHexData.h"
#include "HeadSideHexData.h"
#include "SpeakerHexData.h"



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
    
    Slider panner_az;
    Slider panner_el;
    TextButton loadSOFAButton;
    ToggleButton bypassButton;
    ToggleButton testSwitchButton;
    const String measurementsID =       String("Measurements:         ");
    const String samplesID =            String("Samples:              ");
    const String sofaConventionID =     String("SOFA Convention:      ");
    const String dataTypeID =           String("Data Type:            ");
    const String organizationID =       String("Organization:         ");
    const String listenerShortNameID =  String("Listener Short Name:  ");
    const String roomTypeID =           String("Room Type:            ");
    const String elevationRangID =      String("Elevation Range:      ");
    const String commentID =            String("Comment:              ");
    const String sofaMetadataID = String(measurementsID + "\n" +
                                         samplesID + "\n" +
                                         sofaConventionID + "\n" +
                                         dataTypeID + "\n" +
                                         organizationID + "\n" +
                                         listenerShortNameID + "\n" +
                                         roomTypeID + "\n" +
                                         elevationRangID + "\n" +
                                         commentID);
    
    String sofaMetadataValue;
    
    CustomLookAndFeel customLookAndFeel;
    ElevationSliderLookAndFeel elSliderLookAndFeel;
    
    AudioProcessorParameter* getParameter (const String& paramId);
    float getParameterValue (const String& paramId);
    void setParameterValue (const String& paramId, float value);
    
    void sliderDragStarted(Slider* slider);
    void sliderDragEnded(Slider* slider);

    Image speakerImage;
    Image headTopImage;
    Image headSideImage;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
