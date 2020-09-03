/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>

//==============================================================================
using BufferData = float;

//==============================================================================
enum ADSRStages
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

//==============================================================================
enum VoiceWaveType
{
    SIN,
    SAW
};

//==============================================================================
enum VoiceTranspose
{
    MINUS_TWO_OCTAVES,
    MINUS_ONE_OCTAVE,
    NO_TRANSPOSE,
    PLUS_ONE_OCTAVE,
    PLUS_TWO_OCTAVES
};

//==============================================================================
enum SynthOSC
{
    FIRST_OSC,
    SECOND_OSC
};

//==============================================================================
class SynthesizerState
{
public:
    using ADSRParam = float;
    using Frequency = float;
    using QFactor   = float;
    
    //==============================================================================
    using TransposeHandler    = std::function<void(VoiceTranspose)>;
    using WaveTypeHandler     = std::function<void(VoiceWaveType)>;
    using ADSRHandler         = std::function<void(ADSRParam)>;
    using FilterCutoffhandler = std::function<void(Frequency)>;
    using FilterQHandler      = std::function<void(QFactor)>;
    
    //==============================================================================
    struct SynthesizerInitialState
    {
        SynthesizerInitialState() noexcept {}
        VoiceTranspose osc_1_transpose = VoiceTranspose::NO_TRANSPOSE;
        VoiceTranspose osc_2_transpose = VoiceTranspose::NO_TRANSPOSE;
        VoiceWaveType  osc_1_wave_type = VoiceWaveType::SIN;
        VoiceWaveType  osc_2_wave_type = VoiceWaveType::SAW;
        ADSRParam      amp_attack      = 0.1f;
        ADSRParam      amp_decay       = 0.1f;
        ADSRParam      amp_sustain     = 0.8f;
        ADSRParam      amp_release     = 0.5f;
        Frequency      filter_cutoff   = 100.0f;
        QFactor        filter_q        = 1.0f;
        unsigned int   num_of_voices   = 4;
    };
    
    SynthesizerState(SynthesizerInitialState initial_state = SynthesizerInitialState()):
        osc_1_transpose(initial_state.osc_1_transpose),
        osc_2_transpose(initial_state.osc_2_transpose),
        osc_1_wave_type(initial_state.osc_1_wave_type),
        osc_2_wave_type(initial_state.osc_2_wave_type),
        amp_attack(initial_state.amp_attack),
        amp_decay(initial_state.amp_decay),
        amp_sustain(initial_state.amp_sustain),
        amp_release(initial_state.amp_release),
        filter_cutoff(initial_state.filter_cutoff),
        filter_q(initial_state.filter_q),
        num_of_voices(initial_state.num_of_voices)
    {}
    ~SynthesizerState()
    {
        unsubscribeAllHandlers();
    }
    
    //==============================================================================
    void unsubscribeAllHandlers()
    {
        getTransposeHandlers(SynthOSC::FIRST_OSC).clear();
        getTransposeHandlers(SynthOSC::SECOND_OSC).clear();
        getWaveTypeHandlers(SynthOSC::FIRST_OSC).clear();
        getWaveTypeHandlers(SynthOSC::SECOND_OSC).clear();
        getAmpADSRHandlers(ADSRStages::ATTACK).clear();
        getAmpADSRHandlers(ADSRStages::DECAY).clear();
        getAmpADSRHandlers(ADSRStages::SUSTAIN).clear();
        getAmpADSRHandlers(ADSRStages::RELEASE).clear();
        filter_cutoff_handlers.clear();
        filter_q_handlers.clear();
    }
    
