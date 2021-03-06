/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


void LookAndFeel::drawRotarySlider(juce::Graphics & g, int x,int y, int width, int height, float sliderPosProportional,
                                   float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider){
    
    using namespace juce;
    auto bounds = Rectangle<float> {static_cast<float>(x),static_cast<float>(y),
                                  static_cast<float>(width),static_cast<float>(height)};
    
    auto enabled = slider.isEnabled();
    
    
    
    g.setColour( enabled ? Colour(20u,20u,195u) : Colours::darkgrey);
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colour(255u,255u,255u) : Colours::lightgrey);
    g.drawEllipse(bounds, 1.0f);
    
    // cast the slider to a rotary slider with labels. If that doesnt work something is wrong, so just draw nuttin
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)){
        
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        // the pointer
        r.setLeft(center.getX()-2.0f);
        r.setRight(center.getX()+2.0f);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()-rswl->getTextHeight()*1.25);
        p.addRoundedRectangle(r,2.0f);
        
        
        jassert(rotaryStartAngle<rotaryEndAngle);
        
        auto sliderAngle = jmap(sliderPosProportional,0.0f,1.0f,rotaryStartAngle,rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotation(sliderAngle, center.getX(), center.getY()));
        
        g.fillPath(p); // draw the dial indicator
        
        // draw the current value if enabled
        
        if(enabled) {
        
            g.setFont(rswl->getTextHeight());
            auto text = rswl->getDisplayString();
            auto strWidth = g.getCurrentFont().getStringWidth(text);

            r.setSize(strWidth+4,rswl->getTextHeight()+4);
            r.setCentre(bounds.getCentre());
            g.setColour(Colours::black);
            g.fillRect(r);
            g.setColour(Colours::white);
            g.drawFittedText(text,r.toNearestInt(),juce::Justification::centred,1);
        }
        
    }
    
    
}



void LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& tb,
                                    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    
    // draw custom toggle button.
    using namespace juce;
    // start a path
    Path powerButton;
    auto bounds = tb.getLocalBounds();
    // we want the size to fit to the smallest dimension.
    auto size = jmin(bounds.getWidth(),bounds.getHeight()) - 6;  // a bit shrinking
    
    // making square at center.
    auto r = bounds.withSizeKeepingCentre(size,size).toFloat();
    
    float ang = 30.f; //degrees
    size -= 6;
    
    powerButton.addCentredArc(r.getCentreX(), r.getCentreY(),size*0.5, size*0.5, 0.f,degreesToRadians(ang),degreesToRadians(360.f-ang),true);
    
    // line from center to top of box r
    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre());
    
    PathStrokeType pst(2,PathStrokeType::curved);
    
    auto color = tb.getToggleState() ? Colours::dimgrey : Colours::green;
    g.setColour(color);
    g.strokePath(powerButton, pst);
    
    // outer ellipse
     g.drawEllipse(r, 2);
    
  
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    auto startAng  = degreesToRadians(180.0f+45.0f);
    auto endAng =degreesToRadians(180.0f-  45.0f) +MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    
    // debugging boxes
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    // DRAW SLIDER
    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(),range.getStart(),range.getEnd(), 0.0, 1.0), startAng, endAng, *this);
    
    // DRAW Min and MAx values
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth()*0.5f;
    
    g.setColour(Colour(0u,172u,30u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for (auto i =0; i< numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos,0.f,1.f,startAng,endAng);
        
        // get point just outside circle
        auto c = center.getPointOnCircumference(radius+getTextHeight()*0.5f, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str),getTextHeight());
        r.setCentre(c);
        // shift down point
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str,r.toNearestInt(),juce::Justification::centred,1);
        
        
        
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
   auto bounds = getLocalBounds();
   auto size = juce::jmin(bounds.getWidth(),bounds.getHeight());
    size -= getTextHeight()*2;
    juce::Rectangle<int> r;
    r.setSize(size,size);
    r.setCentre(bounds.getCentreX(),0);
    r.setY(2);
    
    return r;
}


juce::String RotarySliderWithLabels::getDisplayString() const{
    // is this a choice param?
    if (auto *choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK{false};
    if (auto *floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float value = getValue();
        if(value >= 1000.0f){
            value /= 1000.0f;
            addK=true;
        }
        str= juce::String(value,(addK ? 2:0));
    } else {
        jassertfalse;  // should not happen since we dont have any other types.
    }
    if(suffix.isNotEmpty()) {
        str << " " ;
        if(addK)
            str <<"k";
        str << suffix;
        
    }
    return str;
}
//=====================================================================

ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor &p): audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for(auto param : params){
        param->addListener(this);
    }
    
    updateChain();
    startTimerHz(60);
  
    
}

ResponseCurveComponent::~ResponseCurveComponent() {
    
    const auto& params = audioProcessor.getParameters();
    for(auto param : params){
        param->removeListener(this);
    }
    
}


