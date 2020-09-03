/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class GGranulaAudioProcessorEditor  :
        public juce::AudioProcessorEditor,
        public juce::FileDragAndDropTarget
{
public:
    GGranulaAudioProcessorEditor (GGranulaAudioProcessor&);
    ~GGranulaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GGranulaAudioProcessor& audioProcessor;
    
    //==============================================================================
    struct BaseComponent : public Component
    {
        using Colour    = juce::Colour;
        using FontStyle = Font::FontStyleFlags;
        using Track     = juce::Grid::TrackInfo;
        using Fr        = juce::Grid::Fr;
        using Item      = juce::GridItem;
        using Margin    = juce::GridItem::Margin;
        using Justify   = juce::GridItem::JustifySelf;
        using Align     = juce::GridItem::AlignSelf;
        
        //==============================================================================
        BaseComponent(Colour background) : background_colour(background) {}
        
        //==============================================================================
        void paint (juce::Graphics& g) override { g.fillAll(background_colour); }
        void resized() override {}
        
        //==============================================================================
        struct PanelNameFont
        {
            const juce::String   name   { "Helvetica" };
            float                size   { 15 };
            Font::FontStyleFlags style  { FontStyle::bold };
            juce::Colour         colour { juce::Colours::black };
        };
        struct ParameterLabelFont
        {
            const juce::String   name   { "Helvetica" };
            float                size   { 13 };
            Font::FontStyleFlags style  { FontStyle::plain };
            juce::Colour         colour { juce::Colours::black };
        };
        
        //==============================================================================
        PanelNameFont      getPanelNameFont      () { return PanelNameFont(); }
        ParameterLabelFont getParameterLabelFont () { return ParameterLabelFont(); }
        
        //==============================================================================
        Colour background_colour;
    };
    
    //==============================================================================
    struct TransposePanel : public BaseComponent,
                            private juce::ComboBox::Listener
    {
        using TransposeChangeListener = std::function<void(const juce::String& transpose)>;
        
        //==============================================================================
        TransposePanel(juce::Colour background):
            BaseComponent(background),
            combo_box("transpose")
        {
            auto font = getParameterLabelFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            
            combo_box.addItem("-2", 1);
            combo_box.addItem("-1", 2);
            combo_box.addItem("0", 3);
            combo_box.addItem("+1", 4);
            combo_box.addItem("+2", 5);
            combo_box.setSelectedId(3);
            combo_box.addListener(this);
            addAndMakeVisible(combo_box);
        }
        
        //==============================================================================
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)) };
            grid.items =
            {
                Item(label),
                Item(combo_box),
            };
            grid.performLayout(getLocalBounds());
        }
        
        //==============================================================================
        void addListener(TransposeChangeListener listener)
        {
            listeners.push_back(listener);
        }
        
        //==============================================================================
        juce::ComboBox combo_box;
        juce::Label    label { "transpose", "Transpose" };
        
    private:
        std::list<TransposeChangeListener> listeners;
        
        //==============================================================================
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
        {
            for (auto listener : listeners)
            {
                try {
                    listener(comboBoxThatHasChanged->getText());
                } catch (...) {}
            }
        }
    };
    
    //==============================================================================
    struct WaveformPanel :  public BaseComponent,
                            private juce::ComboBox::Listener
    {
        using WaveformChangeListener = std::function<void(const juce::String& wavefrom)>;
        
        //==============================================================================
        WaveformPanel(juce::Colour background):
            BaseComponent(background),
            combo_box("waveform")
        {
            auto font = getParameterLabelFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            
            combo_box.addItem("sin", 1);
            combo_box.addItem("saw", 2);
            combo_box.setSelectedId(1);
            combo_box.addListener(this);
            addAndMakeVisible(combo_box);
        }
        
        //==============================================================================
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)) };
            grid.items =
            {
                Item(label),
                Item(combo_box),
            };
            grid.performLayout(getLocalBounds());
        }
        
        //==============================================================================
        void addListener(WaveformChangeListener listener)
        {
            listeners.push_back(listener);
        }
        
        //==============================================================================
        juce::ComboBox combo_box;
        juce::Label    label { "waveform", "Waveform" };
        
    private:
        std::list<WaveformChangeListener> listeners;
        
        //==============================================================================
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
        {
            for (auto listener : listeners)
            {
                try {
                    listener(comboBoxThatHasChanged->getText());
                } catch (...) {}
            }
        }
    };
    
    //==============================================================================
    struct OscillatorPanel : public BaseComponent
    {
        OscillatorPanel(juce::Colour background, const juce::String& label_text) :
            BaseComponent(background),
            label({}, label_text),
            transpose_panel(background),
            waveform_panel(background)
        {
            auto font = getParameterLabelFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            addAndMakeVisible(transpose_panel);
            addAndMakeVisible(waveform_panel);
        }
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (2)) };
            grid.templateColumns = { Track (Fr (1)), Track (Fr (1)) };
            grid.items =
            {
                Item(label),
                Item(),
                Item(transpose_panel).withMargin(Margin(0, 50, 0, 0)),
                Item(waveform_panel).withMargin(Margin(0, 50, 0, 0)),
            };
            grid.performLayout(getLocalBounds());
        }
        juce::Label    label;
        TransposePanel transpose_panel;
        WaveformPanel  waveform_panel;
    };
    
    //==============================================================================
    struct OscillatorsPanel : public BaseComponent
    {
        OscillatorsPanel(juce::Colour background) :
            BaseComponent(background),
            osc_1_panel(background, "OSC #1"),
            osc_2_panel(background, "OSC #2")
        {
            auto font = getPanelNameFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            addAndMakeVisible(osc_1_panel);
            addAndMakeVisible(osc_2_panel);
        }
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (2)), Track (Fr (2)) };
            grid.templateColumns = { Track (Fr (1)) };
            grid.items =
            {
                Item(label).withMargin(10),
                Item(osc_1_panel).withMargin(10),
                Item(osc_2_panel).withMargin(10)
            };
            grid.performLayout(getLocalBounds());
        }
        juce::Label     label { "oscillators", "Oscillators" };
        OscillatorPanel osc_1_panel;
        OscillatorPanel osc_2_panel;
    };
    
    //==============================================================================
    struct CutoffPanel : public BaseComponent
    {
        CutoffPanel(juce::Colour background) :
            BaseComponent(background),
            cutoff("Cutoff")
        {
            auto font = getPanelNameFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            label.setJustificationType(Justification::topLeft);
            addAndMakeVisible(label);
            
            cutoff.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            cutoff.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50.0f, 20.0f);
            cutoff.setSkewFactor(0.25f);
            cutoff.setRange(Range<double>(10.0f, 16000.0f), 1.0f);
            addAndMakeVisible(cutoff);
        }
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)) };
            grid.items =
            {
                Item(label),
                Item(cutoff)
            };
            grid.performLayout(getLocalBounds());
        }
        juce::Label  label { "cutoff", "Cutoff" };
        juce::Slider cutoff;
    };
    
    //==============================================================================
    struct FilterPanel : public BaseComponent
    {
        FilterPanel(juce::Colour background) : BaseComponent(background)
        {
            auto font = getPanelNameFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            
            cutoff.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            cutoff.setTextBoxStyle(juce::Slider::TextBoxRight, true, 50, 30);
            cutoff.setSkewFactor(0.25f);
            cutoff.setRange(Range<double>(10.0f, 16000.0f), 1.0f);
            addAndMakeVisible(cutoff);
            
            q.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            q.setTextBoxStyle(juce::Slider::TextBoxRight, true, 35.0f, 20.0f);
            q.setSkewFactor(0.5f);
            q.setRange(Range<double>(0.1f, 1.2f), 0.1f);
            addAndMakeVisible(q);
        }
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (2)) };
            grid.templateColumns = { Track (Fr (1)), Track (Fr (1)) };
            grid.items =
            {
                Item(label).withMargin(8),
                Item(),
                Item(cutoff),
                Item(q).withMargin(Margin(0, 20, 0, 0))
            };
            grid.performLayout(getLocalBounds());
        }
        juce::Label  label  { "filter", "Filter" };
        juce::Slider cutoff { "cutoff" };
        juce::Slider q      { "q" };
    };
    
    //==============================================================================
    struct ADSRPanel : public BaseComponent
    {
        ADSRPanel(juce::Colour background) : BaseComponent(background)
        {
            attack.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            attack.setTextBoxStyle(juce::Slider::TextBoxRight, true, 30, 16);
            attack.setNormalisableRange(juce::NormalisableRange<double>(0.01f, 10.0f, 0.01f, 0.5f));
            addAndMakeVisible(attack);
            
            decay.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            decay.setTextBoxStyle(juce::Slider::TextBoxRight, true, 30, 16);
            decay.setNormalisableRange(juce::NormalisableRange<double>(0.01f, 10.0f, 0.01f, 0.5f));
            addAndMakeVisible(decay);
            
            sustain.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            sustain.setTextBoxStyle(juce::Slider::TextBoxRight, true, 30, 16);
            sustain.setNormalisableRange(juce::NormalisableRange<double>(0.01f, 10.0f, 0.01f, 0.5f));
            addAndMakeVisible(sustain);
            
            release.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            release.setTextBoxStyle(juce::Slider::TextBoxRight, true, 30, 16);
            release.setNormalisableRange(juce::NormalisableRange<double>(0.01f, 20.0f, 0.01f, 0.5f));
            addAndMakeVisible(release);
        }
        
        void resized() override
        {
           juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)), Track (Fr (1)), Track (Fr (1)), Track (Fr (1)) };
            grid.items =
            {
                Item(attack),
                Item(decay),
                Item(sustain),
                Item(release)
            };
            grid.performLayout(getLocalBounds());
        }
        
        juce::Slider attack  { "attack" };
        juce::Slider decay   { "decay" };
        juce::Slider sustain { "sustain" };
        juce::Slider release { "release" };
    };
    
    //==============================================================================
    struct AmpPanel : public BaseComponent
    {
        AmpPanel(juce::Colour background) :
            BaseComponent(background),
            amp_panel(background)
        {
            auto font = getPanelNameFont();
            label.setColour(juce::Label::ColourIds::textColourId, font.colour);
            label.setFont(Font(font.name, font.size, font.style));
            addAndMakeVisible(label);
            addAndMakeVisible(amp_panel);
        }
        
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (2)) };
            grid.templateColumns = { Track (Fr (1)) };
            grid.items =
            {
                Item(label).withMargin(8),
                Item(amp_panel)
            };
            grid.performLayout(getLocalBounds());
        }
        
        juce::Label label { "amp", "AMP" };
        ADSRPanel   amp_panel;
    };
    
    //==============================================================================
    struct FilterAmpPanel : public BaseComponent
    {
        FilterAmpPanel(juce::Colour filter_background, juce::Colour adsr_background) :
            BaseComponent(filter_background),
            filter_panel(filter_background),
            amp_panel(adsr_background)
        {
            addAndMakeVisible(filter_panel);
            addAndMakeVisible(amp_panel);
        }
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)), Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)), };
            grid.items =
            {
                Item(filter_panel),
                Item(amp_panel)
            };
            grid.performLayout(getLocalBounds());
        }
        FilterPanel filter_panel;
        AmpPanel    amp_panel;
    };
    
    //==============================================================================
    struct MainPanel : public BaseComponent
    {
        MainPanel() :
            BaseComponent(juce::Colours::grey),
            osc_panel(juce::Colours::silver),
            filter_adsr_panel(juce::Colours::silver, juce::Colours::silver)
        {
            addAndMakeVisible(osc_panel);
            addAndMakeVisible(filter_adsr_panel);
        }
        
        //==============================================================================
        void resized() override
        {
            juce::Grid grid;
            grid.templateRows    = { Track (Fr (1)) };
            grid.templateColumns = { Track (Fr (1)), Track (Fr (1)) };
            grid.items = { Item(osc_panel).withMargin(10), Item(filter_adsr_panel).withMargin(10) };
            grid.performLayout (getLocalBounds());
        }
        
        //==============================================================================
        using TransposeChangeListener = std::function<void(const juce::String& transpose)>;
        using WaveformChangeListener  = std::function<void(const juce::String& wavefrom)>;
        
        void addOSC1TransposeListener(TransposeChangeListener listener)
        {
            osc_panel.osc_1_panel.transpose_panel.addListener(listener);
        }
        void addOSC1WaveformListener(WaveformChangeListener listener)
        {
            osc_panel.osc_1_panel.waveform_panel.addListener(listener);
        }
        void addOSC2TransposeListener(TransposeChangeListener listener)
        {
            osc_panel.osc_2_panel.transpose_panel.addListener(listener);
        }
        void addOSC2WaveformListener(WaveformChangeListener listener)
        {
            osc_panel.osc_2_panel.waveform_panel.addListener(listener);
        }
        
        //==============================================================================
        OscillatorsPanel osc_panel;
        FilterAmpPanel  filter_adsr_panel;
    };
    
    MainPanel main_panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GGranulaAudioProcessorEditor)
};
