#pragma once

#include <JuceHeader.h>
#include "../core/PluginEditor.h"
#include "DrumSynth.h"
#include "../utils/DrumsetXmlHandler.h"
#include "../utils/ReferenceCountedBuffer.h"

class DrumProcessor : public AudioProcessor
{
public:
    enum PluginOptions
    {
        maxMidiChannel = 8,
        startNote = 72
    };

    DrumProcessor();
    ~DrumProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override { }
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int, const String&) override { }

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool canAddBus(bool isInput) const override { return (!isInput && getBusCount(false) < maxMidiChannel); }
    bool canRemoveBus(bool isInput) const override { return (!isInput && getBusCount(false) > 1); }

    juce::OwnedArray<DrumSynth> synth;
    juce::StringArray outputs;

private:
    /*
    * Attach registered parameter values to
    * corresponding pointers for master channel
    */
    void attachMasterParams();

    /*
    * Attach registered parameter values to
    * corresponding pointers for given inst channel
    */
    void attachChannelParams(int i);

    /*
    * Returns true if the channel at the given
    * index has solo enabled, false otherwise.
    */
    bool checkSoloChannel(int index);

    /*
    * Returns true if solo is enabled on some channel,
    * false otherwise.
    */
    bool checkSoloEnabled();
    // Check if there are buffers to free
    void checkForBuffersToFree();


    juce::ReferenceCountedArray<ReferenceCountedBuffer> buffers;
    ReferenceCountedBuffer::Ptr currentBuffer;
    juce::AudioProcessorValueTreeState parameters;
    DrumsetXmlHandler drumsetInfo;
    //UndoManager undoManager;
    Array<std::atomic<float>*> soloChannels;
    int maxOutputs;
    bool buffersAllocated = false;
    bool pluginIsInit = false;
    int sampleBlockInitCount = 2;

    // Pointers to parameters, used by processor
    std::atomic<float>* level = nullptr;
    std::atomic<float>* pan = nullptr;
    std::atomic<float>* muteEnabled = nullptr;

    float prevGain;
    int lastBlockSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumProcessor)
};