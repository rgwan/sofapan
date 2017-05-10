/*
  ==============================================================================

    PlotHRIRComponent.h
    Created: 4 May 2017 11:19:35pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class PlotHRIRComponent    : public Component, public Button::Listener
{
public:
    PlotHRIRComponent()
    {
        LookAndFeel::setDefaultLookAndFeel(&defaultLookAndFeel);
        IR_Left.resize(0);
        IR_Right.resize(0);
        IR_Left_Zoom.resize(0);
        IR_Right_Zoom.resize(0);
        zoomXFactor = 1.0;
        zoomYFactor = 1.0;
        
        
        zoomXButton1.setButtonText("1x");
        zoomXButton1.setClickingTogglesState(true);
        zoomXButton1.setRadioGroupId(1111);
        zoomXButton1.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton1.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomXButton1.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton1.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton1.setToggleState(true, dontSendNotification);
        zoomXButton1.setLookAndFeel(&defaultLookAndFeel);
        zoomXButton1.addListener(this);
        addAndMakeVisible(zoomXButton1);
        
        zoomXButton2.setButtonText("2x");
        zoomXButton2.setClickingTogglesState(true);
        zoomXButton2.setRadioGroupId(1111);
        zoomXButton2.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton2.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomXButton2.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton2.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton2.setToggleState(false, dontSendNotification);
        zoomXButton2.setLookAndFeel(&defaultLookAndFeel);
        zoomXButton2.addListener(this);
        addAndMakeVisible(zoomXButton2);
        
        zoomXButton3.setButtonText("4x");
        zoomXButton3.setClickingTogglesState(true);
        zoomXButton3.setRadioGroupId(1111);
        zoomXButton3.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton3.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomXButton3.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton3.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton3.setToggleState(false, dontSendNotification);
        zoomXButton3.setLookAndFeel(&defaultLookAndFeel);
        zoomXButton3.addListener(this);
        addAndMakeVisible(zoomXButton3);
        
        zoomYButton1.setButtonText("1x");
        zoomYButton1.setClickingTogglesState(true);
        zoomYButton1.setRadioGroupId(2222);
        zoomYButton1.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton1.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomYButton1.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton1.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton1.setToggleState(true, dontSendNotification);
        zoomYButton1.setLookAndFeel(&defaultLookAndFeel);
        zoomYButton1.addListener(this);
        addAndMakeVisible(zoomYButton1);
        
        zoomYButton2.setButtonText("1.3x");
        zoomYButton2.setClickingTogglesState(true);
        zoomYButton2.setRadioGroupId(2222);
        zoomYButton2.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton2.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomYButton2.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton2.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton2.setToggleState(false, dontSendNotification);
        zoomYButton2.setLookAndFeel(&defaultLookAndFeel);
        zoomYButton2.addListener(this);
        addAndMakeVisible(zoomYButton2);
        
        zoomYButton3.setButtonText("1.8x");
        zoomYButton3.setClickingTogglesState(true);
        zoomYButton3.setRadioGroupId(2222);
        zoomYButton3.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton3.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomYButton3.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton3.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton3.setToggleState(false, dontSendNotification);
        zoomYButton3.setLookAndFeel(&defaultLookAndFeel);
        zoomYButton3.addListener(this);
        addAndMakeVisible(zoomYButton3);
        
        zoomYButton4.setButtonText("2.5x");
        zoomYButton4.setClickingTogglesState(true);
        zoomYButton4.setRadioGroupId(2222);
        zoomYButton4.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton4.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomYButton4.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton4.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton4.setToggleState(false, dontSendNotification);
        zoomYButton4.setLookAndFeel(&defaultLookAndFeel);
        zoomYButton4.addListener(this);
        addAndMakeVisible(zoomYButton4);
        
        zoomYButton5.setButtonText("3.5x");
        zoomYButton5.setClickingTogglesState(true);
        zoomYButton5.setRadioGroupId(2222);
        zoomYButton5.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton5.setColour (TextButton::buttonOnColourId, mainCyan);
        zoomYButton5.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton5.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton5.setToggleState(false, dontSendNotification);
        zoomYButton5.setLookAndFeel(&defaultLookAndFeel);
        zoomYButton5.addListener(this);
        addAndMakeVisible(zoomYButton5);
    
        
    }

    ~PlotHRIRComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);   // clear the background
        
        Rectangle<float> bounds = getLocalBounds().toFloat();
        g.setColour (Colours::grey);
        g.drawRect (bounds, 1);
        g.setColour (Colours::black);
        g.drawRect (plotBox, 1);
        g.setFont(Font(9.f));
        float xPos, yPos;
        Line<float> gridLine;
        float dashes[2] = {3.0, 3.0};
        
        gridLine = Line<float>(plotBox.getX(), plotBox.getCentreY(), plotBox.getRight(), plotBox.getCentreY());
        g.drawDashedLine(gridLine, dashes, 2);
        
        
        if(IR_Left.size() == 0 || IR_Right.size()==0){
            return;
        }
        
        g.drawFittedText("Zoom X:", zoomXLabelBox.toNearestInt(), Justification::centred, 1);
        g.drawFittedText("Zoom Y:", zoomYLabelBox.toNearestInt(), Justification::centred, 1);
//        Rectangle<float> buttonGroupArea = Rectangle<float>(plotBox.getX(),
//                                                            plotBox.getBottom()+2,
//                                                            plotBox.getWidth() / 3,
//                                                            bounds.getBottom() - plotBox.getBottom() - 4);
//        g.fillRect(buttonGroupArea);
        
        /* =================================================================================== */
        
        //Create Clipping Regions
        
        Rectangle<int> noDrawArea = Rectangle<int>(bounds.getX(), plotBox.getBottom(), bounds.getWidth(), bounds.getHeight()-plotBox.getBottom());
        g.excludeClipRegion(noDrawArea);
        
        noDrawArea = bounds.withHeight(plotBox.getY()).toNearestInt();
        g.excludeClipRegion(noDrawArea);
        
        noDrawArea = Rectangle<int>(bounds.getX(), bounds.getY(), plotBox.getX(), bounds.getHeight());
        g.excludeClipRegion(noDrawArea);
        
        noDrawArea = Rectangle<int>(plotBox.getRight(), bounds.getY(), bounds.getWidth() - plotBox.getWidth(), bounds.getHeight());
        g.excludeClipRegion(noDrawArea);
        
        
        
        
        
        Path waveform_l;
        Path waveform_r;
        
        int boxWidth = plotBox.getWidth();
        int numSamples = IR_Left_Zoom.size();
        
        float sampleStep = (float)numSamples / (float)boxWidth;
        
        
        //Left Channel
        const float oversampling = 1.5;
        waveform_l.startNewSubPath(plotBox.getX(), plotBox.getY() + plotBox.getHeight()*0.5*(zoomYFactor * IR_Left_Zoom[0] + 1));
        waveform_r.startNewSubPath(plotBox.getX(), plotBox.getY() + plotBox.getHeight()*0.5*(zoomYFactor * IR_Right_Zoom[0] + 1));
        for(int i = 1; i < boxWidth * oversampling; i++){
            int sample = (int)truncf(float(i)/oversampling * sampleStep);
            if(sample >= numSamples)  sample = numSamples-1;
            xPos = float(i)/oversampling + plotBox.getX();
            yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (zoomYFactor * IR_Left_Zoom[sample] + 1);
            waveform_l.lineTo(xPos, yPos);
            yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (zoomYFactor * IR_Right_Zoom[sample] + 1);
            waveform_r.lineTo(xPos, yPos);

        }
        //Left Channel
        g.setColour(Colour(0xffaaaa00));
        g.strokePath(waveform_l.createPathWithRoundedCorners(1.0), PathStrokeType(1.5));
        //g.strokePath(waveform_l, PathStrokeType(1.5));
        g.setColour(Colour(0xff00aaaa));
        g.strokePath(waveform_r.createPathWithRoundedCorners(1.0), PathStrokeType(1.5));
