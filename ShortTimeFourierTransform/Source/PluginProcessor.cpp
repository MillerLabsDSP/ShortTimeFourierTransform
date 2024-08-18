/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::StringRef ShortTimeFourierTransformAudioProcessor::BYPASSED = "BYPASSED";

//==============================================================================
ShortTimeFourierTransformAudioProcessor::ShortTimeFourierTransformAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr, "Params", createParams()) {
}

ShortTimeFourierTransformAudioProcessor::~ShortTimeFourierTransformAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ShortTimeFourierTransformAudioProcessor::createParams() 
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("BYPASSED", ParameterVersionHint), "Bypassed", juce::NormalisableRange<bool>(true, false), false));
    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ShortTimeFourierTransformAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShortTimeFourierTransformAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShortTimeFourierTransformAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShortTimeFourierTransformAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ShortTimeFourierTransformAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShortTimeFourierTransformAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShortTimeFourierTransformAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShortTimeFourierTransformAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ShortTimeFourierTransformAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShortTimeFourierTransformAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ShortTimeFourierTransformAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setLatencySamples(fft[0].getLatencyInSamples());
    
    fft[0].reset();
    fft[1].reset();
}

void ShortTimeFourierTransformAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShortTimeFourierTransformAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void ShortTimeFourierTransformAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    bool bypassed = apvts.getRawParameterValue("Bypass")->load();
    
    float* channelL = buffer.getWritePointer(left);
    float* channelR = buffer.getWritePointer(right);
    
    for (int sample = 0; sample < numSamples; ++sample) {
        float sampleL = channelL[sample];
        float sampleR = channelR[sample];
        
        sampleL = fft[left].processSample(sampleL, bypassed);
        sampleR = fft[right].processSample(sampleR, bypassed);
        
        channelL[sample] = sampleL;
        channelR[sample] = sampleR;
        
    }
}

//==============================================================================
bool ShortTimeFourierTransformAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ShortTimeFourierTransformAudioProcessor::createEditor()
{
    return new ShortTimeFourierTransformAudioProcessorEditor (*this);
}

//==============================================================================
void ShortTimeFourierTransformAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ShortTimeFourierTransformAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShortTimeFourierTransformAudioProcessor();
}
