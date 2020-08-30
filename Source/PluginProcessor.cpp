/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GGranulaAudioProcessor::GGranulaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        synthesizerState(std::make_shared<SynthesizerState>()),
        synthesizer(synthesizerState)
#endif
{
    const juce::StringArray transpose("-2", "-1", "0", "+1", "+2");
    const juce::StringArray waves("Sine", "Sawtooth");
    
    addParameter(osc_1_transpose = new juce::AudioParameterChoice("osc_1_transpose", "OSC #1 - Transpose", transpose, 2));
    addParameter(osc_1_wave = new juce::AudioParameterChoice("osc_1_wave", "OSC #1 - Waveform", waves, 0));
    
    addParameter(osc_2_transpose = new juce::AudioParameterChoice("osc_2_transpose", "OSC #2 - Transpose", transpose, 2));
    addParameter(osc_2_wave = new juce::AudioParameterChoice("osc_2_wave", "OSC #2 - Waveform", waves, 0));
    
    addParameter (amp_attack = new juce::AudioParameterFloat ("amp_attack",
                                                              "AMP - Attack",
                                                              juce::NormalisableRange<float>(0.0f, 10.0f),
                                                              synthesizerState->getAmpADSR(ADSRStages::ATTACK)));
    addParameter (amp_decay = new juce::AudioParameterFloat ("amp_decay",
                                                             "AMP - Decay",
                                                             juce::NormalisableRange<float>(0.0f, 10.0f),
                                                             synthesizerState->getAmpADSR(ADSRStages::DECAY)));
    addParameter (amp_sustain = new juce::AudioParameterFloat ("amp_sustain",
                                                               "AMP - Sustain",
                                                               juce::NormalisableRange<float>(0.0f, 10.0f),
                                                               synthesizerState->getAmpADSR(ADSRStages::SUSTAIN)));
    addParameter (amp_release = new juce::AudioParameterFloat ("amp_release",
                                                               "AMP - Release",
                                                               juce::NormalisableRange<float>(0.0f, 20.0f),
                                                               synthesizerState->getAmpADSR(ADSRStages::RELEASE)));
}

GGranulaAudioProcessor::~GGranulaAudioProcessor()
{
}

//==============================================================================
const juce::String GGranulaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GGranulaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GGranulaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GGranulaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GGranulaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GGranulaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GGranulaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GGranulaAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GGranulaAudioProcessor::getProgramName (int index)
{
    return {};
}

void GGranulaAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GGranulaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synthesizer.prepare({
        .juce_spec = {
            .sampleRate       = sampleRate,
            .maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock),
            .numChannels      = static_cast<juce::uint32>(getTotalNumOutputChannels())
        }
    });
}

void GGranulaAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    synthesizer.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GGranulaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GGranulaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    synthesizerState->setTranspose(SynthOSC::FIRST_OSC,  osc_1_transpose->getCurrentChoiceName());
    synthesizerState->setTranspose(SynthOSC::SECOND_OSC, osc_2_transpose->getCurrentChoiceName());
    synthesizerState->setWaveType(SynthOSC::FIRST_OSC,  osc_1_wave->getCurrentChoiceName());
    synthesizerState->setWaveType(SynthOSC::SECOND_OSC, osc_2_wave->getCurrentChoiceName());
    synthesizerState->setAmpADSR(ADSRStages::ATTACK,  amp_attack->get());
    synthesizerState->setAmpADSR(ADSRStages::DECAY,   amp_decay->get());
    synthesizerState->setAmpADSR(ADSRStages::SUSTAIN, amp_sustain->get());
    synthesizerState->setAmpADSR(ADSRStages::RELEASE, amp_release->get());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (auto message: midiMessages) {
        if (message.getMessage().isNoteOn())
        {
            synthesizer.noteOn(message.getMessage());
        } else if (message.getMessage().isNoteOff())
        {
            synthesizer.noteOff(message.getMessage());
        }
    }
    dsp::AudioBlock<BufferData> block(buffer);
    dsp::ProcessContextReplacing<BufferData> context(block);
    synthesizer.process({
        .juce_context = context
    });
}

//==============================================================================
bool GGranulaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GGranulaAudioProcessor::createEditor()
{
    return new GGranulaAudioProcessorEditor (*this);
}

//==============================================================================
void GGranulaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GGranulaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GGranulaAudioProcessor();
}
