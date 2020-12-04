#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../core/PluginProcessor.h"
#include "../utils/DrumsetXmlHandler.h"

//==============================================================================

class DrumProcessor;

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

class GenericEditor : public AudioProcessorEditor
{
public:
    GenericEditor(DrumProcessor& parent, AudioProcessorValueTreeState& vts);

    void paint(Graphics&) override;
    void resized() override;

private:
    DrumProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
};

class  DrumEditor :
    public AudioProcessorEditor,
    public Slider::Listener,
    public Button::Listener,
    public Timer
{
public:

    DrumEditor(DrumProcessor& parent, AudioProcessorValueTreeState& vts);

    void paint(Graphics&) override;
    void resized() override;

private:
    // Must override this methods in order to inherit Listener properties.
    // Actually they won't do nothing because editor's components changes are
    // automatically notified to the processor by AudioProcessorValueTreeState
    void sliderValueChanged(Slider* slider) override { };
    void sliderDragStarted(Slider* slider) override { };
    void sliderDragEnded(Slider* slider) override { };
    void buttonClicked(Button* button) override { };

    void timerCallback() override;

    // Custom init functions
    void initLabel(Label& label)
    {
        addAndMakeVisible(label);
        label.setFont(Font(18.00f, Font::plain).withTypefaceStyle("Regular"));
        label.setJustificationType(Justification::centred);
        label.setEditable(false, false, false);
        label.setColour(TextEditor::textColourId, Colours::black);
        label.setColour(TextEditor::backgroundColourId, Colour(0x00000000));
    }

    void initSlider(Slider& slider, Slider::SliderStyle style)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(style);
        slider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        slider.addListener(this);
    }

    void initButton(Button& button)
    {
        addAndMakeVisible(button);
        button.addListener(this);
    }

    void initChannel(AudioProcessorValueTreeState& vts,
        String& channelName,
        int index,
        GroupComponent* group,
        Label* levelLabel,
        Label* panLabel,
        Slider* levelSlider,
        Slider* panSlider,
        Button* muteButton,
        Button* soloButton,
        Button* midiLearnButton,
        SliderAttachment* levelAttachment,
        SliderAttachment* panAttachment,
        ButtonAttachment* muteAttachment,
        ButtonAttachment* soloAttachment,
        ButtonAttachment* midiLearnAttachment);

    void drawChannel(int startX,
        Component* group,
        Component* levelLabel,
        Component* panLabel,
        Component* levelSlider,
        Component* panSlider,
        Component* muteButton,
        Component* soloButton,
        Component* midiLearnButton);

    DrumProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    DrumsetXmlHandler drumsetInfo;
    StringArray outputs;

    int numChannels, startingPoint = 5;

    GroupComponent
        group1,
        group2,
        group3,
        group4,
        group5,
        group6,
        group7,
        group8,
        masterGroup;

    Slider
        levelSlider1,
        levelSlider2,
        levelSlider3,
        levelSlider4,
        levelSlider5,
        levelSlider6,
        levelSlider7,
        levelSlider8,
        masterLevelSlider,

        panSlider1,
        panSlider2,
        panSlider3,
        panSlider4,
        panSlider5,
        panSlider6,
        panSlider7,
        panSlider8,
        masterPanSlider;

    std::unique_ptr<SliderAttachment>
        levelAttachment1,
        levelAttachment2,
        levelAttachment3,
        levelAttachment4,
        levelAttachment5,
        levelAttachment6,
        levelAttachment7,
        levelAttachment8,
        masterLevelAttachment,

        panAttachment1,
        panAttachment2,
        panAttachment3,
        panAttachment4,
        panAttachment5,
        panAttachment6,
        panAttachment7,
        panAttachment8,
        masterPanAttachment;

    Label
        levelLabel1,
        levelLabel2,
        levelLabel3,
        levelLabel4,
        levelLabel5,
        levelLabel6,
        levelLabel7,
        levelLabel8,
        masterLevelLabel,

        panLabel1,
        panLabel2,
        panLabel3,
        panLabel4,
        panLabel5,
        panLabel6,
        panLabel7,
        panLabel8,
        masterPanLabel;

    ToggleButton
        muteButton1,
        muteButton2,
        muteButton3,
        muteButton4,
        muteButton5,
        muteButton6,
        muteButton7,
        muteButton8,
        masterMuteButton,

        soloButton1,
        soloButton2,
        soloButton3,
        soloButton4,
        soloButton5,
        soloButton6,
        soloButton7,
        soloButton8,

        midiLearnButton1,
        midiLearnButton2,
        midiLearnButton3,
        midiLearnButton4,
        midiLearnButton5,
        midiLearnButton6,
        midiLearnButton7,
        midiLearnButton8;

    std::unique_ptr<ButtonAttachment>
        muteAttachment1,
        muteAttachment2,
        muteAttachment3,
        muteAttachment4,
        muteAttachment5,
        muteAttachment6,
        muteAttachment7,
        muteAttachment8,
        masterMuteAttachment,

        soloAttachment1,
        soloAttachment2,
        soloAttachment3,
        soloAttachment4,
        soloAttachment5,
        soloAttachment6,
        soloAttachment7,
        soloAttachment8,

        midiLearnAttachment1,
        midiLearnAttachment2,
        midiLearnAttachment3,
        midiLearnAttachment4,
        midiLearnAttachment5,
        midiLearnAttachment6,
        midiLearnAttachment7,
        midiLearnAttachment8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumEditor)
};