    //==============================================================================
    VoiceTranspose getTranspose(SynthOSC osc_name)
    {
        switch (osc_name)
        {
            case (SynthOSC::FIRST_OSC)  : return osc_1_transpose;
            case (SynthOSC::SECOND_OSC) : return osc_2_transpose;
        }
    }
    void setTranspose(SynthOSC osc_name, VoiceTranspose transpose)
    {
        switch(osc_name)
        {
            case(SynthOSC::FIRST_OSC):
                if (transpose == osc_1_transpose) { return; } // no change
                osc_1_transpose = transpose;
                break;
            case(SynthOSC::SECOND_OSC):
                if (transpose == osc_2_transpose) { return; } // no change
                osc_2_transpose = transpose;
                break;
        }
        for (auto handler : getTransposeHandlers(osc_name))
        {
            try {
                handler(transpose);
            } catch (...) {}
        }
    }
    void setTranspose(SynthOSC osc_name, const juce::String transpose)
    {
        setTranspose(osc_name, toVoiceTranspose(transpose));
    }
    void onTransposeChnge(SynthOSC osc_name, TransposeHandler handler)
    {
        getTransposeHandlers(osc_name).push_back(handler);
    }
    VoiceTranspose toVoiceTranspose(const juce::String& value)
    {
        if (value == "-2")
        {
            return VoiceTranspose::MINUS_TWO_OCTAVES;
        } else if (value == "-1")
        {
            return VoiceTranspose::MINUS_ONE_OCTAVE;
        } else if (value == "0")
        {
            return VoiceTranspose::NO_TRANSPOSE;
        } else if (value == "+1")
        {
            return VoiceTranspose::PLUS_ONE_OCTAVE;
        } else if (value == "+2")
        {
            return VoiceTranspose::PLUS_TWO_OCTAVES;
        } else
        {
            return VoiceTranspose::NO_TRANSPOSE;
        }
    }
    
    //==============================================================================
    VoiceWaveType getWaveType(SynthOSC osc_name)
    {
        switch (osc_name)
        {
            case (SynthOSC::FIRST_OSC)  : return osc_1_wave_type;
            case (SynthOSC::SECOND_OSC) : return osc_2_wave_type;
        }
    }
    void setWaveType(SynthOSC osc_name, VoiceWaveType wave_type)
    {
        switch(osc_name)
        {
            case(SynthOSC::FIRST_OSC):
                if (wave_type == osc_1_wave_type) { return; } // no change
                osc_1_wave_type = wave_type;
                break;
            case(SynthOSC::SECOND_OSC):
                if (wave_type == osc_2_wave_type) { return; } // no change
                osc_2_wave_type = wave_type;
                break;
        }
        for (auto handler : getWaveTypeHandlers(osc_name))
        {
            try {
                handler(wave_type);
            } catch (...) {}
        }
    }
    void setWaveType(SynthOSC osc_name, const juce::String wave_type)
    {
        setWaveType(osc_name, toVoiceWaveType(wave_type));
    }
    void onWaveTypeChange(SynthOSC osc_name, WaveTypeHandler handler)
    {
        getWaveTypeHandlers(osc_name).push_back(handler);
    }
    VoiceWaveType toVoiceWaveType(const juce::String& value)
    {
        if (
            value == "Sine" |
            value == "Sin"  |
            value == "sine" |
            value == "sin"
            )
        {
            return VoiceWaveType::SIN;
        } else if (
            value == "Sawtooth" |
            value == "Saw"  |
            value == "sawtooth" |
            value == "saw"
            )
        {
            return VoiceWaveType::SAW;
        } else
        {
            return VoiceWaveType::SIN;
        }
    }
    
    //==============================================================================
    ADSRParam getAmpADSR(ADSRStages adsr_stage)
    {
        switch(adsr_stage)
        {
            case (ADSRStages::ATTACK)  : return amp_attack;
            case (ADSRStages::DECAY)   : return amp_decay;
            case (ADSRStages::SUSTAIN) : return amp_sustain;
            case (ADSRStages::RELEASE) : return amp_release;
        }
    }
    void setAmpADSR(ADSRStages adsr_stage, ADSRParam value)
    {
        switch(adsr_stage)
        {
            case (ADSRStages::ATTACK):
                if (value == amp_attack) return; // no-change
                amp_attack = value;
                break;
            case (ADSRStages::DECAY):
                if (value == amp_decay) return; // no-change
                amp_decay = value;
                break;
            case (ADSRStages::SUSTAIN):
                if (value == amp_sustain) return; // no-change
                amp_sustain = value;
                break;
            case (ADSRStages::RELEASE):
                if (value == amp_release) return; // no-change
                amp_release = value;
                break;
        }
        for (auto handler : getAmpADSRHandlers(adsr_stage))
        {
            try
            {
                handler(value);
            } catch(...) {}
        }
    }
    void onAmpADSRChange(ADSRStages adsr_stage, ADSRHandler handler)
    {
        getAmpADSRHandlers(adsr_stage).push_back(handler);
    }
    
