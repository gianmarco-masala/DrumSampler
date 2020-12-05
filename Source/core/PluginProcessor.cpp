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

static MidiBuffer filterMidiMessagesForChannel(const MidiBuffer& input, int channel)
{
    MidiBuffer output;

    for (const auto metadata : input)
    {
        const auto message = metadata.getMessage();

        if (message.getChannel() == channel)
            output.addEvent(message, metadata.samplePosition);
    }

    return output;
}

//==============================================================================
DrumProcessor::DrumProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Master", AudioChannelSet::stereo(), true)
        //.withOutput("Kick", AudioChannelSet::stereo(), false)
        //.withOutput("Snare", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 1", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 2", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 3", AudioChannelSet::stereo(), false)
        //.withOutput("HiHat", AudioChannelSet::stereo(), false)
        //.withOutput("Overhead", AudioChannelSet::stereo(), false)
    )
    , parameters(*this, nullptr, Identifier("DrumSamplerVTS"), createParameterLayout())
{
    // Retrieve drumset info
    outputs = drumsetInfo.getActiveOutputs();
    maxOutputs = outputs.size();
    int note = startNote;

    // Generate a synth for each active output and attach parameters
    for (auto channel = 0; channel < maxOutputs; channel++)
    {
        Logger::getCurrentLogger()->writeToLog(outputs[channel]);
        synth.add(
            new DrumSynth(
            parameters,
            outputs[channel],
            channel,
            note++
        ));

        attachChannelParams(channel);
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
    String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlock = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    Logger::getCurrentLogger()->writeToLog(message);

    auto lastSampleRate = sampleRate;
    outputs = drumsetInfo.getActiveOutputs();
    prevGain = *level;

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

void DrumProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiBuffer)
{
    ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    //auto busCount = getBusCount(false);

    // Clear buffer before fill it
    for (auto i = 0; i < totalNumOutputChannels; i++)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Copy param values
    auto curGain = static_cast<float>(*level);
    auto curPan = static_cast<float>(*pan);
    auto isMuteEnabled = *muteEnabled > 0.5f ? true : false;

    // Pass midi messages to each synth so 
    // they can fill the output buffer
    for (auto chNr = 0; chNr < maxOutputs; chNr++)
    {
        synth[chNr]->renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
    }

    // Write buffer to output after applying levels
    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    auto numSamples = buffer.getNumSamples();

    if (isMuteEnabled) { curGain = 0; }

    if (curGain == prevGain)
    {
        // Normal behaviour
        while (--numSamples >= 0)
        {
            if (outR != nullptr)
            {
                *outL++ *= curGain * jmin(1.0f - curPan, 1.0f);
                *outR++ *= curGain * jmin(1.0f + curPan, 1.0f);
            }
            else
            {
                *outL++ *= curGain * jmin(1.0f - curPan, 1.0f);
            }
        }
    }
    else
    {
        // Creates fades between audio blocks 
        // if level param is changing
        while (--numSamples >= 0)
        {
            if (outR != nullptr)
            {
                *outL++ *= curGain * jmin(1.0f - curPan, 1.0f);
                *outR++ *= curGain * jmin(1.0f + curPan, 1.0f);
            }
            else
            {
                *outL++ *= curGain * jmin(1.0f - curPan, 1.0f);
            }
        }
        buffer.applyGainRamp(0, buffer.getNumSamples(), prevGain, curGain);
        prevGain = curGain;
    }
}

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

void DrumProcessor::attachMasterParams()
{
    level = parameters.getRawParameterValue("pMasterLevel");
    pan = parameters.getRawParameterValue("pMasterPan");
    muteEnabled = parameters.getRawParameterValue("pMasterMute");
}

void DrumProcessor::attachChannelParams(int midiChannel)
{
    String paramID;

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
            currentVoice->level = parameters.getRawParameterValue(paramID << channelName << "Level");
            paramID.clear();

            // Pan
            paramID << "p";
            currentVoice->pan = parameters.getRawParameterValue(paramID << channelName << "Pan");
            paramID.clear();

            // Mute
            paramID << "p";
            currentVoice->muteEnabled = parameters.getRawParameterValue(paramID << channelName << "Mute");
            paramID.clear();

            // Solo
            paramID << "p";
            currentVoice->soloEnabled = parameters.getRawParameterValue(paramID << channelName << "Solo");
            paramID.clear();

            // Learn
            paramID << "p";
            synth[midiChannel]->attachMidiLearn(parameters.getRawParameterValue(paramID << channelName << "Learn"));
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