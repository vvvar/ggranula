/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GGranulaAudioProcessorEditor::GGranulaAudioProcessorEditor (GGranulaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (268, 268);
}

GGranulaAudioProcessorEditor::~GGranulaAudioProcessorEditor()
{
}

//==============================================================================
void GGranulaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Da, eto jestko", getLocalBounds(), juce::Justification::centred, 1);
    
    //Image background = ImageCache::getFromMemory(BinaryData::krug_jpg, BinaryData::krug_jpgSize);
    Image background = ImageCache::getFromMemory(BinaryData::BroVoging_jpg, BinaryData::BroVoging_jpgSize);
    g.drawImageAt (background, 0, 0);
}

void GGranulaAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
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