    //==============================================================================
    Frequency getFilterCutoff()
    {
        return filter_cutoff;
    }
    void setFilterCutoff(Frequency frequency)
    {
        if (filter_cutoff == frequency) return; // no-change
        filter_cutoff = frequency;
        for (auto handler : filter_cutoff_handlers)
        {
            try
            {
                handler(frequency);
            } catch (...) {}
        }
    }
    void onFilterCutoffChange(FilterCutoffhandler handler)
    {
        filter_cutoff_handlers.push_back(handler);
    }
    
    //==============================================================================
    QFactor getFilterQ()
    {
        return filter_q;
    }
    void setFilterQ(QFactor q)
    {
        if (filter_q == q) return; // no-change
        filter_q = q;
        for (auto handler : filter_q_handlers)
        {
            try
            {
                handler(q);
            } catch (...) {}
        }
    }
    void onFilterQChange(FilterQHandler handler)
    {
        filter_q_handlers.push_back(handler);
    }
    
private:
    using TransposeHandlers = std::list<TransposeHandler>;
    using TransposeListners = std::map<SynthOSC, TransposeHandlers>;
    VoiceTranspose    osc_1_transpose = VoiceTranspose::NO_TRANSPOSE;
    VoiceTranspose    osc_2_transpose = VoiceTranspose::NO_TRANSPOSE;
    TransposeListners transpose_listeners;
    TransposeHandlers& getTransposeHandlers(SynthOSC osc_name)
    {
        if (transpose_listeners.count(osc_name) == 0)
        {
            transpose_listeners.insert(TransposeListners::value_type(osc_name, TransposeHandlers()));
        }
        return transpose_listeners[osc_name];
    }
    
    //==============================================================================
    using WaveTypeHandlers  = std::list<WaveTypeHandler>;
    using WaveTypeListners  = std::map<SynthOSC, WaveTypeHandlers>;
    VoiceWaveType    osc_1_wave_type = VoiceWaveType::SIN;
    VoiceWaveType    osc_2_wave_type = VoiceWaveType::SIN;
    WaveTypeListners wave_type_listeners;
    WaveTypeHandlers& getWaveTypeHandlers(SynthOSC osc_name)
    {
        if (wave_type_listeners.count(osc_name) == 0)
        {
            wave_type_listeners.insert(WaveTypeListners::value_type(osc_name, WaveTypeHandlers()));
        }
        return wave_type_listeners[osc_name];
    }
    
    //==============================================================================
    using ADSRHandlers    = std::list<ADSRHandler>;
    using AmpADSRListners = std::map<ADSRStages, ADSRHandlers>;
    ADSRParam       amp_attack  = 0.1f;
    ADSRParam       amp_decay   = 0.1f;
    ADSRParam       amp_sustain = 1.0f;
    ADSRParam       amp_release = 1.0f;
    AmpADSRListners ampADSRListeners;
    ADSRHandlers& getAmpADSRHandlers(ADSRStages adsr_stage)
    {
        if (ampADSRListeners.count(adsr_stage) == 0)
        {
            ampADSRListeners.insert(AmpADSRListners::value_type(adsr_stage, ADSRHandlers()));
        }
        return ampADSRListeners[adsr_stage];
    }
    
    //==============================================================================
    using FilterCutoffHandlers = std::list<FilterCutoffhandler>;
    Frequency            filter_cutoff = 1000.0f;
    FilterCutoffHandlers filter_cutoff_handlers;
    
    //==============================================================================
    using FilterQHandlers = std::list<FilterQHandler>;
    QFactor         filter_q = 1.0f;
    FilterQHandlers filter_q_handlers;
    
    //==============================================================================
    unsigned int   num_of_voices   = 4;
};

//==============================================================================
struct IAudioProcessorConfig
{
    juce::dsp::ProcessSpec juce_spec;
};
struct IAudioProcessContext
{
    juce::dsp::ProcessContextReplacing<BufferData> juce_context;
};

