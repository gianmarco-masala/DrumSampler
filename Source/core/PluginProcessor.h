#pragma once

#define MAX_INSTRUMENTS 8

#include "../JuceLibraryCode/JuceHeader.h"
#include "../gui/PluginEditor.h"
#include "DrumSynth.h"
//#include "../envelope/ADSR.h"

struct ChannelNames {
	const String outputs[MAX_INSTRUMENTS] =
	{
		"Kick",
		"Snare",
		"Tom1",
		"Tom2",
		"Tom3",
		"HiHat",
		"Overhead",
		"Empty",
	};
};

class DrumProcessor : public AudioProcessor
{
public:
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
	void setCurrentProgram(int) override {}
	const String getProgramName(int) override { return {}; }
	void changeProgramName(int, const String&) override {}

	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	bool canAddBus(bool isInput) const override
	{
		return (!isInput && getBusCount(false) < MAX_INSTRUMENTS);
	}
	bool canRemoveBus(bool isInput) const override
	{
		return (!isInput && getBusCount(false) > 1);
	}

	// CUSTOM METHODS
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
		for (auto channel = 0; channel < MAX_INSTRUMENTS; channel++)
		{
			if (isSoloEnabled[channel])
				return &channel;
		}
		return nullptr;
	}

	bool allSoloDisabled()
	{
		for (auto channel = 0; channel < MAX_INSTRUMENTS; channel++)
		{
			if (chNames.outputs[channel] == "Empty")
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

	OwnedArray<DrumSynth> synth;
	ChannelNames chNames;

private:
	juce::AudioProcessorValueTreeState parameters;
	//UndoManager undoManager;
	int soloChannel;
	bool isConstructorInitialised;

	// Pointers to parameters, used by processor
	std::atomic<float>* level[MAX_INSTRUMENTS];
	std::atomic<float>* pan[MAX_INSTRUMENTS];
	std::atomic<float>* masterLevel = nullptr;
	std::atomic<float>* masterPan = nullptr;
	std::atomic<float>* masterMute = nullptr;

	//==============================================================================

	// Creates fades in audio blocks when changing levels
	float previousChannelGain[MAX_INSTRUMENTS];
	float previousMasterGain;

	// Mute and solo booleans
	bool isChannelMuteEnabled[MAX_INSTRUMENTS];
	bool isSoloEnabled[MAX_INSTRUMENTS];
	bool isMasterMuteEnabled = false;

	// Need this to prevent creating master's parameters twice
	bool isMasterSet = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumProcessor)
};