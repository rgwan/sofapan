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

#include "sofaPanLookAndFeel.h"
#include "azimuthSliderLookAndFeel.h"
#include "elevationSliderLookAndFeel.h"
#include "LogoHexData.h"
#include "HeadTopHexData.h"
#include "HeadSideHexData.h"
#include "SpeakerHexData.h"
#include "SofaMetaDataView.h"
#include "PlotHRTFComponent.h"
#include "PlotHRIRComponent.h"


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
    Slider panner_dist;
    TextButton loadSOFAButton;
    TextButton showSOFAMetadataButton;
    ToggleButton bypassButton;
    ToggleButton testSwitchButton;
    const String measurementsID =       String("Measurements:         ");
    const String samplesID =            String("Samples:              ");
    const String sofaConventionID =     String("SOFA Convention:      ");
    const String dataTypeID =           String("Data Type:            ");
    const String listenerShortNameID =  String("Listener Short Name:  ");
    const String elevationRangID =      String("Elevation Range:      ");
    const String distanceRangeID =      String("Distance Range:      ");
    const String sofaMetadataID = String(listenerShortNameID + "\n" +
                                         measurementsID + "\n" +
                                         samplesID + "\n" +
                                         sofaConventionID + "\n" +
                                         dataTypeID + "\n" +
                                         elevationRangID + "\n" +
                                         distanceRangeID);
    
    String sofaMetadataValue;
    
    SofaPanLookAndFeel sofaPanLookAndFeel;
    ElevationSliderLookAndFeel elSliderLookAndFeel;
    AzimuthSliderLookAndFeel azSliderLookAndFeel;
    
    AudioProcessorParameter* getParameter (const String& paramId);
    float getParameterValue (const String& paramId);
    void setParameterValue (const String& paramId, float value);
    
    void sliderDragStarted(Slider* slider);
    void sliderDragEnded(Slider* slider);

    Image speakerImage;
    Image headTopImage;
    Image headSideImage;
    
    SofaMetadataView metadataView;
    PlotHRTFComponent plotHRTFView;
    PlotHRIRComponent plotHRIRView;
    int counter;
    
    float lastElevationValue;
    float lastAzimuthValue;
    float lastDistanceValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