//==============================================================================
class IAudioProcessor
{
public:
    //==============================================================================
    using NumChannels   = int;
    using NumSamples    = int;
    using SynthStatePtr = std::shared_ptr<SynthesizerState>;
    
    //==============================================================================
    SynthStatePtr synthesizer_state_ptr;
    
    //==============================================================================
    IAudioProcessor (SynthStatePtr state_ptr): synthesizer_state_ptr(state_ptr) {};
    virtual ~IAudioProcessor () = default;

    //==============================================================================
    virtual void prepare (const IAudioProcessorConfig&) noexcept {};
    virtual void process (const IAudioProcessContext&) noexcept {};
    virtual void reset () noexcept {};
    
    //==============================================================================
    static dsp::AudioBlock<BufferData> DuplicateAudioBlock (dsp::AudioBlock<BufferData>& src)
    {
        juce::AudioBuffer<BufferData> buffer(src.getNumChannels(), src.getNumSamples()); // init empty buffer with same size
        dsp::AudioBlock<BufferData> block(buffer); // init audio block from buffer
        block.copyFrom(src); // copy src buffer to it
        return block;
    }
    SynthStatePtr getSynthState()
    {
        return synthesizer_state_ptr;
    }
};

//==============================================================================
class ADSRProcessor : public juce::dsp::ProcessorBase
{
public:
    //==============================================================================
    virtual void prepare (const juce::dsp::ProcessSpec &spec) noexcept override
    {
        getADSR().setSampleRate(spec.sampleRate);
        updateParameters();
    }
    virtual void process (const juce::dsp::ProcessContextReplacing<BufferData> &context) noexcept override
    {
        for (int channel = 0; channel < context.getOutputBlock().getNumChannels(); ++channel)
        {
            auto* output = context.getOutputBlock().getChannelPointer(channel);
            for (auto sample = 0; sample < context.getOutputBlock().getNumSamples(); ++sample)
            {
                output[sample] = getADSR().getNextSample() * output[sample];
            }
        }
    }
    virtual void reset () noexcept override
    {
        getADSR().reset();
    }
    
    //==============================================================================
    void noteOn ()
    {
        getADSR().reset();
        getADSR().noteOn();
    }
    void noteOff ()
    {
        getADSR().noteOff();
    }
    bool isActive ()
    {
        return (getADSR().isActive());
    }
    void setParameter(ADSRStages stage, float value)
    {
        switch(stage)
        {
            case (ADSRStages::ATTACK)  : _attack  = value; break;
            case (ADSRStages::DECAY)   : _decay   = value; break;
            case (ADSRStages::SUSTAIN) : _sustain = value; break;
            case (ADSRStages::RELEASE) : _release = value; break;
        }
        updateParameters();
    }
    
private:
    //==============================================================================
    juce::ADSR _juce_adsr;
    float _attack  = 0.1f;
    float _decay   = 0.1f;
    float _sustain = 1.0f;
    float _release = 6.9f;
    
    //==============================================================================
    juce::ADSR& getADSR () noexcept
    {
        return _juce_adsr;
    }
    void updateParameters ()
    {
        const auto current_adsr_parameters = getADSR().getParameters();
        if (
            current_adsr_parameters.attack  != _attack |
            current_adsr_parameters.decay   != _decay |
            current_adsr_parameters.sustain != _sustain |
            current_adsr_parameters.release != _release
            )
        {
            getADSR().setParameters({
                .attack  = _attack,
                .decay   = _decay,
                .sustain = _sustain,
                .release = _release,
            });
        }
    }
};

//==============================================================================
class Voice : public IAudioProcessor
{
public:
    Voice(IAudioProcessor::SynthStatePtr state_ptr): IAudioProcessor(state_ptr)
    {
        using namespace std::placeholders;
        getSynthState()->onAmpADSRChange(ADSRStages::ATTACK,  std::bind(&Voice::onAttackChange,  this, _1));
        getSynthState()->onAmpADSRChange(ADSRStages::DECAY,   std::bind(&Voice::onDecayChange,   this, _1));
        getSynthState()->onAmpADSRChange(ADSRStages::SUSTAIN, std::bind(&Voice::onSustainChange, this, _1));
        getSynthState()->onAmpADSRChange(ADSRStages::RELEASE, std::bind(&Voice::onReleaseChange, this, _1));
        
        setFrequency(calculateFrequency(440));
        setGain(calculateGain(0.0f));
    };
    
