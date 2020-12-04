#include "../core/PluginProcessor.h"
#include "../core/PluginEditor.h"

//==============================================================================
DrumEditor::DrumEditor(DrumProcessor& parent, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(parent)
    , processor(parent)
    , valueTreeState(vts)
{
    Logger::getCurrentLogger()->writeToLog(">>>>>>>>> Editor constructor called.");

    const MessageManagerLock mmLock;
    startTimer(100);

    // CHANNELS
    auto index = 0;
    outputs = drumsetInfo.getActiveOutputs();

    // CANALE 1
    auto name = outputs[index];
    auto curGroup = &group1;
    auto curLevelLabel = &levelLabel1;
    auto curPanLabel = &panLabel1;
    auto curLevelSlider = &levelSlider1;
    auto curPanSlider = &panSlider1;
    auto curMuteButton = &muteButton1;
    auto curSoloButton = &soloButton1;
    auto curLearnButton = &midiLearnButton1;
    auto curLevelAttach = levelAttachment1.get();
    auto curPanAttach = panAttachment1.get();
    auto curMuteAttach = muteAttachment1.get();
    auto curSoloAttach = soloAttachment1.get();
    auto curLearnAttach = midiLearnAttachment1.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 2
    name = outputs[index];
    curGroup = &group2;
    curLevelLabel = &levelLabel2;
    curPanLabel = &panLabel2;
    curLevelSlider = &levelSlider2;
    curPanSlider = &panSlider2;
    curMuteButton = &muteButton2;
    curSoloButton = &soloButton2;
    curLearnButton = &midiLearnButton2;
    curLevelAttach = levelAttachment2.get();
    curPanAttach = panAttachment2.get();
    curMuteAttach = muteAttachment2.get();
    curSoloAttach = soloAttachment2.get();
    curLearnAttach = midiLearnAttachment2.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 3
    name = outputs[index];
    curGroup = &group3;
    curLevelLabel = &levelLabel3;
    curPanLabel = &panLabel3;
    curLevelSlider = &levelSlider3;
    curPanSlider = &panSlider3;
    curMuteButton = &muteButton3;
    curSoloButton = &soloButton3;
    curLearnButton = &midiLearnButton3;
    curLevelAttach = levelAttachment3.get();
    curPanAttach = panAttachment3.get();
    curMuteAttach = muteAttachment3.get();
    curSoloAttach = soloAttachment3.get();
    curLearnAttach = midiLearnAttachment3.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 4
    name = outputs[index];
    curGroup = &group4;
    curLevelLabel = &levelLabel4;
    curPanLabel = &panLabel4;
    curLevelSlider = &levelSlider4;
    curPanSlider = &panSlider4;
    curMuteButton = &muteButton4;
    curSoloButton = &soloButton4;
    curLearnButton = &midiLearnButton4;
    curLevelAttach = levelAttachment4.get();
    curPanAttach = panAttachment4.get();
    curMuteAttach = muteAttachment4.get();
    curSoloAttach = soloAttachment4.get();
    curLearnAttach = midiLearnAttachment4.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 5
    name = outputs[index];
    curGroup = &group5;
    curLevelLabel = &levelLabel5;
    curPanLabel = &panLabel5;
    curLevelSlider = &levelSlider5;
    curPanSlider = &panSlider5;
    curMuteButton = &muteButton5;
    curSoloButton = &soloButton5;
    curLearnButton = &midiLearnButton5;
    curLevelAttach = levelAttachment5.get();
    curPanAttach = panAttachment5.get();
    curMuteAttach = muteAttachment5.get();
    curSoloAttach = soloAttachment5.get();
    curLearnAttach = midiLearnAttachment5.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 6
    name = outputs[index];
    curGroup = &group6;
    curLevelLabel = &levelLabel6;
    curPanLabel = &panLabel6;
    curLevelSlider = &levelSlider6;
    curPanSlider = &panSlider6;
    curMuteButton = &muteButton6;
    curSoloButton = &soloButton6;
    curLearnButton = &midiLearnButton6;
    curLevelAttach = levelAttachment6.get();
    curPanAttach = panAttachment6.get();
    curMuteAttach = muteAttachment6.get();
    curSoloAttach = soloAttachment6.get();
    curLearnAttach = midiLearnAttachment6.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 7
    name = outputs[index];
    curGroup = &group7;
    curLevelLabel = &levelLabel7;
    curPanLabel = &panLabel7;
    curLevelSlider = &levelSlider7;
    curPanSlider = &panSlider7;
    curMuteButton = &muteButton7;
    curSoloButton = &soloButton7;
    curLearnButton = &midiLearnButton7;
    curLevelAttach = levelAttachment7.get();
    curPanAttach = panAttachment7.get();
    curMuteAttach = muteAttachment7.get();
    curSoloAttach = soloAttachment7.get();
    curLearnAttach = midiLearnAttachment7.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // CANALE 8
    name = outputs[index];
    curGroup = &group8;
    curLevelLabel = &levelLabel8;
    curPanLabel = &panLabel8;
    curLevelSlider = &levelSlider8;
    curPanSlider = &panSlider8;
    curMuteButton = &muteButton8;
    curSoloButton = &soloButton8;
    curLearnButton = &midiLearnButton8;
    curLevelAttach = levelAttachment8.get();
    curPanAttach = panAttachment8.get();
    curMuteAttach = muteAttachment8.get();
    curSoloAttach = soloAttachment8.get();
    curLearnAttach = midiLearnAttachment8.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // Master
    name = "Master";
    curGroup = &masterGroup;
    curLevelLabel = &masterLevelLabel;
    curPanLabel = &masterPanLabel;
    curLevelSlider = &masterLevelSlider;
    curPanSlider = &masterPanSlider;
    curMuteButton = &masterMuteButton;
    curSoloButton = nullptr;
    curLearnButton = nullptr;
    curLevelAttach = masterLevelAttachment.get();
    curPanAttach = masterPanAttachment.get();
    curMuteAttach = masterMuteAttachment.get();

    initChannel(vts, name, ++index, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton,
        curSoloButton, curLearnButton, curLevelAttach, curPanAttach, curMuteAttach, curSoloAttach, curLearnAttach);

    // Window size
    setSize(1000, 600);
}

//==============================================================================
void  DrumEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void  DrumEditor::resized()
{
    auto start = startingPoint;

    // Channel 1
    auto curGroup = &group1;
    auto curLevelLabel = &levelLabel1;
    auto curPanLabel = &panLabel1;
    auto curLevelSlider = &levelSlider1;
    auto curPanSlider = &panSlider1;
    auto curMuteButton = &muteButton1;
    auto curSoloButton = &soloButton1;
    auto curLearnButton = &midiLearnButton1;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 2
    curGroup = &group2;
    curLevelLabel = &levelLabel2;
    curPanLabel = &panLabel2;
    curLevelSlider = &levelSlider2;
    curPanSlider = &panSlider2;
    curMuteButton = &muteButton2;
    curSoloButton = &soloButton2;
    curLearnButton = &midiLearnButton2;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 3
    curGroup = &group3;
    curLevelLabel = &levelLabel3;
    curPanLabel = &panLabel3;
    curLevelSlider = &levelSlider3;
    curPanSlider = &panSlider3;
    curMuteButton = &muteButton3;
    curSoloButton = &soloButton3;
    curLearnButton = &midiLearnButton3;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 4
    curGroup = &group4;
    curLevelLabel = &levelLabel4;
    curPanLabel = &panLabel4;
    curLevelSlider = &levelSlider4;
    curPanSlider = &panSlider4;
    curMuteButton = &muteButton4;
    curSoloButton = &soloButton4;
    curLearnButton = &midiLearnButton4;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 5
    curGroup = &group5;
    curLevelLabel = &levelLabel5;
    curPanLabel = &panLabel5;
    curLevelSlider = &levelSlider5;
    curPanSlider = &panSlider5;
    curMuteButton = &muteButton5;
    curSoloButton = &soloButton5;
    curLearnButton = &midiLearnButton5;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 6
    curGroup = &group6;
    curLevelLabel = &levelLabel6;
    curPanLabel = &panLabel6;
    curLevelSlider = &levelSlider6;
    curPanSlider = &panSlider6;
    curMuteButton = &muteButton6;
    curSoloButton = &soloButton6;
    curLearnButton = &midiLearnButton6;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 7
    curGroup = &group7;
    curLevelLabel = &levelLabel7;
    curPanLabel = &panLabel7;
    curLevelSlider = &levelSlider7;
    curPanSlider = &panSlider7;
    curMuteButton = &muteButton7;
    curSoloButton = &soloButton7;
    curLearnButton = &midiLearnButton7;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth();

    // Channel 8
    curGroup = &group8;
    curLevelLabel = &levelLabel8;
    curPanLabel = &panLabel8;
    curLevelSlider = &levelSlider8;
    curPanSlider = &panSlider8;
    curMuteButton = &muteButton8;
    curSoloButton = &soloButton8;
    curLearnButton = &midiLearnButton8;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);
    start += curGroup->getWidth() + 10;		// last channel, blank space to separate channels from master

    // Master
    curGroup = &masterGroup;
    curLevelLabel = &masterLevelLabel;
    curPanLabel = &masterPanLabel;
    curLevelSlider = &masterLevelSlider;
    curPanSlider = &masterPanSlider;
    curMuteButton = &masterMuteButton;

    drawChannel(start, curGroup, curLevelLabel, curPanLabel, curLevelSlider, curPanSlider, curMuteButton, curSoloButton, curLearnButton);

}

