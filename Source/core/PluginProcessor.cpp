#include "PluginProcessor.h"
#include "../utils/DrumsetXmlHandler.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumProcessor();
}

AudioProcessorEditor* DrumProcessor::createEditor()
{
    return new GenericEditor(*this, parameters);
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    // RETRIEVE DRUMSET INFO
    DrumsetXmlHandler drumsetInfo;
    StringArray outputs = drumsetInfo.getActiveOutputs();

    // INIT PARAMS
    AudioProcessorValueTreeState::ParameterLayout params;
    String paramID = "p";

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
    for (int i = 0; i < outputs.size(); i++) {
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
    // RETRIEVE DRUMSET INFO
    outputs = drumsetInfo.getActiveOutputs();
    maxOutputs = outputs.size();

    // Generate a synth for each active output and attach parameters
    for (auto channel = 0; channel < maxOutputs; channel++)
    {
        Logger::getCurrentLogger()->writeToLog(outputs[channel]);
        synth.add(new DrumSynth(outputs[channel], parameters));
        attachChannelParams(channel);
        isChannelMuteEnabled[channel] = false;
        isSoloEnabled[channel] = false;
    }

    // Attach master parameters
    attachMasterParams();
}

DrumProcessor::~DrumProcessor()
{ }

//==============================================================================
void DrumProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    ignoreUnused(samplesPerBlock);
    auto lastSampleRate = sampleRate;
    String message;
    outputs = drumsetInfo.getActiveOutputs();

    for (auto channel = 0; channel < maxOutputs; channel++)
        previousChannelGain[channel] = *level[channel];

    previousMasterGain = *masterLevel;

    message << "Preparing to play audio...\n";
    message << " samplesPerBlock = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    Logger::getCurrentLogger()->writeToLog(message);

    for (auto midiChannel = 0; midiChannel < maxOutputs; ++midiChannel)
        synth[midiChannel]->setCurrentPlaybackSampleRate(lastSampleRate);
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
    //auto busCount = getBusCount(false);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Copy value from parameters (for master channel)
    auto curMasterGain = static_cast<float>(*masterLevel);
    auto curMasterPan = static_cast<float>(*masterPan);
    auto isMasterMuteEnabled = *masterMute > 0.5f ? true : false;

    // Channels management
    for (auto busNr = 0; busNr < maxOutputs; ++busNr)
    {
        // Copy value from parameters
        auto curChannelGain = static_cast<float>(*level[busNr]);

        //if (!allSoloDisabled()) {
        //	if (busNr != *getSoloChannel())
        //		;
        //}

        //std::unique_ptr<AudioSampleBuffer> channelData;
        //channelData.reset(new AudioSampleBuffer(buffer.getNumChannels(), buffer.getNumSamples()));

        synth[busNr]->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

        if (isChannelMuteEnabled[busNr]) { curChannelGain = 0; }

        if (curChannelGain == previousChannelGain[busNr])
            buffer.applyGain(curChannelGain);
        else
        {
            buffer.applyGainRamp(0, buffer.getNumSamples(), previousChannelGain[busNr], curChannelGain);
            previousChannelGain[busNr] = curChannelGain;
        }
    }

    // Master management

    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    auto numSamples = buffer.getNumSamples();
    // Mute
    if (isMasterMuteEnabled) { curMasterGain = 0; }
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
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> vts_xml(state.createXml());
    copyXmlToBinary(*vts_xml, destData);
}

void DrumProcessor::setStateInformation(const void* data, int sizeInBytes)
{
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
    auto channelName = outputs[midiChannel];
    auto numVoices = synth[midiChannel]->getNumVoices();

    if (numVoices != 0) {
        for (auto i = 0; i < numVoices; i++)
        {
            auto currentVoice = static_cast<DrumVoice*>(synth[midiChannel]->getVoice(i));
            auto sampleRate = currentVoice->getSampleRate();

            // Level
            paramID << "p";
            level[midiChannel] = parameters.getRawParameterValue(paramID << channelName << "Level");
            paramID.clear();

            // Mute
            paramID << "p";
            isChannelMuteEnabled[midiChannel] = parameters.getRawParameterValue(paramID << channelName << "Mute");
            paramID.clear();

            // Solo
            paramID << "p";
            isSoloEnabled[midiChannel] = parameters.getRawParameterValue(paramID << channelName << "Solo");
            paramID.clear();

            // Learn
            paramID << "p";
            synth[midiChannel]->attachMidiLearn(parameters.getRawParameterValue(paramID << channelName << "Learn"));
            paramID.clear();

            // Pan
            paramID << "p";
            currentVoice->pan = parameters.getRawParameterValue(paramID << channelName << "Pan");
            paramID.clear();

            // Coarse
            paramID << "p";
            currentVoice->coarse = parameters.getRawParameterValue(paramID << channelName << "Coarse");
            paramID.clear();

            // Fine
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