void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
    
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback(){
    if(parametersChanged.compareAndSetBool(false, true)) {
        //update GUI filter
        updateChain();
        //redraw
        repaint();
    }
}

void ResponseCurveComponent::updateChain(){
    //update GUI filter chain
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    
    monoChain.setBypassed<Peak>(chainSettings.peakBypassed);
    monoChain.setBypassed<LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<HighCut>(chainSettings.highCutBypassed);
    
    auto sampleRate = audioProcessor.getSampleRate();
    auto peakCoef = makePeakFilter(chainSettings, sampleRate);
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients,peakCoef);
    
    auto lowCutCoef = makeLowCutFilter(chainSettings, sampleRate);
    auto highCutCoef = makeHighCutFilter(chainSettings, sampleRate);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoef, chainSettings.highCutSlope,sampleRate);
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoef, chainSettings.lowCutSlope,sampleRate);

   
    
}

void ResponseCurveComponent::paint(juce::Graphics &g){
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    auto responseArea = getAnalysisArea();
    
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags; // width of the region
    
    mags.resize(w);
    
    for(int i=0; i<w; ++i)
    {
        double mag =1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        
       
        if(!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        
        if(!monoChain.isBypassed<HighCut>()) {
            if(!highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        if(!monoChain.isBypassed<LowCut>()) {
            if(!lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
            if(!lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        mags[i] = Decibels::gainToDecibels(mag);
    }
    
    
    // draw background image
    
    g.drawImage(background,  getLocalBounds().toFloat());
    // Draw curve
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    
    auto map = [outputMin,outputMax](double input){
        return jmap(input,-24.0,24.0,outputMin,outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(),map(mags.front()));
    
    for(size_t i=1;i< mags.size();++i) {
        responseCurve.lineTo(responseArea.getX()+i,map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    // rectangle around render area, which is slightly bigger then response area.
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
    

    
}

void ResponseCurveComponent::resized(){
    using namespace juce;
    
    background = Image(Image::PixelFormat::RGB,getWidth(),getHeight(),true);
    
    Graphics g{background};
    // Commented out frequencies were colliding.
    Array<float> freqs{
        20,/* 30,40,*/ 50,100,200,
        /* 300,400, */
        500,1000,
        2000,/* 3000,4000, */ 5000,10000,
        20000
    };
    
    // cache the analysis area which is isnide the full render area.
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    
    // cache the x's
    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f,20.0f,20000.0f);
        xs.add(left + width*normX);
    }
    
    
    g.setColour(Colours::darkgrey);
    for (auto x : xs)
    {
         
        g.drawVerticalLine(x, top,bottom);
    }
    
    
    Array<float> gain{
        -24,-12,0,12,24
    };

    for (auto gdB : gain)
    {
        auto y = jmap(gdB,-24.f,24.f,static_cast<float>(bottom),static_cast<float>(top));
       
        g.setColour( gdB == 0.0 ? Colour(0u,172u,1u) : Colours::darkgrey);
        g.drawHorizontalLine(y,left, right);
    }
    
    // Labels
    g.setColour(Colours::lightgrey);
    int fontHeight{10};
    g.setFont(fontHeight);
    
    for(int i=0; i< freqs.size();++i)
    {
        auto f =freqs[i];
        auto x = xs[i];
        
        bool addK = false;
        String str;
        if (f >= 1000.0f) {
            addK= true;
            f /= 1000.f;
        }
        
        str << f;
        if(addK)
            str << "k";
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        // bound the text
        Rectangle<int> r;
        r.setSize(textWidth,fontHeight);
        r.setCentre(x,0);
        r.setY(1);
       
        g.drawFittedText(str,r,juce::Justification::centred,1);
        
    }
    // gain labels
    for (auto gdB : gain)
    {
        auto y = jmap(gdB,-24.f,24.f,static_cast<float>(bottom),static_cast<float>(top));
        
        String str;
        if (gdB > 0 )
            str << "+";
        str << gdB; // << "dB";
        
        // a hack
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        
        // bound the text
        Rectangle<int> r;
        r.setSize(textWidth,fontHeight);
        r.setCentre(0,y);
        r.setX(getWidth()-textWidth);
        
        // drawtxt
        g.setColour( gdB == 0.0 ? Colour(0u,172u,1u) : Colours::lightgrey);
        g.drawFittedText(str,r,juce::Justification::centred,1);
        
        // We also need to put in scale for spectrum analizer
        
        str.clear();
        str << (gdB-24.f);
        // move rectangle to left edge
        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        //new size
        r.setSize(textWidth,fontHeight);
        
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str,r,juce::Justification::centred,1);
    }
    
    
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea(){
    
    auto renderArea = getLocalBounds();

    renderArea.removeFromTop(12);
    renderArea.removeFromLeft(20);
    renderArea.removeFromRight(20);
    renderArea.removeFromBottom(1);
    
    return renderArea;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea(){
    auto bounds=getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    
    return bounds;
}


//==============================================================================

//===  SimpleEQAudioProcessorEditor

SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Q"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"),"dB/Oct"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"),"dB/Oct"),
responseCurveComponent(audioProcessor),
peakFreqSliderAttachment(audioProcessor.apvts,"Peak Freq",peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts,"Peak Gain",peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts,"Peak Q",peakQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts,"LowCut Freq",lowCutFreqSlider),
highCutFreqSliderAttachment(audioProcessor.apvts,"HighCut Freq",highCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.apvts,"LowCut Slope",lowCutSlopeSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts,"HighCut Slope",highCutSlopeSlider),
lowcutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowcutBypassButton),
highcutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highcutBypassButton),
peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    // Tedius labels here... could use a helper function or two i think
    peakFreqSlider.labels.add({0.f,"20 Hz"});
    peakFreqSlider.labels.add({1.f,"20 kHz"});
    
    peakQualitySlider.labels.add({0.f,"0.1"});
    peakQualitySlider.labels.add({1.f,"10.0"});
    
    peakGainSlider.labels.add({0.f,"-24dB"});
    peakGainSlider.labels.add({1.f,"+24dB"});
        
    lowCutFreqSlider.labels.add({0.f,"20 Hz"});
    lowCutFreqSlider.labels.add({1.f,"20 kHz"});
    
    highCutFreqSlider.labels.add({0.f,"20 Hz"});
    highCutFreqSlider.labels.add({1.f,"20 kHz"});
    
    lowCutSlopeSlider.labels.add({0.f,"12"});
    lowCutSlopeSlider.labels.add({1.f,"48"});
    
    highCutSlopeSlider.labels.add({0.f,"12"});
    highCutSlopeSlider.labels.add({1.f,"48"});
    
    
    
    for (auto* comp: getComps())
    {
        addAndMakeVisible(comp);
    }
    
   
    // set lnf for the buttons to our custom lnf
    peakBypassButton.setLookAndFeel(&lnf);
    highcutBypassButton.setLookAndFeel(&lnf);
    lowcutBypassButton.setLookAndFeel(&lnf);

    // set up disabling knobs when the band is bypassed
    
    // safePTR is to make sure nothing bad happens if we get closed. (It automatically becomes
    // null if the component is deleted)
    
    auto safePtr = juce::Component::SafePointer<SimpleEQAudioProcessorEditor>(this);
    
    peakBypassButton.onClick = [safePtr]()
    {
        if(auto *comp = safePtr.getComponent()) {
            auto bypassed = comp->peakBypassButton.getToggleState();
            comp->peakFreqSlider.setEnabled(!bypassed);
            comp->peakGainSlider.setEnabled(!bypassed);
            comp->peakQualitySlider.setEnabled(!bypassed);
            
        }
    };
    
    highcutBypassButton.onClick = [safePtr]()
    {
        if(auto *comp = safePtr.getComponent()) {
            auto bypassed = comp->highcutBypassButton.getToggleState();
            comp->highCutFreqSlider.setEnabled(!bypassed);
            comp->highCutSlopeSlider.setEnabled(!bypassed);
        }
    };
    
    lowcutBypassButton.onClick = [safePtr]()
    {
        if(auto *comp = safePtr.getComponent()) {
            auto bypassed = comp->lowcutBypassButton.getToggleState();
            comp->lowCutFreqSlider.setEnabled(!bypassed);
            comp->lowCutSlopeSlider.setEnabled(!bypassed);
        }
    };
    
    setSize (600, 480);
}

// this is not optional
SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    
    // clear out look and feels when destroying editor.
    peakBypassButton.setLookAndFeel(nullptr);
    highcutBypassButton.setLookAndFeel(nullptr);
    lowcutBypassButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
 
  g.fillAll (juce::Colours::black);
 
                                  
    
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    
    // ratio of the top to the remainder. THe live constant is for experimetning
    // float hRatio = JUCE_LIVE_CONSTANT(33)/100.f;
    
    float hRatio = 0.25f;
    
    // reserve area for spectral display
   auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);
    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5); // make some space between the spectral part and the knobs.
    // Filter controls
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth()*0.50);
    
    lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    highcutBypassButton.setBounds(highCutArea.removeFromTop(25));
    
    
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.50));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.50));
    highCutSlopeSlider.setBounds(highCutArea);
                    
    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.50));
    peakQualitySlider.setBounds(bounds);
    
    
    
}

 

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent,
        &lowcutBypassButton,
        &peakBypassButton,
        &highcutBypassButton
        
    };
}