    //==============================================================================
    void prepare (const IAudioProcessorConfig &spec) noexcept
    {
        _processorChain.prepare(spec.juce_spec);
    }
    void process (const IAudioProcessContext &context) noexcept
    {
        _processorChain.process(context.juce_context);
    }
    void reset () noexcept
    {
        _processorChain.reset();
    }
    
    //==============================================================================
    void noteOn (const juce::MidiMessage& midiMessage)
    {
        setCurrentNote(midiMessage.getNoteNumber());
        getADSR().noteOn();
        setFrequency(calculateFrequency(midiMessage.getMidiNoteInHertz(getCurrentNote())));
        setGain(calculateGain(midiMessage.getVelocity()));
    }
    void noteOff ()
    {
        getADSR().noteOff();
        _current_note = -1;
    }
    
    //==============================================================================
    void setWaveType(VoiceWaveType wave_type)
    {
        switch(wave_type)
        {
            case (VoiceWaveType::SIN):
                getOSC().initialise(genSinWave, 128);
                break;
            case (VoiceWaveType::SAW):
                getOSC().initialise(genSawWave, 128);
                break;
        }
        getOSC().reset();
    }
    void setFrequency(int frequency)
    {
        getOSC().setFrequency(frequency, false);
    }
    void setGain(float gain)
    {
        getGain().setGainLinear(gain);
    }
    void setTranspose(VoiceTranspose transpose)
    {
        _transpose = transpose;
    }
    
    //==============================================================================
    bool isBusy ()
    {
        return (getADSR().isActive());
    }
    void setCurrentNote (int note_number)
    {
        _current_note = note_number;
    }
    int getCurrentNote ()
    {
        return _current_note;
    }
    
private:
    //==============================================================================
    using OSC            = juce::dsp::Oscillator<BufferData>;
    using Gain           = juce::dsp::Gain<BufferData>;
    using ProcessorChain = juce::dsp::ProcessorChain<OSC, ADSRProcessor, Gain>;
    
    //==============================================================================
    ProcessorChain _processorChain;
    int            _current_note = -1;
    VoiceTranspose _transpose = VoiceTranspose::NO_TRANSPOSE;
    
    //==============================================================================
    OSC& getOSC () noexcept
    {
        return _processorChain.get<0>();
    }
    ADSRProcessor& getADSR () noexcept
    {
        return _processorChain.get<1>();
    }
    Gain& getGain () noexcept
    {
        return _processorChain.get<2>();
    }

    //==============================================================================
    BufferData static genSinWave (BufferData angle)
    {
        return std::sin(angle);
    }
    BufferData static genSawWave (BufferData angle)
    {
        return juce::jmap(angle,
                          BufferData(-juce::MathConstants<double>::pi),
                          BufferData(juce::MathConstants<double>::pi),
                          BufferData(-1),
                          BufferData(1));
    }
    
    //==============================================================================
    int calculateFrequency (float note_freq)
    {
        switch (_transpose)
        {
            case VoiceTranspose::MINUS_TWO_OCTAVES : return note_freq / 4;
            case VoiceTranspose::MINUS_ONE_OCTAVE  : return note_freq / 2;
            case VoiceTranspose::NO_TRANSPOSE      : return note_freq;
            case VoiceTranspose::PLUS_ONE_OCTAVE   : return note_freq * 2;
            case VoiceTranspose::PLUS_TWO_OCTAVES  : return note_freq * 4;
        }
    }
    float calculateGain (float velocity)
    {
        return velocity / 127.0f * 0.05f;
    }
    
    //==============================================================================
    void onAttackChange (float attack)
    {
        getADSR().setParameter(ADSRStages::ATTACK, attack);
    }
    void onDecayChange (float decay)
    {
        getADSR().setParameter(ADSRStages::DECAY, decay);
    }
    void onSustainChange (float sustain)
    {
        getADSR().setParameter(ADSRStages::SUSTAIN, sustain);
    }
    void onReleaseChange (float release)
    {
        getADSR().setParameter(ADSRStages::RELEASE, release);
    }
};

