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
    for (int i = 0; i < outputs.size(); i++)
    {
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
    : juce::AudioProcessor(BusesProperties()
        .withOutput("Master", juce::AudioChannelSet::stereo(), true)
        //.withOutput("Kick", AudioChannelSet::stereo(), false)
        //.withOutput("Snare", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 1", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 2", AudioChannelSet::stereo(), false)
        //.withOutput("Tom 3", AudioChannelSet::stereo(), false)
        //.withOutput("HiHat", AudioChannelSet::stereo(), false)
        //.withOutput("Overhead", AudioChannelSet::stereo(), false)
    )
    , parameters(*this, nullptr, juce::Identifier("DrumSamplerVTS"), createParameterLayout())
{
    // Retrieve drumset info
    outputs = drumsetInfo.getActiveOutputs();
    maxOutputs = outputs.size();
    int note = startNote;

    // Generate a synth for each active output, then attach parameters
    for (auto channel = 0; channel < maxOutputs; channel++)
    {
        DBG(outputs[channel]);
        synth.add(new DrumSynth(parameters, outputs[channel], note++));
        attachChannelParams(channel);
    }

    attachMasterParams();
}

DrumProcessor::~DrumProcessor()
{ }

//==============================================================================
void DrumProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlock = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    DBG(message);

    auto lastSampleRate = sampleRate;
    outputs = drumsetInfo.getActiveOutputs();
    prevGain = *level;

    // Prepare outputs
    for (auto midiChannel = 0; midiChannel < maxOutputs; ++midiChannel)
    {
        synth[midiChannel]->setCurrentPlaybackSampleRate(lastSampleRate);

        // If host changes block size while plugin is running,
        // update size and recreate bufferswith new one.
        if (pluginIsInit && lastBlockSize != samplesPerBlock) {
            buffers.clear();
            buffersAllocated = false;
        }

        if (!buffersAllocated && sampleBlockInitCount == 0)
        {
            ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(outputs[midiChannel],
                                                                               getMainBusNumOutputChannels(),
                                                                               samplesPerBlock);
            buffers.add(newBuffer);
            lastBlockSize = samplesPerBlock;
        }
    }

    // This method gets called 4 times.
    // In the first 2 calls samplesPerBlock has an arbitrary value,
    // then we wait for the later calls in order
    // to get the actual block size from host.
    if (sampleBlockInitCount > 0)
    {
        sampleBlockInitCount--;
    }
    else
    {
        buffersAllocated = true;
        pluginIsInit = true;
    }
}

void DrumProcessor::releaseResources()
{
    DBG("Releasing audio resources");
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
    auto totalNumOutputChannels = getMainBusNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    //auto busCount = getBusCount(false);

    // Clear buffer before fill it
    for (auto i = 0; i < totalNumOutputChannels; i++)
        buffer.clear(i, 0, numSamples);

    // Copy param values
    auto isMuteEnabled = *muteEnabled > 0.5f ? true : false;
    auto curPan = pan->load();
    auto curGain = isMuteEnabled ? 0.0f : level->load();

    if (buffersAllocated) // ignore synth buffers if not allocated yet
    {
        // Fill each synth buffer
        for (auto i = 0; i < maxOutputs; i++)
        {
            currentBuffer = buffers[i];

            if (currentBuffer == nullptr)
                jassertfalse; // currentBuffer has to exist. @todo: gestire eccezione

            currentBuffer->getAudioSampleBuffer()->clear();

            // Pass midi messages to each synth so they can fill their buffer
            synth[i]->renderNextBlock(*currentBuffer->getAudioSampleBuffer(), midiBuffer, 0, buffer.getNumSamples());

            // Add to main output buffer
            for (auto ch = 0; ch < numChannels; ch++)
                buffer.addFrom(ch, 0, currentBuffer->getAudioSampleBuffer()->getReadPointer(ch), numSamples);
        }
    }

    // Write main buffer to output applying levels
    float* outL = buffer.getWritePointer(0);
    float* outR = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    while (--numSamples >= 0)
    {
        if (outR != nullptr)
        {
            *outL++ *= jmin(1.0f - curPan, 1.0f);
            *outR++ *= jmin(1.0f + curPan, 1.0f);
        }
        else
        {
            *outL++ *= jmin(1.0f - curPan, 1.0f);
        }
    }

    if (curGain == prevGain)
    {
        buffer.applyGain(curGain);
    }
    else
    {
        // Creates fades between audio blocks 
        // if level param is changing
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

    if (numVoices != 0)
    {
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

/*
    Check if there are buffers to free
*/
void DrumProcessor::checkForBuffersToFree()
{
    // Iterate in reverse to avoid corrupting the array index access if we remove items
    for (auto i = buffers.size(); --i >= 0;)
    {
        ReferenceCountedBuffer::Ptr buffer (buffers.getUnchecked (i));

        if (buffer->getReferenceCount() == 2)
            buffers.remove (i);
    }
}
