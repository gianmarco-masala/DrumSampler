#pragma once

#define MAX_INSTRUMENTS 8

#include "../JuceLibraryCode/JuceHeader.h"
#include "../gui/PluginEditor.h"
#include "DrumSynth.h"
//#include "../envelope/ADSR.h"

class DrumProcessor : public AudioProcessor
{
public:
	enum
	{
		maxMidiChannel = 8
	};

	DrumProcessor();
	~DrumProcessor();

	//==============================================================================

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

	//==============================================================================

	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; }

	//==============================================================================

	const String getName() const override { return JucePlugin_Name; }

	bool acceptsMidi() const override { return true; }
	bool producesMidi() const override { return false; }
	bool isMidiEffect() const override { return false; }
	double getTailLengthSeconds() const override { return 0.0; }

	//==============================================================================

	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int) override {}
	const String getProgramName(int) override { return {}; }
	void changeProgramName(int, const String&) override {}

	//==============================================================================
	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	bool canAddBus(bool isInput) const override
	{
		return (!isInput && getBusCount(false) < maxMidiChannel);
	}
	bool canRemoveBus(bool isInput) const override
	{
		return (!isInput && getBusCount(false) > 1);
	}

	//void createParameters(AudioProcessorValueTreeState& parameters, int i);

	void attachMasterParams();
	void attachChannelParams(int i);

	// Used in GUI to trigger mute and solo.
	// A negative index value will trigger the master channel
	void setMuteEnabled(const bool shouldBeEnabled, int index)
	{
		if (index >= 0)
			isChannelMuteEnabled[index] = shouldBeEnabled;
		else
			isMasterMuteEnabled = shouldBeEnabled;
	}

	void setSoloEnabled(const bool shouldBeEnabled, int index)
	{
		if (index >= 0)
			isSoloEnabled[index] = shouldBeEnabled;
	}

	int* getSoloChannel()
	{
		for (auto channel = 0; channel < maxMidiChannel; channel++)
		{
			if (isSoloEnabled[channel])
				return &channel;
		}
		return nullptr;
	}

	bool allSoloDisabled()
	{
		for (auto channel = 0; channel < maxMidiChannel; channel++)
		{
			if (outputs[channel] == "Empty")
				break;
			else
			{
				if (isSoloEnabled[channel])
					return false;
			}
		}
		return true;
	}

	void setLearnFromMidi(bool isLearnSet, int channel) { synth[channel]->isMidiLearning = isLearnSet; }

	const String outputs[maxMidiChannel] =
	{
		"Kick",
		"Snare",
		//"Tom 1",
		//"Tom 2",
		//"Tom 3",
		//"HiHat",
		//"Overhead",
		"Empty",
		"Empty",
		"Empty",
		"Empty",
		"Empty",
		"Empty",
	};
	String paramID = "p";

	OwnedArray<DrumSynth> synth;

private:
	juce::AudioProcessorValueTreeState parameters;
	//UndoManager undoManager;
	String channelName;
	int soloChannel;
	bool isConstructorInitialised;

	// Pointers to parameters, used by processor
	//std::atomic<float>* level[maxMidiChannel];
	//std::atomic<float>* pan[maxMidiChannel];
	std::atomic<float>* masterLevel = nullptr;
	std::atomic<float>* masterPan = nullptr;
	std::atomic<float>* masterMute = nullptr;

	//==============================================================================

	// Creates fades in audio blocks when changing levels
	float previousChannelGain[maxMidiChannel];
	float previousMasterGain;

	// Mute and solo booleans
	bool isChannelMuteEnabled[maxMidiChannel];
	bool isSoloEnabled[maxMidiChannel];
	bool isMasterMuteEnabled = false;

	// Need this to prevent creating master's parameters twice
	bool isMasterSet = false;

	//juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout = {
	//	std::make_unique<AudioParameterFloat>
	//						   ("pMasterLevel",
	//							"Master Level",
	//							0.0f, 1.0f, 1.0f),
	//	std::make_unique<AudioParameterFloat>
	//								 ("pMasterPan",
	//								 "Master Pan",
	//								 NormalisableRange<float>(-1.0f, 1.0f),			// range [-1; +1]
	//								 0.0f,											// default value
	//								 "",											// parameter label (suffix)
	//								 AudioProcessorParameter::genericParameter,	    // category
	//								 nullptr,
	//								 nullptr)

	//};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumProcessor)
};