void  DrumEditor::timerCallback()
{
    repaint();

    //if (processor.synth[0]->isMidiLearning)
    //{
    //    midiLearnButton1.setButtonText(TRANS("Learning..."));
    //    //midiLearnButton1.setToggleState(true, dontSendNotification);
    //}
    //else {
    //    midiLearnButton1.setButtonText(TRANS("Learn"));
    //    //midiLearnButton1.setToggleState(false, dontSendNotification);
    //}
    // ...

}

void  DrumEditor::sliderValueChanged(Slider* slider)
{

}

void  DrumEditor::sliderDragStarted(Slider* slider)
{

}


void  DrumEditor::sliderDragEnded(Slider* slider)
{

}

void  DrumEditor::buttonClicked(Button* button)
{ }


void  DrumEditor::initChannel(AudioProcessorValueTreeState& vts,
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
    ButtonAttachment* midiLearnAttachment
)
{
    String componentID, paramID, title;

    componentID = static_cast<String>(index);
    title.clear();
    title << "#" << componentID << ": " << channelName;

    // Group
    addAndMakeVisible(group);
    group->setName(channelName);
    group->setText(title);
    group->setTextLabelPosition(Justification::centred);
    group->setComponentID(componentID);

    // Labels
    initLabel(*levelLabel);
    initLabel(*panLabel);
    levelLabel->setText("Level", dontSendNotification);
    panLabel->setText("Pan", dontSendNotification);

    // Sliders
    initSlider(*levelSlider, Slider::LinearVertical);
    levelSlider->setRange(0, 1, 0);
    levelAttachment = new SliderAttachment(vts, paramID << "p" << channelName << "Level", *levelSlider);
    paramID.clear();

    initSlider(*panSlider, Slider::LinearHorizontal);
    panSlider->setRange(-50.0, 50.0, 0.01);
    panAttachment = new SliderAttachment(vts, paramID << "p" << channelName << "Pan", *panSlider);
    paramID.clear();

    // Mute Button
    initButton(*muteButton);
    muteButton->setButtonText(TRANS("M"));
    muteAttachment = new ButtonAttachment(vts, paramID << "p" << channelName << "Mute", *muteButton);
    paramID.clear();

    // Solo Button
    if (soloButton != nullptr) {
        initButton(*soloButton);
        soloButton->setButtonText(TRANS("S"));
        soloAttachment = new ButtonAttachment(vts, paramID << "p" << channelName << "Solo", *soloButton);
        paramID.clear();
    }
    // Learn Button
    if (midiLearnButton != nullptr) {
        initButton(*midiLearnButton);
        midiLearnButton->setButtonText(TRANS("Learn"));
        midiLearnAttachment = new ButtonAttachment(vts, paramID << "p" << channelName << "Learn", *midiLearnButton);
    }

    // Attach components
    group->addChildComponent(levelLabel, -1);
    group->addChildComponent(levelSlider, -1);
    group->addChildComponent(panLabel, -1);
    group->addChildComponent(panSlider, -1);
    group->addChildComponent(muteButton, -1);
    if (soloButton != nullptr)
        group->addChildComponent(soloButton, -1);
    if (midiLearnButton != nullptr)
        group->addChildComponent(midiLearnButton, -1);
}