//        g.setColour(Colour(0x33666666));
//        g.fillPath(spectrogram_l);
        
        

    }
    
    void resized() override
    {
        Rectangle<float> bounds = getLocalBounds().toFloat();
        plotBox = bounds.withTrimmedBottom(20).withTrimmedLeft(20).withTrimmedTop(5).withTrimmedRight(5);
        
        Rectangle<float> buttonXGroupArea = Rectangle<float>(plotBox.getX(),
                                                        plotBox.getBottom()+2,
                                                        plotBox.getWidth() / 3,
                                                        bounds.getBottom() - plotBox.getBottom() - 4);
        Rectangle<float> buttonYGroupArea = buttonXGroupArea.withX(plotBox.getCentreX()).withWidth(plotBox.getWidth()/2);
        
        zoomXLabelBox = buttonXGroupArea.removeFromLeft(buttonXGroupArea.getWidth()/3.5);
        float buttonWidth = buttonXGroupArea.getWidth() / 3.0;
        Rectangle<int> button = buttonXGroupArea.withWidth(buttonWidth).toNearestInt();
        zoomXButton1.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomXButton2.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomXButton3.setBounds(button);
        
        
        zoomYLabelBox = buttonYGroupArea.removeFromLeft(buttonXGroupArea.getWidth()/3.5);
        buttonWidth = buttonYGroupArea.getWidth() / 5.0;
        button = buttonYGroupArea.withWidth(buttonWidth).toNearestInt();
        zoomYButton1.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton2.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton3.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton4.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton5.setBounds(button);
        
        
        
        

    }
    
    /** Size is the length of one IR. The float vector hrir should contain the left AND the right IR, making the vector of length size*2 */
    void drawHRIR(float* _HRIR, int size, int _sampleRate){
        
        sampleRate = _sampleRate;
        IR_Left.resize(size);
        IR_Right.resize(size);
        for(int i = 0; i< size; i++){
            IR_Left[i] = _HRIR[i];
            IR_Right[i] = _HRIR[i + size];
            //printf("Draw: \n%d: %f", i, IR_Left[i]);
        }
        
        //Remove zero-tail
        for(int i = 1; i< size; i++){
            if(IR_Left[size - i] == 0.0 && IR_Right[size - i] == 0.0){
                IR_Left.pop_back();
                IR_Right.pop_back();
            }else{
                break;
            }
        }
        
        IR_Left_Zoom = IR_Left;
        IR_Right_Zoom = IR_Right;
        
        if(zoomXFactor != 1.0){
            int newSize = IR_Left.size()/zoomXFactor;
            IR_Left_Zoom.resize(newSize);
            IR_Right_Zoom.resize(newSize);
            
        }
        
        
    }
    
    void setZoomX(float _zoomFactor){
        
        if(_zoomFactor != zoomXFactor){
            int newSize = IR_Left.size()/_zoomFactor;
            
            if(_zoomFactor < zoomXFactor){
                IR_Left_Zoom = IR_Left;
                IR_Right_Zoom = IR_Right;
            }
            
            IR_Left_Zoom.resize(newSize);
            IR_Right_Zoom.resize(newSize);
            
            zoomXFactor = _zoomFactor;
            
            repaint();
        }
    }
    
    void setZoomY(float _zoomFactor){
        
        if(_zoomFactor != zoomYFactor){

            zoomYFactor = _zoomFactor;
            
            repaint();
        }
    }
    
    void buttonClicked (Button* button) override{
        
        if(button == &zoomXButton1) setZoomX(1.0);
        if(button == &zoomXButton2) setZoomX(2.0);
        if(button == &zoomXButton3) setZoomX(4.0);
        if(button == &zoomYButton1) setZoomY(1.0);
        if(button == &zoomYButton2) setZoomY(1.3);
        if(button == &zoomYButton3) setZoomY(1.8);
        if(button == &zoomYButton4) setZoomY(2.5);
        if(button == &zoomYButton5) setZoomY(3.5);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlotHRIRComponent)

    std::vector<float> IR_Left;
    std::vector<float> IR_Right;
    
    std::vector<float> IR_Left_Zoom;
    std::vector<float> IR_Right_Zoom;
    
    float zoomXFactor;
    float zoomYFactor;
    int sampleRate;
    
    TextButton zoomXButton1;
    TextButton zoomXButton2;
    TextButton zoomXButton3;
    
    TextButton zoomYButton1;
    TextButton zoomYButton2;
    TextButton zoomYButton3;
    TextButton zoomYButton4;
    TextButton zoomYButton5;
                                                        
    Rectangle<float> plotBox;
    Rectangle<float> zoomXLabelBox;
    Rectangle<float> zoomYLabelBox;
    
    const Colour mainCyan = Colour (0xff80cbc4);
    
    LookAndFeel_V4 defaultLookAndFeel;
    
};
