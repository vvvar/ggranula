/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GGranulaAudioProcessorEditor::GGranulaAudioProcessorEditor (GGranulaAudioProcessor& p)
    : AudioProcessorEditor (&p),
        audioProcessor (p)
{
    addAndMakeVisible(main_panel);
    setSize(720, 200);
}

GGranulaAudioProcessorEditor::~GGranulaAudioProcessorEditor()
{
}

//==============================================================================
void GGranulaAudioProcessorEditor::paint (juce::Graphics& g)
{}

void GGranulaAudioProcessorEditor::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Item = juce::GridItem;
    grid.templateRows    = { Track (Fr (1)) };
    grid.templateColumns = { Track (Fr (1)) };
    grid.items = { Item(main_panel) };
    grid.performLayout (getLocalBounds());
}

bool GGranulaAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

void GGranulaAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    std::cout << "filesDropped: " << x << " " << y << std::endl;
    for (auto file : files)
    {
        std::cout << "file: " << file << std::endl;
    }
}
