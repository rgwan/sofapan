/*
  ==============================================================================

    PlotHRTFComponent.h
    Created: 4 May 2017 10:24:17am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "fftw3.h"
//==============================================================================
/*
*/
class PlotHRTFComponent    : public Component
{
public:
    PlotHRTFComponent()
    {
        mag_l.resize(0);
        mag_r.resize(0);

    }

    ~PlotHRTFComponent()
    {
    }

    void paint (Graphics& g) override
    {
        
        g.fillAll (Colours::white);   // clear the background
        
        Rectangle<float> bounds = getLocalBounds().toFloat();
        Rectangle<float> plotBox = bounds.withTrimmedBottom(20).withTrimmedLeft(20).withTrimmedTop(5).withTrimmedRight(5);
        g.setColour (Colours::grey);
        g.drawRect (bounds, 1);
        
        g.drawRect (plotBox, 1);
        g.setFont(Font(9.f));
        float xPos, yPos;
        Line<float> gridLine;
        float dashes[2] = {3.0, 3.0};
        
        //draw grid X
        float gridLinesX[8] = {50, 100, 200, 500, 1000, 2000, 5000, 10000};
        for(int i = 0; i < 8; i++){
            xPos = log10f(gridLinesX[i]/20.0) * (plotBox.getWidth()/3) + plotBox.getX();
            gridLine = Line<float>(xPos , plotBox.getBottom() , xPos , plotBox.getY());
            g.drawDashedLine(gridLine, dashes, 2);
            g.drawText(String(gridLinesX[i]) , xPos-40 , plotBox.getBottom() + 5 , 80 , 20 , juce::Justification::centredTop);
        }
        g.drawText("20", plotBox.getX()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        g.drawText("20k", plotBox.getRight()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        
        //draw grix Y
        float dBmax = 24.0;
        float dBmin = -60.0;
        float dBrange = dBmax - dBmin;
//        float gridLinesY[9]{ 12 , 0, -12, -24, -36, -48, -60, -72, -84};
//        for(int i = 0; i < 9; i++){
//            yPos = plotBox.getY() - plotBox.getHeight() * (gridLinesY[i]-dBmax) / dBrange;
//            gridLine = Line<float>(plotBox.getX(), yPos , plotBox.getRight() , yPos);
//            if(gridLinesY[i] < dBmax && gridLinesY[i] > dBmin)
//                g.drawDashedLine(gridLine, dashes, 2);
//            if(gridLinesY[i] <= dBmax && gridLinesY[i] >= dBmin)
//                g.drawText(String(gridLinesY[i]), plotBox.getX()-30, yPos-10, 30, 20, juce::Justification::centredRight);
//            
//        }
//        
        g.setColour (Colours::black);
        
        if(mag_l.size() == 0 || mag_r.size()==0){
            return;
        }
        
        
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
        
        
        
        
        
        float frequencyStep = (float)sampleRate / (float)fftSize;
        Path spectrogram_l;
        Path spectrogram_r;
        
        //Left Channel
        spectrogram_l.startNewSubPath(plotBox.getX(), plotBox.getBottom());
        spectrogram_l.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_l[0])-dBmax) / dBrange);
        //RightChannel
        spectrogram_r.startNewSubPath(plotBox.getX(), plotBox.getBottom());
        spectrogram_r.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_r[0])-dBmax) / dBrange);
        
        float oldXPos = 0;
        for(int i = 1; i < mag_l.size(); i++){
            float freq = i * frequencyStep;
            if(freq > 20 && freq < 20000){
                xPos = log10f(freq / 20.0) * (plotBox.getWidth()/3) + plotBox.getX();
                if(xPos - oldXPos > 1.0){
                    //Left Channel
                    yPos = plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_l[i])-dBmax) / dBrange;
                    spectrogram_l.lineTo(xPos, yPos);
                    //Right Channel
                    yPos = plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_r[i])-dBmax) / dBrange;
                    spectrogram_r.lineTo(xPos, yPos);
                    oldXPos = xPos;
            
                }
            }
        }
        //Left Channel
        spectrogram_l.lineTo(plotBox.getRight(), plotBox.getBottom());
        spectrogram_l.closeSubPath();
        g.setColour(Colour(0xffaaaa00));
        g.strokePath(spectrogram_l.createPathWithRoundedCorners(2.0), PathStrokeType(1.5));
        g.setColour(Colour(0x33666666));
        //g.fillPath(spectrogram_l);
        
//        //Right Channel
        spectrogram_r.lineTo(plotBox.getRight(), plotBox.getBottom());
        spectrogram_r.closeSubPath();
        g.setColour(Colour(0xff00aaaa));
        g.strokePath(spectrogram_r.createPathWithRoundedCorners(1.0), PathStrokeType(1.5));
        g.setColour(Colour(0x33666666));
        //g.fillPath(spectrogram_r);
        
        
        
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }
    
    /** Size is the length of one TF. The complex vector hrtf should contain the left AND the right TF, making the vector of length size*2 */
    void drawHRTF(fftwf_complex* hrtf, int size, int _sampleRate){
        
        mag_l.resize(size);
        mag_r.resize(size);
    
        fftSize = (size - 1) * 2;
        float scale = 1.0;// 8.0 / fftSize;
        for(int i = 0; i< size; i++){
            mag_l[i] = scale * (sqrtf(hrtf[i][0] * hrtf[i][0] + hrtf[i][1] * hrtf[i][1]));
            mag_r[i] = scale * (sqrtf(hrtf[i+size][0] * hrtf[i+size][0] + hrtf[i+size][1] * hrtf[i+size][1]));
        }
        
        
        sampleRate = _sampleRate;
        
//        printf("\ndrawHRTF");
//        repaint();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlotHRTFComponent)
    
    std::vector<float> mag_l;
    std::vector<float> mag_r;
    int sampleRate;
    int fftSize;
    
};
