/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


// how the fark did he figure out this dark hole
struct LookAndFeel : juce::LookAndFeel_V4 {
    void  drawRotarySlider (juce::Graphics&,
                            int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &) override;
    
    
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
};






class RotarySliderWithLabels : public juce::Slider
{
public:
    RotarySliderWithLabels(juce::RangedAudioParameter& rap,const juce::String& unitSuffix) :
     juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
     juce::Slider::TextEntryBoxPosition::NoTextBox),
     param(&rap),
     suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    
    
    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    // custom visuals
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const {return 14;}
    juce::String getDisplayString() const;
    
    
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;  // for example 'Hz'
};


struct ResponseCurveComponent : juce::Component,juce::AudioProcessorParameter::Listener,
juce::Timer {
    ResponseCurveComponent(SimpleEQAudioProcessor&);
    ~ResponseCurveComponent();
    void parameterValueChanged (int parameterIndex, float newValue)  override;

   
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {};
    void timerCallback() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::Atomic<bool> parametersChanged {false};
    SimpleEQAudioProcessor&  audioProcessor;
    MonoChain monoChain;
    juce::Image background;
    
    void updateChain();  // helper , called to update monoChain to match parameters
    
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
 

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;
    
    
    
    RotarySliderWithLabels peakFreqSlider,
    peakGainSlider,
    peakQualitySlider,
    lowCutFreqSlider,
    highCutFreqSlider,
    lowCutSlopeSlider,
    highCutSlopeSlider;
    
    ResponseCurveComponent responseCurveComponent;
    
    
    juce::ToggleButton lowcutBypassButton,highcutBypassButton,peakBypassButton;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    using ButtonAttachment= APVTS::ButtonAttachment;
    
    Attachment peakFreqSliderAttachment,
    peakGainSliderAttachment,
    peakQualitySliderAttachment,
    lowCutFreqSliderAttachment,
    highCutFreqSliderAttachment,
    lowCutSlopeSliderAttachment,
    highCutSlopeSliderAttachment;
    
    ButtonAttachment lowcutBypassButtonAttachment,highcutBypassButtonAttachment,peakBypassButtonAttachment;
    
    std::vector<juce::Component*> getComps();
    
    
   
     // look and feel for the buttoms
    LookAndFeel lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