//==============================================================================
class VoiceManager : public IAudioProcessor
{
public:
    //==============================================================================
    VoiceManager (IAudioProcessor::SynthStatePtr state_ptr): IAudioProcessor(state_ptr)
    {
       for (VoiceNum i = 0; i < _NUM_OF_VOICES; i++)
       {
           _voices.add(std::make_shared<Voice>(state_ptr));
       }
    }
    ~VoiceManager ()
    {
        reset();
        _voices.clear();
    }
    
    //==============================================================================
    void prepare (const IAudioProcessorConfig &spec) noexcept override
    {
        forEachVoice([&spec](auto voice) { voice->prepare(spec); });
    }
    void process (const IAudioProcessContext &context) noexcept override
    {
        forEachVoice([&context](auto voice) {
            auto& outputBlock = context.juce_context.getOutputBlock(); // get output audio block
            
            juce::AudioBuffer<BufferData> buffer(outputBlock.getNumChannels(), outputBlock.getNumSamples()); // init empty buffer with same size
            dsp::AudioBlock<BufferData> block(buffer); // init audio block from buffer
            block.copyFrom(outputBlock); // copy output buffer to it
            dsp::ProcessContextReplacing<BufferData> voice_context(block); // create context from it
            voice->process({ // process it independently
                .juce_context = voice_context
            });
            
            outputBlock.replaceWithSumOf(voice_context.getOutputBlock(), outputBlock); // mix it with output
        });
    }
    void reset () noexcept override
    {
        forEachVoice([](auto voice) { voice->reset(); });
    }
    
    //==============================================================================
    void noteOn (const juce::MidiMessage& midiMessage)
    {
        // try to play note using free voice
        try {
            findVoice([](auto voice) -> bool { return (!voice->isBusy()); })->noteOn(midiMessage);
        } catch (std::runtime_error& no_free_voice_error) {
            // steal voice if there are no free voices available
            try {
                findVoice([](auto voice) -> bool { return (voice->isBusy()); })->noteOn(midiMessage);
            } catch (std::runtime_error& steal_voice_error) {
                // something weird happened, do nothing
                std::cerr << "Cannot steal voice!" << std::endl;
            }
        }
    }
    void noteOff (const juce::MidiMessage& midiMessage)
    {
        try {
            findVoice([&midiMessage](auto voice) -> bool {
                return (voice->isBusy() && voice->getCurrentNote() == midiMessage.getNoteNumber());
            })->noteOff();
        } catch (std::runtime_error& no_voice_playing_note_error) {
            std::cerr << "Cannot find voice playing provided note!" << std::endl;
        }
    }
    void setTranspose(VoiceTranspose transpose)
    {
        forEachVoice([&transpose](auto voice) {
            voice->setTranspose(transpose);
        });
    }
    void setWaveType(VoiceWaveType wave_type)
    {
        forEachVoice([&wave_type](auto voice) { voice->setWaveType(wave_type); });
    }
    
private:
    //==============================================================================
    using VoiceNum = unsigned int;
    using VoicePtr = std::shared_ptr<Voice>;
    using VoiceCallback = std::function<void(VoicePtr)>;
    using VoicePredicate = std::function<bool(VoicePtr)>;
    
    //==============================================================================
    const VoiceNum        _NUM_OF_VOICES = 4;
    juce::Array<VoicePtr> _voices;
    
    //==============================================================================
    void forEachVoice (VoiceCallback callback)
    {
        for (auto _voice : _voices)
        {
            callback(_voice);
        }
    }
    VoicePtr findVoice (VoicePredicate predicate)
    {
        for (auto _voice : _voices)
        {
            if (predicate(_voice)) {
                return _voice;
            }
        }
        throw std::runtime_error("Cannot find voice");
    }
};