void  DrumEditor::drawChannel(int startX,
    Component* group,
    Component* levelLabel,
    Component* panLabel,
    Component* levelSlider,
    Component* panSlider,
    Component* muteButton,
    Component* soloButton,
    Component* midiLearnButton)
{
    auto windowWidth = getWidth() - 30;
    auto windowHeight = getHeight() - 30;
    auto groupWidth = windowWidth / (MAX_INSTRUMENTS + 1); // per fare spazio al master
    auto groupHeight = windowHeight;
    auto buttonWidth = 50;
    auto buttonHeight = 50;
    auto labelHeight = 50;

    auto groupStartX = startX;
    auto groupStartY = startingPoint;
    auto blankSpace = 20;

    group->setBounds(
        groupStartX,
        groupStartY,
        groupWidth,
        groupHeight
    );

    panLabel->setBounds(
        panLabel->getBoundsInParent().getX(),			// x
        panLabel->getBoundsInParent().getY() + 5,						    // y
        panLabel->getParentWidth(),									// width
        labelHeight + 20								// height
    );

    panSlider->setBounds(
        panSlider->getBoundsInParent().getX(),		// x
        panSlider->getBoundsInParent().getY() + panLabel->getHeight(),      // y
        panSlider->getParentWidth(),									// width
        50								// height
    );

    muteButton->setBounds(
        muteButton->getBoundsInParent().getCentreX() + blankSpace,			// x
        blankSpace + panSlider->getY() + panSlider->getHeight(),											// y
        buttonWidth,									// width
        buttonHeight);								// height

    soloButton->setBounds(
        muteButton->getX() + buttonWidth - blankSpace / 2,			// x
        blankSpace + panSlider->getY() + panSlider->getHeight(),											// y
        buttonWidth,									// width
        buttonHeight);								// height

    midiLearnButton->setBounds
    (midiLearnButton->getBoundsInParent().getX(),
        blankSpace + muteButton->getY() + muteButton->getHeight(),
        midiLearnButton->getParentWidth(),
        buttonHeight);

    levelLabel->setBounds
    (levelLabel->getBoundsInParent().getX(),			// x
        blankSpace + midiLearnButton->getBoundsInParent().getY() + midiLearnButton->getHeight(),						    // y
        levelLabel->getParentWidth(),									// width
        labelHeight);								// height

    levelSlider->setBounds
    (levelSlider->getBoundsInParent().getX(),		// x
        levelLabel->getBoundsInParent().getY() + levelLabel->getHeight(),      // y
        levelSlider->getParentWidth(),									// width
        levelSlider->getParentHeight() / 3);								// height
}