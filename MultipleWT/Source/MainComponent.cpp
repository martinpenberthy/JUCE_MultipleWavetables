#include "MainComponent.h"
/*
 This code was written to create a standalone app that generates wavetables
 for the four basic waveshapes (sine, saw, triangle and square). This program
 also generates some other strange wavetables with very interesting timbres
 that change significantly based on the frequency. A sine wave is used as the
 base waveform which is modified based on the wavetable sample index.
 */

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

  // specify the number of input and output channels that we want to open
          setAudioChannels (2, 2);
  
  /*SLIDERS*/
  freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
  freqSlider.setRange(10.0, 1000.0, 1.0);
  freqSlider.setTextValueSuffix("Hz");
  freqSlider.addListener(this);
  freqSlider.setValue(90.0);
  addAndMakeVisible(freqSlider);
  freqLabel.setText("Frequency", juce::dontSendNotification);
  freqLabel.attachToComponent(&freqSlider, true);
 
  ampSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
  ampSlider.setRange(0.0, 1.0);
  ampSlider.addListener(this);
  ampSlider.setValue(0.0);
  addAndMakeVisible(ampSlider);
  ampLabel.setText("Amplitude", juce::dontSendNotification);
  ampLabel.attachToComponent(&ampSlider, true);
  
  selectSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  selectSlider.setRange(1, 9, 1);
  selectSlider.addListener(this);
  selectSlider.setValue(1);
  selectSlider.setColour(juce::ColourSelector::backgroundColourId, juce::Colour(10,30,100));
  addAndMakeVisible(selectSlider);
  selectLabel.setText("Waveform", juce::dontSendNotification);
  selectLabel.attachToComponent(&selectSlider, true);
  
  waveformLabel.setText("SINE", juce::dontSendNotification);
  addAndMakeVisible(waveformLabel);
  
  addAndMakeVisible(visualiser);
  //visualiser.setRepaintRate(32);
  
  cpuUsageLabel.setText ("CPU Usage", juce::dontSendNotification);
  cpuUsageText.setJustificationType (juce::Justification::right);
  addAndMakeVisible (cpuUsageLabel);
  addAndMakeVisible (cpuUsageText);
  startTimer (50);
  
  

}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void MainComponent::sliderValueChanged (juce::Slider *slider)
{
  //Check whether the slider is the freq or amp slider and set the vars
  if(slider == &freqSlider)
    currentFreq = freqSlider.getValue();
  else if(slider == &ampSlider)
    amplitude = ampSlider.getValue() / 2;
  else if(slider == &selectSlider)
  {
    waveformSelect = selectSlider.getValue();
    if(waveformSelect == 1)
      waveformLabel.setText("SINE", juce::dontSendNotification);
    else if(waveformSelect == 2)
      waveformLabel.setText("TRIANGLE", juce::dontSendNotification);
    else if(waveformSelect == 3)
      waveformLabel.setText("SQUARE", juce::dontSendNotification);
    else if(waveformSelect == 4)
      waveformLabel.setText("SINE * TRIANGLE", juce::dontSendNotification);
    else if(waveformSelect == 5)
      waveformLabel.setText("SINE - TRIANGLE", juce::dontSendNotification);
    else if(waveformSelect == 6)
      waveformLabel.setText("GRIND", juce::dontSendNotification);
    else if(waveformSelect == 7)
      waveformLabel.setText("SINE,%3 = 0", juce::dontSendNotification);
    else if(waveformSelect == 8)
      waveformLabel.setText("SINE, %7 = 0", juce::dontSendNotification);
    else if(waveformSelect == 9)
      waveformLabel.setText("SAW", juce::dontSendNotification);
  }
}


