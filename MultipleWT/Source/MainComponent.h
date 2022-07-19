#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

//Implementation of the visualizer
class Visualiser : public juce::AudioVisualiserComponent
{
public:
    Visualiser() : AudioVisualiserComponent (2)
    {
        setBufferSize(128);
        setSamplesPerBlock(16);
        setColours(juce::Colours::black, juce::Colours::white);
    }
};


class MainComponent  : public juce::AudioAppComponent,
public juce::Slider::Listener, public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void sliderValueChanged (juce::Slider *slider) override;
    void updateFreq(const double &bufferSize);
  
    void timerCallback() override
    {
        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText (juce::String (cpu, 6) + " %", juce::dontSendNotification);
    }
    

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
  //Sliders and label objects
  juce::Slider freqSlider, ampSlider, selectSlider;
  juce::Label freqLabel, ampLabel, selectLabel, waveformLabel;
  
  juce::Label cpuUsageLabel;
  juce::Label cpuUsageText;
  
  //Create managed arrays for the wavetables
  juce::Array<double> waveTableSquare;
  juce::Array<double> waveTableTriangle;
  juce::Array<double> waveTableSine;
  juce::Array<double> waveTableSaw;
  juce::Array<double> waveTableGrind;
  juce::Array<double> waveTableSinMod3;
  juce::Array<double> waveTableSinMod7;
  
  
  double wtSize;
  double currentFreq, targetFreq;
  double phase;
  double increment;
  double amplitude;
  double currentSampleRate;
  double bufferSize;
  int waveformSelect;
  
  //Random number gen object
  juce::Random randomGen;
  
  Visualiser visualiser;
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

