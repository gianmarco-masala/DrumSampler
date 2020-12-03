#include "PluginProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumProcessor();
}

AudioProcessorEditor* DrumProcessor::createEditor()
{
    return new DrumEditor(*this, parameters);
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout params;
    String paramID = "p";
    ChannelNames chNames;
    auto outputs = chNames.outputs;

    // Create Master params
    params.add(std::make_unique<AudioParameterFloat>
        ("pMasterLevel",
               "Master Level",
               0.0f, 1.0f, 1.0f));
    params.add(std::make_unique<AudioParameterFloat>
        ("pMasterPan",
               "Master Pan",
               -1.0f, 1.0f, 0.0f));
    params.add(std::make_unique<AudioParameterBool>
        ("pMasterMute",
               "Master Mute",
               false));

    // Create channels params
    for (int i = 0; i < MAX_INSTRUMENTS; i++) {
        String curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterFloat>
            (paramID << "Level",
                   curOut << " Level",
                   0.0f, 1.0f, 1.0f)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterFloat>
            (paramID << "Pan",
                   curOut << " Pan",
                   -1.0f, 1.0f, 0.0f)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterBool>
            (paramID << "Mute",
                   curOut << " Mute",
                   false)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterBool>
            (paramID << "Solo",
                   curOut << " Solo",
                   false)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterBool>
            (paramID << "Learn",
                   curOut << " Learn",
                   false)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterFloat>
            (paramID << "Coarse",
                   curOut << " Coarse",
                   -36.0f, 36.0f, 0.0f)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;

        params.add(std::make_unique<AudioParameterFloat>
            (paramID << "Fine",
                   curOut << " Fine",
                   -100.0f, 100.0f, 0.0f)
        );
        curOut = outputs[i];
        paramID.clear();
        paramID << "p" << curOut;
    }

    return params;
}

//==============================================================================
DrumProcessor::DrumProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Master", AudioChannelSet::stereo(), true)
        .withOutput("Kick", AudioChannelSet::stereo(), false)
        .withOutput("Snare", AudioChannelSet::stereo(), false)
        .withOutput("Tom 1", AudioChannelSet::stereo(), false)
        .withOutput("Tom 2", AudioChannelSet::stereo(), false)
        .withOutput("Tom 3", AudioChannelSet::stereo(), false)
        .withOutput("HiHat", AudioChannelSet::stereo(), false)
        .withOutput("Overhead", AudioChannelSet::stereo(), false)
    )
    , parameters(*this, nullptr, Identifier("DrumSamplerVTS"), createParameterLayout())
{
    auto outputs = chNames.outputs;

    // Generate instrument channels and attach parameters
    for (auto channel = 0; channel < MAX_INSTRUMENTS; channel++)
    {
        if (outputs[channel] != "Empty")
        {
            Logger::getCurrentLogger()->writeToLog(outputs[channel]);

            synth.add(new DrumSynth(outputs[channel]));

            attachChannelParams(channel);

            isChannelMuteEnabled[channel] = false;
            isSoloEnabled[channel] = false;
        }
        else
            break;
    }

    attachMasterParams();
}

DrumProcessor::~DrumProcessor()
{ }

//==============================================================================
void DrumProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    ignoreUnused(samplesPerBlock);
    auto lastSampleRate = sampleRate;
    auto outputs = chNames.outputs;
    String message;

    /*for (auto channel = 0; channel < maxMidiChannel; channel++) {
        if (outputs[channel] != "Empty")
            previousChannelGain[channel] = *level[channel];
    }*/

    previousMasterGain = *masterLevel;

    message << "Preparing to play audio...\n";
    message << " samplesPerBlock = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    Logger::getCurrentLogger()->writeToLog(message);

    for (auto midiChannel = 0; midiChannel < MAX_INSTRUMENTS; ++midiChannel)
    {
        if (outputs[midiChannel] != "Empty")
            synth[midiChannel]->setCurrentPlaybackSampleRate(lastSampleRate);
        else break;
    }
}