//==============================================================================
class SynthFilter : public IAudioProcessor
{
public:
    SynthFilter (IAudioProcessor::SynthStatePtr state_ptr): IAudioProcessor(state_ptr)
    {
        using namespace std::placeholders;
        
        getFilter().setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        
        setCutoff(getSynthState()->getFilterCutoff());
        getSynthState()->onFilterCutoffChange(std::bind(&SynthFilter::setCutoff, this, _1));
        
        setQ(getSynthState()->getFilterQ());
        getSynthState()->onFilterQChange(std::bind(&SynthFilter::setQ, this, _1));
    };
    
    //==============================================================================
    void prepare (const IAudioProcessorConfig& spec) noexcept override
    {
        getFilter().prepare(spec.juce_spec);
    }
    void process (const IAudioProcessContext& context) noexcept override
    {
        getFilter().process(context.juce_context);
    }
    void reset () noexcept override
    {
        getFilter().reset();
    }
    
    //==============================================================================
    void setCutoff (float frequency)
    {
        getFilter().setCutoffFrequency(frequency);
    }
    void setQ (float q)
    {
        getFilter().setResonance(q);
    }
    
private:
    //==============================================================================
    using JuceFilter = juce::dsp::StateVariableTPTFilter<BufferData>;
    
    JuceFilter _juce_filter;
    
    JuceFilter& getFilter()
    {
        return _juce_filter;
    }
};

//==============================================================================
class Synthesizer: public IAudioProcessor
{
public:
    Synthesizer (IAudioProcessor::SynthStatePtr state_ptr):
        IAudioProcessor(state_ptr),
        _voiceManager_1(state_ptr),
        _voiceManager_2(state_ptr),
        _filter(state_ptr)
    {
        using namespace std::placeholders;
        
        _voiceManager_1.setWaveType(state_ptr->getWaveType(SynthOSC::FIRST_OSC));
        getSynthState()->onWaveTypeChange(SynthOSC::FIRST_OSC,  std::bind(&Synthesizer::onOSC1WaveTypeChange, this, _1));
        
        _voiceManager_1.setTranspose(state_ptr->getTranspose(SynthOSC::FIRST_OSC));
        getSynthState()->onTransposeChnge(SynthOSC::FIRST_OSC,  std::bind(&Synthesizer::onOSC1TransposeChange, this, _1));
        
        _voiceManager_2.setWaveType(state_ptr->getWaveType(SynthOSC::SECOND_OSC));
        getSynthState()->onWaveTypeChange(SynthOSC::SECOND_OSC, std::bind(&Synthesizer::onOSC2WaveTypeChange, this, _1));
        
        _voiceManager_2.setTranspose(state_ptr->getTranspose(SynthOSC::SECOND_OSC));
        getSynthState()->onTransposeChnge(SynthOSC::SECOND_OSC, std::bind(&Synthesizer::onOSC2TransposeChange, this, _1));
    };
    
    //==============================================================================
    void prepare (const IAudioProcessorConfig &spec) noexcept override
    {
        _voiceManager_1.prepare(spec);
        _voiceManager_2.prepare(spec);
        _filter.prepare(spec);
    }
    void process (const IAudioProcessContext &context) noexcept override
    {
        _voiceManager_1.process(context);
        processVoiceParalell(_voiceManager_2, context);
        _filter.process(context);
    }
    void reset () noexcept override
    {
        _voiceManager_1.reset();
        _voiceManager_2.reset();
        _filter.reset();
    }
    
    //==============================================================================
    void noteOn (const juce::MidiMessage& midiMessage)
    {
        _voiceManager_1.noteOn(midiMessage);
        _voiceManager_2.noteOn(midiMessage);
    }
    void noteOff (const juce::MidiMessage& midiMessage)
    {
        _voiceManager_1.noteOff(midiMessage);
        _voiceManager_2.noteOff(midiMessage);
    }
    
private:
    VoiceManager _voiceManager_1;
    VoiceManager _voiceManager_2;
    SynthFilter  _filter;
    
