/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings
{
    float peakFreq {0},peakGainInDecibels{0}, peakQ {1.0f};
    float lowCutFreq {0},highCutFreq{0};
    Slope lowCutSlope {Slope_12},highCutSlope{Slope_12};
};

using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter,Filter,CutFilter>;

enum ChainPositions
{
    LowCut,
    Peak,
    HighCut
};



ChainSettings getChainSettings(juce::AudioProcessorValueTreeState&  );



class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout  createParameterLayout() ;
    juce::AudioProcessorValueTreeState apvts {*this, nullptr,"Parameters", createParameterLayout()};
    
private:
    
    void  updateFilters(float sampleRate);
    void  updatePeakFilter(const ChainSettings& cs,float sampleRate);
    void  updateHighCutFilters(const ChainSettings& cs,float sampleRate);
    void  updateLowCutFilters(const ChainSettings& cs,float sampleRate);
    

    
    MonoChain leftChain,rightChain;
    
   
    
    using Coefficients = Filter::CoefficientsPtr;
    
    static void updateCoefficients(Coefficients old, const Coefficients& replacements);
    
    
    // helper
    
    template  <int Index, typename ChainType,typename CoefficientType>
    void update(ChainType& chain,const CoefficientType& cutCoeffs){
        
        updateCoefficients(chain.template get<Index>().coefficients, cutCoeffs[Index]);
        chain.template setBypassed<Index>(false);
    }
    
    // setup cut filteres
    
    template<typename ChainType,typename CoefficientType>
    void updateCutFilter(ChainType& cutFilter, const CoefficientType& cutCoeffs,
                         Slope cutSlope, const float sampleRate) {
   
        
     
        cutFilter.template setBypassed<0>(true);
        cutFilter.template setBypassed<1>(true);
        cutFilter.template setBypassed<2>(true);
        cutFilter.template setBypassed<3>(true);
        
        switch(cutSlope)
        {
             
                
            case Slope_48:
                update<3>(cutFilter,cutCoeffs);
            
                
            case Slope_36:
                update<2>(cutFilter,cutCoeffs);
                
            case Slope_24:
                update<1>(cutFilter,cutCoeffs);
       
            case Slope_12:
                update<0>(cutFilter,cutCoeffs);
 
 
           }
        
        
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
