/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SofaPanAudioProcessorEditor::SofaPanAudioProcessorEditor (SofaPanAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
    
    setSize (400, 300);
    
    panner.setSliderStyle(Slider::Rotary);
    panner.setCentrePosition(278, 155);
    panner.setRange(0.0, 359.0, 1.0);
    panner.setTextValueSuffix(" degrees");
    panner.setPopupDisplayEnabled(false, this);
    panner.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner.setRotaryParameters(0, M_PI*2.0, false);
    panner.addListener(this);
    addAndMakeVisible(&panner);
    
    loadSOFAButton.setButtonText ("Load HRTF");
    loadSOFAButton.addListener(this);
    addAndMakeVisible(&loadSOFAButton);
    
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(ToggleButton::textColourId, Colours::white);
    bypassButton.addListener(this);
    addAndMakeVisible(&bypassButton);
    
    showSOFADimensions_numMeasurements_label.setColour(Label::textColourId, Colours::white);
    showSOFADimensions_numMeasurements_label.setText("Number of HRIRs:", NotificationType::sendNotification);
    addAndMakeVisible(&showSOFADimensions_numMeasurements_label);
    showSOFADimensions_numMeasurements.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(&showSOFADimensions_numMeasurements);
    
    showSOFADimensions_numSamples_label.setColour(Label::textColourId, Colours::white);
    showSOFADimensions_numSamples_label.setText("HRIR Size (Samples):", NotificationType::sendNotification);
    addAndMakeVisible(&showSOFADimensions_numSamples_label);
    showSOFADimensions_numSamples.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(&showSOFADimensions_numSamples);
    
    
    startTimer(50);

}

SofaPanAudioProcessorEditor::~SofaPanAudioProcessorEditor()
{
}

//==============================================================================
void SofaPanAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    
    g.setColour (Colours::black);
    g.setFont (15.0f);
    
    
    // Der Versuch einer Einteilung in Quadranten
    g.setColour(Colours::white);
    g.setOpacity(1.0f);
    auto w = static_cast<float>(getWidth());
    auto h = static_cast<float>(getHeight());
    auto lineHorizontal = Line<float>(0, h * 0.5f, w, h * 0.5f);
    auto lineVertical = Line<float>(w * 0.675f, 0.f, w * 0.675f, h);
    float dashes[] = { 3, 3 };
    g.drawDashedLine(lineHorizontal, dashes, 2);
    g.drawDashedLine(lineVertical, dashes, 2);
    
    // Hier wird das HSD-Logo eingebunden
    const Image logo = ImageFileFormat::loadFrom(hsd_logo, hsd_logo_size);
    g.drawImageAt(logo, 0, getHeight()-40, true);

}

void SofaPanAudioProcessorEditor::resized()
{
    panner.setBounds(getWidth() * 0.25,
                     getHeight() * 0.50 - getWidth() * 0.25,
                     getWidth() * 0.5,
                     getWidth() * 0.5);
    
    loadSOFAButton.setBounds(-5.,
                             10.,
                             150.,
                             30.);
    
    bypassButton.setBounds(10.,
                           50.,
                           150.,
                           30.);
    showSOFADimensions_numMeasurements_label.setBounds(10., 100., 150., 30.);
    showSOFADimensions_numMeasurements.setBounds(110., 100., 150., 30.);
    showSOFADimensions_numSamples_label.setBounds(10., 80., 150., 30.);
    showSOFADimensions_numSamples.setBounds(130., 80., 150., 30.);
}

// This timer periodically checks whether any of the filter's parameters have changed...
void SofaPanAudioProcessorEditor::timerCallback() {
    float paramDegreeValue = getParameterValue("pan") * 360;
    panner.setValue(paramDegreeValue, NotificationType::dontSendNotification);
    bypassButton.setToggleState((bool)getParameterValue("bypass"), NotificationType::dontSendNotification);
    
    juce::String numMeasurement_Note = static_cast <String> (processor.metadata_sofafile.numMeasurements);
    showSOFADimensions_numMeasurements.setText(numMeasurement_Note, NotificationType::sendNotification);
    
    juce::String numSamples_Note = static_cast <String> (processor.metadata_sofafile.numSamples);
    showSOFADimensions_numSamples.setText(numSamples_Note, NotificationType::sendNotification);
    
}

void SofaPanAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if(slider == &panner){
        float panNormValue = panner.getValue() / 360;
        setParameterValue("pan", panNormValue);
    }
}

void SofaPanAudioProcessorEditor::sliderDragStarted(Slider* slider)
{
    if (slider == &panner) {
        if (AudioProcessorParameter* param = getParameter("pan")) {
            param->beginChangeGesture();
        }
    }
}

void SofaPanAudioProcessorEditor::sliderDragEnded(Slider* slider)
{
    if (slider == &panner) {
        if (AudioProcessorParameter* param = getParameter("pan")) {
            param->endChangeGesture();
        }
    }
}

AudioProcessorParameter* SofaPanAudioProcessorEditor::getParameter (const String& paramId)
{
    
    const OwnedArray<AudioProcessorParameter>& params = processor.getParameters();
    
    for (int i = 0; i < params.size(); ++i)
    {
        if (AudioProcessorParameterWithID* param = dynamic_cast<AudioProcessorParameterWithID*> (params[i]))
        {
            if (param->paramID == paramId)
                return param;
        }
    }
    
    return nullptr;
}

//==============================================================================
float SofaPanAudioProcessorEditor::getParameterValue (const String& paramId)
{
    if (AudioProcessorParameter* param = getParameter (paramId))
        return param->getValue();
    
    return 0.0f;
}

void SofaPanAudioProcessorEditor::setParameterValue (const String& paramId, float value)
{
    if (AudioProcessorParameter* param = getParameter (paramId))
        param->setValueNotifyingHost(value);
}

void SofaPanAudioProcessorEditor::buttonClicked(Button *button){
    
    if (button == &loadSOFAButton) {
        FileChooser fc ("W&auml;hle HRTF!",
                        File::getCurrentWorkingDirectory(),
                        "*.sofa",
                        true);
        
        if (fc.browseForFileToOpen())
        {
            String chosen = fc.getResult().getFullPathName();
            processor.setSOFAFilePath(chosen);
            processor.initData(chosen);
//            int loadSOFA_status = processor.LoadSOFAFile();
//            if (loadSOFA_status)
//            {
//                processor.ErrorHandling_LoadSOFAFile(loadSOFA_status);
//            }
//            else
//            {
//                AlertWindow::showNativeDialogBox("Binaural Renderer", "SOFA File successfully loaded", false);
//                
//            }
        }
    }
    
    if (button == &bypassButton) {
        bool togglestate = bypassButton.getToggleState();
        setParameterValue("bypass", bypassButton.getToggleState());
    }
}