void MainComponent::updateFreq(const double &bufferSize)
{
  //If the target freq is different than the current, calculate
  //increment for the frequency
  if(targetFreq != currentFreq)
  {
    const double freqIncrement = (targetFreq - currentFreq) / bufferSize;
    increment = (currentFreq + freqIncrement) * wtSize / currentSampleRate;
    phase = fmod((phase + increment), wtSize);
    
  }else
  {
    increment = currentFreq * wtSize / currentSampleRate;
    phase = fmod((phase + increment), wtSize);
  }
}


//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
  //
  
  currentFreq = freqSlider.getValue();
  targetFreq = currentFreq;
  amplitude = ampSlider.getValue();
  phase = 0;
  wtSize = 1024;
  currentSampleRate = sampleRate;
  waveformSelect = 1;
  
  visualiser.setBufferSize(wtSize);
  
  double triIncrement = (1.0f / (wtSize / 4.0f));
  double runningTotal = 0.0f;
  int i = 0;
  
  double sawIncrement = 2.0f / (double)wtSize;
  
  
  /*SINE WAVE*/
  for(int k = 0; k < wtSize; k++)
  {
    waveTableSine.insert(k, sin(2.0 * juce::MathConstants<double>::pi * k / wtSize));
  }
  
  /*TRIANGLE WAVE*/
  //Calc first quarter of the wavetable
  for(i = i; i < wtSize / 4; i++)
  {
    waveTableTriangle.insert(i, runningTotal);
    runningTotal += triIncrement;
  }
  
  //Calc second quarter of the wavetable
  for(i = i; i < wtSize / 2; i++)
  {
    waveTableTriangle.insert(i, runningTotal);
    runningTotal -= triIncrement;
  }
  
  //Calc third quarter of the wavetable
  for(i = i; i < (wtSize / 2) + (wtSize / 4); i++)
  {
    waveTableTriangle.insert(i, runningTotal);
    runningTotal -= triIncrement;
  }
  
  //Calc fourth quarter of the wavetable
  for(i = i; i < wtSize; i++)
  {
    waveTableTriangle.insert(i, runningTotal);
    runningTotal += triIncrement;
  }
  
  /*SQUARE WAVE*/
  for(int j = 0; j < wtSize; j++)
  {
    if(j < wtSize / 2)
      waveTableSquare.insert(j, 1.0f);
    else
      waveTableSquare.insert(j, -1.0f);
  }
  
  /*GRIND*/
  for(int j = 0; j < wtSize; j++)
  {
    if(j % 3)
      waveTableGrind.insert(j, randomGen.nextFloat());
    else
      waveTableGrind.insert(j, sin(2.0 * juce::MathConstants<double>::pi * j / wtSize));
  }
  
  /* SINE, %3 = 0*/
  for(int j = 0; j < wtSize; j++)
  {
    if(j % 3)
      waveTableSinMod3.insert(j, 0.0f);
    else
      waveTableSinMod3.insert(j, sin(2.0 * juce::MathConstants<double>::pi * j / wtSize));
  }
  
  /* SINE, %7 = 0*/
  for(int j = 0; j < wtSize; j++)
  {
    if(j % 7)
      waveTableSinMod7.insert(j, 0.0f);
    else
      waveTableSinMod7.insert(j, sin(2.0 * juce::MathConstants<double>::pi * j / wtSize));
  }
  
  runningTotal = -1.0f;
  /*SAW*/
  for(int j = 0; j < wtSize; j++)
  {
    waveTableSaw.insert(j, runningTotal);
    runningTotal += sawIncrement;
  }
  
  
  visualiser.clear();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
  bufferToFill.clearActiveBufferRegion();
  
  float* const leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
  float* const rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    
  const double bufferSize = bufferToFill.numSamples;
  
  switch(waveformSelect)
  {
    //SINE WAVE
    case 1:
       for(int sample = 0; sample < bufferSize; ++sample)
       {
         leftChannel[sample] = waveTableSine[(int)phase] * amplitude;
         rightChannel[sample] = waveTableSine[(int)phase] * amplitude;
         
         updateFreq(bufferSize);
       }
       break;
    //TRIANGLE WAVE
    case 2:
       for(int sample = 0; sample < bufferSize; ++sample)
       {
         leftChannel[sample] = waveTableTriangle[(int)phase] * amplitude;
         rightChannel[sample] = waveTableTriangle[(int)phase] * amplitude;
         updateFreq(bufferSize);
       }
       break;
    //SQUARE WAVE
    case 3:
       for(int sample = 0; sample < bufferSize; ++sample)
       {
         leftChannel[sample] = waveTableSquare[(int)phase] * (amplitude / 2.5f);
         rightChannel[sample] = waveTableSquare[(int)phase] * (amplitude / 2.5f);
         updateFreq(bufferSize);
       }
       break;
    //SINE * TRIANGLE
    case 4:
       for(int sample = 0; sample < bufferSize; ++sample)
       {
         leftChannel[sample] = ((waveTableSine[(int)phase]) * (waveTableTriangle[(int)phase])) * amplitude;
         rightChannel[sample] = ((waveTableSine[(int)phase]) * (waveTableTriangle[(int)phase])) * amplitude;
         updateFreq(bufferSize);
       }
       break;
      
    //SINE - TRIANGLE
    case 5:
      for(int sample = 0; sample < bufferSize; ++sample)
      {
        leftChannel[sample] = ((waveTableSine[(int)phase]) - (waveTableTriangle[(int)phase])) * (amplitude * 2);
        rightChannel[sample] = ((waveTableSine[(int)phase]) - (waveTableTriangle[(int)phase])) * (amplitude * 2);
        
        if(leftChannel[sample] > 1.0f)
          leftChannel[sample] = fmod(leftChannel[sample], 1.0f) * .5f;
        if(rightChannel[sample] > 1.0f)
          rightChannel[sample] = fmod(rightChannel[sample], 1.0f) * .5f;
        
        updateFreq(bufferSize);
      }
      
      //GRIND
    case 6:
      for(int sample = 0; sample < bufferSize; ++sample)
      {
        leftChannel[sample] = waveTableGrind[(int)phase] * amplitude;
        rightChannel[sample] = waveTableGrind[(int)phase] * amplitude;
        updateFreq(bufferSize);
      }
      break;
      
      //SINE, if(j % 3) sample = 0;
    case 7:
      for(int sample = 0; sample < bufferSize; ++sample)
      {
        leftChannel[sample] = waveTableSinMod3[(int)phase] * amplitude;
        rightChannel[sample] = waveTableSinMod3[(int)phase] * amplitude;
        updateFreq(bufferSize);
      }
      break;
      
      //SINE, if(j % 7) sample = 0;
    case 8:
      for(int sample = 0; sample < bufferSize; ++sample)
      {
        leftChannel[sample] = waveTableSinMod7[(int)phase] * amplitude;
        rightChannel[sample] = waveTableSinMod7[(int)phase] * amplitude;
        updateFreq(bufferSize);
      }
      break;
      //SAW
    case 9:
      for(int sample = 0; sample < bufferSize; ++sample)
      {
        leftChannel[sample] = waveTableSaw[(int)phase] * amplitude;
        rightChannel[sample] = waveTableSaw[(int)phase] * amplitude;
        updateFreq(bufferSize);
      }
      break;
  }
  
  
  visualiser.pushBuffer(bufferToFill);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
  
    
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
  const int labelSpace = 100;
  freqSlider.setBounds(labelSpace, 25, getWidth() - 100, 20);
  ampSlider.setBounds(labelSpace, 50, getWidth() - 100, 50);
  selectSlider.setBounds(labelSpace, 100, 200, 200);
  
  visualiser.setBounds(labelSpace, 300, getWidth() - 200, 200);
  
  waveformLabel.setBounds (60, 220, 120, 20);
  
  cpuUsageLabel.setBounds (10, 10, getWidth() - 20, 20);
  cpuUsageText.setBounds (10, 10, getWidth() - 20, 20);
}