void DrumProcessor::releaseResources()
{
    Logger::getCurrentLogger()->writeToLog("Releasing audio resources");
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DrumProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void DrumProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto busCount = getBusCount(false);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Copy value from parameters (for master channel)
    auto curMasterGain = *masterLevel * 1;
    auto curMasterPan = *masterPan * 1;
    //auto isMasterMuteEnabled = *masterMute * 1;
    isMasterMuteEnabled = *masterMute > 0.5f ? true : false;

    //for (auto busNr = 0; busNr < busCount; ++busNr)
    //{
    //}

    // Channels management
    //for (auto busNr = 0; busNr < busCount; ++busNr)
    //{
    //	if (outputs[busNr] != "Empty")
    //	{
    //		// Copy value from parameters
    //		auto curChannelGain = level[busNr]->get();

    //		//if (!allSoloDisabled()) {
    //		//	if (busNr != *getSoloChannel())
    //		//		;
    //		//}

    //		//std::unique_ptr<AudioSampleBuffer> channelData;
    //		//channelData.reset(new AudioSampleBuffer(buffer.getNumChannels(), buffer.getNumSamples()));

    //		synth[busNr]->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //		if (isChannelMuteEnabled[busNr]) { curChannelGain = 0; }

    //		if (curChannelGain == previousChannelGain[busNr])
    //			buffer.applyGain(curChannelGain);
    //		else
    //		{
    //			buffer.applyGainRamp(0, buffer.getNumSamples(), previousChannelGain[busNr], curChannelGain);
    //			previousChannelGain[busNr] = curChannelGain;
    //		}

    //	}
    //}

    // Master management

    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    auto numSamples = buffer.getNumSamples();
    // Mute
    //if (isMasterMuteEnabled) { curMasterGain = 0; }
    // Level and Pan
    if (curMasterGain == previousMasterGain)
    {
        while (--numSamples >= 0)
        {
            if (outR != nullptr)
            {
                *outL++ *= curMasterGain * jmin(1.0f - curMasterPan, 1.0f);
                *outR++ *= curMasterGain * jmin(1.0f + curMasterPan, 1.0f);
            }
            else
            {
                *outL++ *= curMasterGain * jmin(1.0f - curMasterPan, 1.0f);
            }
        }
    }
    else
    {
        while (--numSamples >= 0)
        {
            if (outR != nullptr)
            {
                *outL++ *= curMasterGain * jmin(1.0f - curMasterPan, 1.0f);
                *outR++ *= curMasterGain * jmin(1.0f + curMasterPan, 1.0f);
            }
            else
            {
                *outL++ *= curMasterGain * jmin(1.0f - curMasterPan, 1.0f);
            }
        }
        buffer.applyGainRamp(0, buffer.getNumSamples(), previousMasterGain, curMasterGain);
        previousMasterGain = curMasterGain;
    }
}

//==============================================================================
void DrumProcessor::getStateInformation(MemoryBlock& destData)
{
    // ValueTree
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> vts_xml(state.createXml());
    copyXmlToBinary(*vts_xml, destData);
}

void DrumProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // ValueTree
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

/*
 * Attach registered parameter values to corresponding pointers for master channel
 */
void DrumProcessor::attachMasterParams()
{
    masterLevel = parameters.getRawParameterValue("pMasterLevel");
    masterPan = parameters.getRawParameterValue("pMasterPan");
    masterMute = parameters.getRawParameterValue("pMasterMute");
}

/*
 * Attach registered parameter values to corresponding pointers for each inst channel
 */
void DrumProcessor::attachChannelParams(int midiChannel)
{
    String paramID = "p";
    paramID.clear();
    paramID = "p";

    // Parametri Canali
    auto channelName = chNames.outputs[midiChannel];
    auto numVoices = synth[midiChannel]->getNumVoices();

    if (numVoices != 0) {
        for (auto i = 0; i < numVoices; i++)
        {
            auto currentVoice = static_cast<DrumVoice*>(synth[midiChannel]->getVoice(i));
            auto sampleRate = currentVoice->getSampleRate();
            paramID << "p";

            level[midiChannel] = parameters.getRawParameterValue(paramID << channelName << "Level");
            paramID.clear();
            paramID << "p";

            isChannelMuteEnabled[midiChannel] =
                parameters.getRawParameterValue(paramID << channelName << "Mute");
            paramID.clear();

            isSoloEnabled[midiChannel] =
                parameters.getRawParameterValue(paramID << channelName << "Solo");
            paramID.clear();

            paramID << "p";
            currentVoice->pan = parameters.getRawParameterValue(paramID << channelName << "Pan");
            paramID.clear();

            paramID << "p";
            currentVoice->coarse = parameters.getRawParameterValue(paramID << channelName << "Coarse");
            paramID.clear();

            paramID << "p";
            currentVoice->fine = parameters.getRawParameterValue(paramID << channelName << "Fine");
            paramID.clear();

            //paramID << "e";
            ////				auto env = currentVoice->env;
            //auto curAttack = parameters.getRawParameterValue(paramID << channelName << "Attack");
            ////				env->setAttackRate(*curAttack * sampleRate);
            //paramID.clear();

            //paramID << "e";
            //auto curHold = parameters.getRawParameterValue(paramID << channelName << "Hold");
            ////				env->setDecayRate(*curHold * sampleRate);
            //paramID.clear();

            //paramID << "e";
            //auto curRelease = parameters.getRawParameterValue(paramID << channelName << "Release");
            ////				env->setReleaseRate(*curRelease * sampleRate);
            //paramID.clear();
        }
    }
}