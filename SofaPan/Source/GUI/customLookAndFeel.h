/*
  ==============================================================================

    customLookAndFeel.h
    Created: 14 Dec 2016 3:25:21pm
    Author:  David Bau

  ==============================================================================
*/

#ifndef CUSTOMLOOKANDFEEL_H_INCLUDED
#define CUSTOMLOOKANDFEEL_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class CustomLookAndFeel : public LookAndFeel_V3
{
public:
    //==============================================================================
    CustomLookAndFeel()
    {
        setColour (ResizableWindow::backgroundColourId, windowBackgroundColour);
        setColour (TextButton::buttonOnColourId, brightButtonColour);
        setColour (TextButton::buttonColourId, disabledButtonColour);
    }
    
    //==============================================================================
    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& /*backgroundColour*/,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        ignoreUnused (isMouseOverButton);
        
        const int width = button.getWidth();
        const int height = button.getHeight();
        
        Rectangle<float> buttonRect (0, 0, width, height);
        buttonRect.reduce (haloRadius, 0.0f);
        
        if (isButtonDown)
            g.setColour (brightButtonColour.withAlpha (0.7f));
            else if (! button.isEnabled())
                g.setColour (disabledButtonColour);
                else
                    g.setColour (brightButtonColour);
                    
                    g.fillRoundedRectangle (buttonRect, 5.0f);
                    }
    
    //==============================================================================
    void drawButtonText (Graphics& g,
                         TextButton& button,
                         bool isMouseOverButton,
                         bool isButtonDown) override
    {
        ignoreUnused (isMouseOverButton, isButtonDown);
        
        Font font (getTextButtonFont (button, button.getHeight()));
        g.setFont (font);
        
        if (button.isEnabled())
            g.setColour (Colours::white);
            else
                g.setColour (backgroundColour);
                
                g.drawFittedText (button.getButtonText(), 0, 1,
                                  button.getWidth(),
                                  button.getHeight(),
                                  Justification::centred, 2);
                }
    
    
    
    //==============================================================================
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle style, Slider& slider) override
    {
        
        // printf("x: %d, \n y: %d \n width: %d \n heigth: %d \n sliderPos: %f \n minSliderPos: %f \n msxSliderPos: %f ", x, y, width, height, sliderPos, minSliderPos, maxSliderPos);
        
        ignoreUnused (style, minSliderPos, maxSliderPos);
        
        if(style == Slider::LinearHorizontal){
            Rectangle<int> r = Rectangle<int> (x + haloRadius, y, width - (haloRadius * 2), height);
            Rectangle<int> backgroundBar = r.withSizeKeepingCentre(r.getWidth(), 2);
            
            
            
            
            sliderPos = (sliderPos - minSliderPos) / static_cast<float> (width);
            
            int knobPos = static_cast<int> (sliderPos * r.getWidth());
            
            g.setColour (sliderActivePart);
            g.fillRect (backgroundBar.removeFromLeft (knobPos));
            
            g.setColour (sliderInactivePart);
            g.fillRect (backgroundBar);
            
            if (slider.isMouseOverOrDragging())
            {
                Rectangle<int> haloBounds = r.withTrimmedLeft (knobPos - haloRadius)
                .withWidth (haloRadius*2)
                .withSizeKeepingCentre(haloRadius, r.getHeight());
                
                g.setColour (sliderActivePart.withAlpha (0.5f));
                g.fillRoundedRectangle (haloBounds.toFloat(), 3.f);
            }
            
            const int knobRadius = slider.isMouseOverOrDragging() ? knobActiveRadius : knobInActiveRadius;
            Rectangle<int> knobBounds = r.withTrimmedLeft (knobPos - knobRadius)
            .withWidth (knobRadius*2)
            .withSizeKeepingCentre(knobRadius, r.getHeight()*0.7);
            
            //g.setColour (sliderActivePart);
            g.setColour(Colour(0xFF444444));
            g.fillRoundedRectangle (knobBounds.toFloat(), 2.f);
            
            g.setColour (Colour(0xFFcccccc));
            
            //g.drawRoundedRectangle(r.toFloat().withSizeKeepingCentre(r.getWidth()+haloRadius*2, r.getHeight()), 4.0f, 2.0f);
            
            g.setColour(Colour(0x33DDDDDD));
            //g.fillRoundedRectangle(r.toFloat().withSizeKeepingCentre(r.getWidth()+haloRadius*2, r.getHeight()), 4.0f);
            
        }
        
        if(style == Slider::LinearVertical){
            Rectangle<int> r = Rectangle<int> (x , y + haloRadius, width,  height- (haloRadius * 2));
            //printf("\n +++++++ WIDTH: %d", width);
            Rectangle<int> backgroundBar = r.withSizeKeepingCentre(2, r.getHeight());
            
            //            g.setColour (Colour(0xcccccccc));
            //            g.fillRect(r);
            
            sliderPos = (minSliderPos - sliderPos) / static_cast<float> (height);
            
            int knobPos = static_cast<int> (sliderPos * r.getHeight());
            
            g.setColour (sliderActivePart);
            g.fillRect (backgroundBar.removeFromBottom (knobPos));
            
            g.setColour (sliderInactivePart);
            g.fillRect (backgroundBar);
            
            if (slider.isMouseOverOrDragging())
            {
                Rectangle<int> haloBounds = r.withTrimmedTop (r.getHeight() - knobPos - haloRadius)
                .withHeight (haloRadius*2)
                .withSizeKeepingCentre(r.getWidth(), haloRadius);
                
                g.setColour (sliderActivePart.withAlpha (0.5f));
                g.fillRoundedRectangle (haloBounds.toFloat(), 3.f);
            }
            
            const int knobRadius = slider.isMouseOverOrDragging() ? knobActiveRadius : knobInActiveRadius;
            Rectangle<int> knobBounds =r.withTrimmedTop (r.getHeight() - knobPos - knobRadius)
            .withHeight(knobRadius*2)
            .withSizeKeepingCentre(r.getWidth()*0.7, knobRadius);
            
            g.setColour (Colour(0xFF444444));
            g.fillRoundedRectangle (knobBounds.toFloat(), 2.f);
        }
        
        
    }
    //==============================================================================
    
    void drawRotarySlider  	(Graphics & g, int x, int y, int width, int height,
                             float sliderPosProportional, float rotaryStartAngle,
                             float rotaryEndAngle, Slider& slider) override
    {
        
        
        //assure that the knob is always the size of the smallest edge
        if(width > height){
            width = height;
        }
        
        Rectangle<int> r = Rectangle<int>(x, y, width, width);
        
        
        
        const int borderlineThickness = 4;
        const int innerRadius = r.getWidth() * 0.5 - 10;
        const float thumbSize = r.getWidth() * 0.2;
        const float thumbThickness = thumbSize * 0.5;
        const float markerSize = thumbSize * 0.7;
        const float markerThickness = borderlineThickness * 0.5;
        //
        //printf("sliderPos: %f  startAngle: %f  endAngle: %f \n", sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
        
        
        
        //Outer Circle (size = full rectangle, non-adaptive thickness)
        g.setColour(sliderInactivePart);
        const int t_2 = borderlineThickness * 0.5;
        g.drawEllipse(r.getX() + t_2,
                      r.getY() + t_2,
                      r.getWidth() - 2*t_2,
                      r.getWidth() - 2*t_2,
                      borderlineThickness);
        
        //Inner Circle (size = always 10px smaller than outer circle)
        Rectangle<int> innerCircle = r.withSizeKeepingCentre(innerRadius*2, innerRadius*2);
        g.setColour(Colour(knobBackgroundColour));
        g.fillEllipse(innerCircle.toFloat());
        
        AffineTransform rotate;
        float angle;
        
        
        
        
//        //Active Circle (size = full rectangle, non-adaptive thickness)
//        g.setColour(sliderActivePart);
        angle = sliderPosProportional * (rotaryEndAngle-rotaryStartAngle) + rotaryStartAngle;
//        float activeCircleThickness = borderlineThickness * 0.7;
//        PathStrokeType activeEllipseFormat = PathStrokeType(activeCircleThickness, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::rounded);
//        Path activeEllipse = Path();
//        float w2 = r.getWidth() * 0.5;
//        float radius = w2 - borderlineThickness - 0.5*activeCircleThickness;
//        float alpha = rotaryStartAngle;
//        activeEllipse.startNewSubPath(w2 + sin(alpha)*radius, w2 - cos(alpha)*radius);
//        
//        activeEllipse.addArc(r.getX() + borderlineThickness + 0.5 * activeCircleThickness,
//                             r.getY() + borderlineThickness + 0.5 * activeCircleThickness,
//                             r.getWidth() - 2 * borderlineThickness - activeCircleThickness,
//                             r.getWidth() - 2 * borderlineThickness - activeCircleThickness,
//                             rotaryStartAngle,
//                             angle);
//        
//        g.strokePath(activeEllipse, activeEllipseFormat);
        
        
        
        
        
        
        //set colour for the markers
       // g.setColour(sliderInactivePart);
        //
        //        //Creating the little markers
        //        Rectangle<int> littleMarker = r.withSizeKeepingCentre(markerThickness * 0.5, 10).translated(0, -(r.getWidth() * 0.5 - 5.0));
        //        float markerAngle;
        //        for (int i = 1; i < 10; i++){
        //            markerAngle =  (float)i * 0.1 * (rotaryEndAngle-rotaryStartAngle) + rotaryStartAngle;
        //            rotate = AffineTransform::rotation(markerAngle, r.getCentreX(), r.getCentreY());
        //            g.addTransform(rotate);
        //            g.fillRect(littleMarker);
        //            rotate = AffineTransform::rotation(-markerAngle, r.getCentreX(), r.getCentreY());
        //            g.addTransform(rotate);
        //        }
        
//        //Creating the marker
//        Rectangle<int> marker = r.withSizeKeepingCentre(markerThickness, markerSize).translated(0, -(r.getWidth() - markerSize )/1.7);
//        
//        //rotate context to the start-marker & draw it
//        marker.translate(-(thumbThickness * 0.5 + markerThickness * 0.5), 0);
//        rotate = AffineTransform::rotation(rotaryStartAngle, r.getCentreX(), r.getCentreY());
//        g.addTransform(rotate);
//        g.fillRect(marker);
//        
//        //rotate further to the end-marker & draw it
//        marker.translate(thumbThickness + markerThickness, 0);
//        rotate = AffineTransform::rotation(rotaryEndAngle-rotaryStartAngle, r.getCentreX(), r.getCentreY());
//        g.addTransform(rotate);
//        g.fillRect(marker);
//        
//        //rotate to initial angle again
//        rotate = AffineTransform::rotation(-(rotaryEndAngle), r.getCentreX(), r.getCentreY());
//        g.addTransform(rotate);
//        
//        
        //Creating the Thumb
        Rectangle<int> thumb;
        thumb = r.withSizeKeepingCentre(thumbThickness, thumbSize - borderlineThickness).translated(0, -(r.getWidth() - thumbSize)/2.0);
        
        g.setColour(sliderActivePart);
        
        //rotate to knob position (knobPos*range + startAngle)
        rotate = AffineTransform::rotation(angle, r.getCentreX(), r.getCentreY());
        g.addTransform(rotate);
        g.setColour(sliderActivePart);
        g.fillRoundedRectangle(thumb.toFloat(), thumbThickness * 0.25);
        
        //inverse rotation
        rotate = AffineTransform::rotation(-angle, r.getCentreX(), r.getCentreY());
        g.addTransform(rotate);
        
        
    }
    
    //==============================================================================
    Font getTextButtonFont (TextButton& button, int buttonHeight) override
    {
        return LookAndFeel_V3::getTextButtonFont (button, buttonHeight).withHeight (buttonFontSize);
    }
    
    Font getLabelFont (Label& label) override
    {
        return LookAndFeel_V3::getLabelFont (label).withHeight (labelFontSize);
    }
    
    //==============================================================================
    const int labelFontSize = 12;
    const int buttonFontSize = 15;
    
    //=============================================================================
    const int knobActiveRadius = 12;
    const int knobInActiveRadius = 8;
    const int haloRadius = 18;
    
    //==============================================================================
    const Colour windowBackgroundColour = Colour (0xff262328);
    const Colour backgroundColour = Colour (0xff4d4d4d);
    const Colour brightButtonColour = Colour (0xff80cbc4);
    const Colour disabledButtonColour = Colour (0xffe4e4e4);
    const Colour sliderInactivePart = Colour (0xff545d62); //benutzt für rotary->außenring und slider->inaktiverpart
    const Colour sliderActivePart = Colour (0xff80cbc4);
    const Colour knobBackgroundColour = Colour(0xffcccccc);
};






#endif  // CUSTOMLOOKANDFEEL_H_INCLUDED