    //==============================================================================
    void processVoiceParalell(VoiceManager& voice_manager, const IAudioProcessContext &context)
    {
        auto& outputBlock = context.juce_context.getOutputBlock(); // get output audio block
        juce::AudioBuffer<BufferData> buffer(outputBlock.getNumChannels(), outputBlock.getNumSamples()); // init empty buffer with same size
        dsp::AudioBlock<BufferData> block(buffer); // init audio block from buffer
        block.copyFrom(outputBlock); // copy output buffer to it
        dsp::ProcessContextReplacing<BufferData> voice_context(block); // create context from it
        voice_manager.process(context);
        outputBlock.replaceWithSumOf(voice_context.getOutputBlock(), outputBlock); // mix it with output
    }
    
    //==============================================================================
    void onOSC1TransposeChange(VoiceTranspose transpose)
    {
        _voiceManager_1.setTranspose(transpose);
    }
    void onOSC1WaveTypeChange(VoiceWaveType wave_type)
    {
        _voiceManager_1.setWaveType(wave_type);
    }
    void onOSC2TransposeChange(VoiceTranspose transpose)
    {
        _voiceManager_2.setTranspose(transpose);
    }
    void onOSC2WaveTypeChange(VoiceWaveType wave_type)
    {
        _voiceManager_2.setWaveType(wave_type);
    }
};

//==============================================================================
class SynthesizerDefaultParams
{
public:
    static VoiceTranspose OSC1Transpose()
    {
        return VoiceTranspose::NO_TRANSPOSE;
    }
    static VoiceTranspose OSC2Transpose()
    {
        return VoiceTranspose::NO_TRANSPOSE;
    }
    static VoiceWaveType OSC1WaveType()
    {
        return VoiceWaveType::SIN;
    }
    static VoiceWaveType OSC2WaveType()
    {
        return VoiceWaveType::SIN;
    }
    static float FilterFrequency()
    {
        return 1000.0f;
    }
    static float AMPAttack()
    {
        return 1.1f;
    }
    static float AMPDecay()
    {
        return 0.1f;
    }
    static float AMPSustain()
    {
        return 1.0f;
    }
    static float AMPRelease()
    {
        return 3.9f;
    }
};

//==============================================================================
/**
*/
class GGranulaAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    GGranulaAudioProcessor();
    ~GGranulaAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources () override;

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

    //==============================================================================
    int toTransposeParameterIndex(const juce::String& transpose)
    {
        if (transpose == "-2")
        {
            return 1;
        } else if (transpose == "-1")
        {
            return 2;
        } else if (transpose == "0")
        {
            return 3;
        } else if (transpose == "+1")
        {
            return 4;
        } else if (transpose == "+2")
        {
            return 5;
        } else
        {
            return 3;
        }
    }
    int toWaveformParameterIndex(const juce::String& waveform)
    {
        if (waveform == "sin")
        {
            return 0;
        } else {
            return 1;
        }
    }
    void setOSC1TransposeParameter(const juce::String& transpose)
    {
       *osc_1_transpose = toTransposeParameterIndex(transpose);
    }
    void setOSC1WaveformParameter(const juce::String& waveform)
    {
        *osc_1_wave = toWaveformParameterIndex(waveform);
    }
    void setOSC2TransposeParameter(const juce::String& transpose)
    {
        *osc_1_transpose = toTransposeParameterIndex(transpose);
    }
    void setOSC2WaveformParameter(const juce::String& waveform)
    {
        *osc_2_wave = toWaveformParameterIndex(waveform);
    }
    void setCutoffParameter(float cutoff)
    {
        *filter_cutoff = cutoff;
    }
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GGranulaAudioProcessor)
    
    //==============================================================================
    using SynthesizerStatePtr = std::shared_ptr<SynthesizerState>;
    
    //==============================================================================
    SynthesizerStatePtr         synthesizerState;
    Synthesizer                 synthesizer;
    juce::AudioParameterChoice* osc_1_transpose;
    juce::AudioParameterChoice* osc_2_transpose;
    juce::AudioParameterChoice* osc_1_wave;
    juce::AudioParameterChoice* osc_2_wave;
    juce::AudioParameterFloat*  amp_attack;
    juce::AudioParameterFloat*  amp_decay;
    juce::AudioParameterFloat*  amp_sustain;
    juce::AudioParameterFloat*  amp_release;
    juce::AudioParameterFloat*  filter_cutoff;
    juce::AudioParameterFloat*  filter_q;
};
