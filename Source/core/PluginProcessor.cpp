#include "PluginProcessor.h"

//using Parameter = AudioProcessorValueTreeState::Parameter;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	// forse qua potrei inizializzare le macro?
	return new DrumProcessor();
}

AudioProcessorEditor* DrumProcessor::createEditor()
{
	return new DrumEditor(*this, parameters);
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
	AudioProcessorValueTreeState::ParameterLayout params;
	//String paramID = "p";

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
	//for (int i = 1; i < maxChannels; ++i) {
	//	String curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterFloat>
	//		(paramID << "Level",
	//			curOut << " Level",
	//			0.0f, 1.0f, 1.0f)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterFloat>
	//		(paramID << "Pan",
	//			curOut << " Pan",
	//			-1.0f, 1.0f, 0.0f)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterBool>
	//		(paramID << "Mute",
	//			curOut << " Mute",
	//			false)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterBool>
	//		(paramID << "Solo",
	//			curOut << " Solo",
	//			false)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterFloat>
	//		(paramID << "Coarse",
	//			curOut << " Coarse",
	//			-36.0f, 36.0f, 0.0f)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;

	//	params.add(std::make_unique<AudioParameterFloat>
	//		(paramID << "Fine",
	//			curOut << " Fine",
	//			-100.0f, 100.0f, 0.0f)
	//	);
	//	curOut = outputs[i];
	//	paramID.clear();
	//	paramID << "p" << curOut;
	//}

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
	// Generate instrument channels and attach parameters
	for (auto channel = 0; channel < maxMidiChannel; channel++)
	{
		if (outputs[channel] != "Empty")
		{
			Logger::getCurrentLogger()->writeToLog(outputs[channel]);

			synth.add(new DrumSynth(outputs[channel]));

			//attachChannelParams(channel);

			isChannelMuteEnabled[channel] = false;
			isSoloEnabled[channel] = false;
		}
		else
			break;
	}

	attachMasterParams();
}

DrumProcessor::~DrumProcessor()
{
}

//==============================================================================
void DrumProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	ignoreUnused(samplesPerBlock);
	auto lastSampleRate = sampleRate;
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

	for (auto midiChannel = 0; midiChannel < maxMidiChannel; ++midiChannel)
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
	//auto curMasterMute = *masterMute * 1;
	//auto curMasterMute = *masterMute > 0.5f ? true : false;

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
	//if (curMasterMute) { curMasterGain = 0; }
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



//void DrumProcessor::createParameters(AudioProcessorValueTreeState& parameters, int i)
//{
//	//	for (auto i = 0; i < maxMidiChannel; i++)
//	{
//
//		////// MASTER =======================================================================================
//
//		if (!isMasterSet) {
//
//			// level
//
//			//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//			//	                            ("pMasterLevel",								// parameterID
//			//								 "Master Level",								// parameter name
//			//								 0.0f, 1.0f, 1.0f));
//
//			//addParameter(masterLevel = new juce::AudioParameterFloat
//			//("pMasterLevel",
//			//	"Master Level",
//			//	0.0f,
//			//	1.0f,
//			//	1.0f));
//
//			//// pan
//			//addParameter(masterLevel = new juce::AudioParameterFloat
//			//("pMasterPan",
//			//	"Master Pan",
//			//	-1.0f,
//			//	1.0f,
//			//	0.0f));
//
//			//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//			//								 ("pMasterPan",		// parameterID
//			//								 "Master Pan",									// parameter name
//			//								 NormalisableRange<float>(-1.0f, 1.0f),			// range [-1; +1]
//			//								 0.0f,											// default value
//			//								 "",											// parameter label (suffix)
//			//								 AudioProcessorParameter::genericParameter,	    // category
//			//								 nullptr, 
//			//							     nullptr)
//
//											 //[](float value)								// valueToText
//												//{ return String(value * 1000.0f); },
//											 //[](const String& text)							// textToValue
//												//{ return text.getFloatValue() / 1000.0f; }))
//				//);
//
//			// mute
//			//parameters.createAndAddParameter(std::make_unique<AudioParameterBool>
//			//	("pMasterMute",									 // parameterID
//			//		"Master Mute",									 // parameter name
//			//		false,											 // default value
//			//		"",										 // parameter label (suffix)
//			//		nullptr,
//			//		nullptr
//
//			//		//std::function<String (bool value)>								 // stringFromBool
//			//		   //{ return value ? "on" : "off"; },		
//			//		//[](const String& text)							 // boolFromString
//			//		   //{
//			//		   //if (text.toLowerCase() == "mute on") return true;
//			//		   //if (text.toLowerCase() == "mute off") return false;
//			//		   //return false; //?
//			//		   //}
//			//		)
//			//);
//
//			isMasterSet = true;
//		}
//
//		//////// CHANNELS =======================================================================================
//		//
//
//		String curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//		// Channel =========================================
//
//		// level
//		//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//		//								(paramID << "Level",							// parameterID
//		//								 curOut << " Level",							// parameter name
//		//								 0.0f,											// min value
//		//								 1.0f,											// max value
//		//								 1.0f)											// default value
//
//		//);
//
//		// level
//		//parameterLayout.add(std::make_unique<Parameter>
//		//	(paramID << "Level",							// parameterID
//		//		curOut << " Level",							// parameter name
//		//		"dB",											// parameter label (suffix)
//		//		NormalisableRange<float>(0.0f, 1.0f),			// range
//		//		1.0f,											// default value
//		//		nullptr,
//		//		nullptr)
//		//);
//		curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//		// pan
//		parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//			(paramID << "Pan",								// parameterID
//				curOut << " Pan",							    // parameter name
//				NormalisableRange<float>(-1.0f, 1.0f),		    // range [-1; +1]
//				0.0f,											// default value
//				"",											// parameter label (suffix)
//				AudioProcessorParameter::genericParameter,		 // category
//
//
//
//				//[](float value)								// valueToText
//				   //{ return String(value * 1000.0f); },
//				nullptr,
//
//				/*[](const String& text)							// textToValue
//				   { return text.getFloatValue() / 1000.0f; })*/
//				nullptr)
//		);
//		curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//		// mute
//		parameters.createAndAddParameter(std::make_unique<AudioParameterBool>
//			(paramID << "Mute",								 // parameterID
//				curOut << " Mute",								 // parameter name
//				false,											 // default value
//				"",											 // parameter label (suffix)
//				nullptr, nullptr)
//
//			//   [](float value)								 // valueToText
//				  //{ return value > 0.5 ? "on" : "off"; },		
//
//			   //[](const String& text)							 // textToValue
//				  //{
//				  //if (text.toLowerCase() == "mute on") return 0.0f;
//				  //if (text.toLowerCase() == "mute off") return 1.0f;
//				  //return 0.0f;
//				  //},	
//
//			   //false,											 // is meta
//			   //true,											 // is automatable
//			   //true,                                           // is discrete
//			   //AudioProcessorParameter::genericParameter,		 // category
//			   //true)											 // is boolean
//		);
//		curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//		// solo
//		parameters.createAndAddParameter(std::make_unique<AudioParameterBool>
//			(paramID << "Solo",									 // parameterID
//				curOut << " Solo",									 // parameter name
//				false,											 // default value
//				"",											 // parameter label (suffix)
//				nullptr, nullptr)
//
//			//   [](float value)								 // valueToText
//				  //{ return value > 0.5 ? "on" : "off"; },		
//
//			   //[](const String& text)							 // textToValue
//				  //{
//				  //if (text.toLowerCase() == "solo on") return 0.0f;
//				  //if (text.toLowerCase() == "solo off") return 1.0f;
//				  //return 0.0f;
//				  //},	
//
//			   //false,											 // is meta
//			   //true,											 // is automatable
//			   //true,                                           // is discrete
//			   //AudioProcessorParameter::genericParameter,		 // category
//			   //true)											 // is boolean
//		);
//		curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//
//		// Tuning ======================================
//
//		// coarse
//		parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//			(paramID << "Coarse",
//				curOut << " Coarse",
//				NormalisableRange<float>(-36.0f, 36.0f, 1.0f),
//				0.0f,
//				String("st"),
//				AudioProcessorParameter::genericParameter,		 // category
//				nullptr, nullptr)
//
//
//			// [](float value)
//			// {
//			   ////value = ((int)(value * 24.0f) - 12.0f);
//			   //if (value > 0.0f)
//			   //	return String("+") + String(value, 0);
//			   //return String(value, 0);
//			// },
//
//			// [](const String& text)
//			// {
//			   //return text.getFloatValue();
//			// })
//		);
//		curOut = outputs[i];
//		paramID.clear();
//		paramID << "p" << curOut;
//
//		// fine
//		parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//			(paramID << "Fine",
//				curOut << " Fine",
//				NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
//				0.0f,
//				String("cents"),
//				AudioProcessorParameter::genericParameter,		 // category
//				nullptr, nullptr)
//			/*0.0f,
//
//			[](double value)
//			{
//				if (value > 0.0f)
//					return String("+") + String(value, 0);
//				return String(value, 0);
//			},
//
//			[](const String& text)
//			{
//				return (text.getFloatValue() + 12.0f) / 24.f;
//			})*/
//		);
//		curOut = outputs[i];
//		paramID.clear();
//
//		//paramID << "e" << curOut;
//		//// Envelope ==================================
//
//		//// attack
//		//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>
//		//	                            (paramID << "Attack",									// parameterID
//		//								 curOut << " Env Attack",								// parameter name
//		//								 "ms",											// parameter label (suffix)
//		//								 //NormalisableRange<float>(0.0f, 0.25f),			// range [0 ms; 250 ms]
//		//								 AudioProcessorParameter::genericParameter)		 // category
//		//	
//		//								 //0.0f,											// default value
//
//		//								 //[](float value)								// valueToText
//		//									//{ return String(value * 1000.0f); },
//
//		//								 //[](const String& text)							// textToValue
//		//									//{ return text.getFloatValue() / 1000.0f; })
//		//);
//		//curOut = outputs[i];
//		//paramID.clear();
//
//		//paramID << "e" << curOut;
//		//// hold
//		//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>(paramID << "Hold",									// parameterID
//		//								 curOut << " Env Hold",								// parameter name
//		//								 "ms",											// parameter label (suffix)
//		//								 //NormalisableRange<float>(0.005f, 1.0f),		// range [5 ms; 1 s]
//		//								 AudioProcessorParameter::genericParameter)		 // category
//		//	
//		//								 //0.005f,										// default value
//
//		//								 //[](float value)								// valueToText
//		//									//{ return String(value * 1000.0f); },
//
//		//								 //[](const String& text)							// textToValue
//		//									//{ return text.getFloatValue() / 1000.0f; })
//		//);
//		//curOut = outputs[i];
//		//paramID.clear();
//
//		//paramID << "e" << curOut;
//		//// release
//		//parameters.createAndAddParameter(std::make_unique<AudioParameterFloat>(paramID << "Release",								// parameterID
//		//								 curOut << " Env Release",							// parameter name
//		//								 "ms",											// parameter label (suffix)
//		//								 //NormalisableRange<float>(0.005f, 5.0f),		// range [5 ms; 5 s]
//		//								 AudioProcessorParameter::genericParameter)		 // category
//
//		//								 //5.0f,											// default value
//		//								 //
//		//								 //[](float value)								// valueToText
//		//									//{ return String(value * 1000.0f); },
//
//		//								 //[](const String& text)							// textToValue
//		//									//{ return text.getFloatValue() / 1000.0f; })
//		//);
//		//curOut = outputs[i];
//		//paramID.clear();
//		////==========================================================================================================================
//	}
//}

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
	paramID.clear();
	paramID = "p";

	// Parametri Canali
	auto channelName = outputs[midiChannel];
	auto numVoices = synth[midiChannel]->getNumVoices();

	//		if (numVoices != 0) {
	//			for (auto i = 0; i < numVoices; i++)
	//			{
	//				auto currentVoice = static_cast<DrumVoice*>(synth[midiChannel]->getVoice(i));
	//				auto sampleRate = currentVoice->getSampleRate();
	//				paramID << "p";
	//				
	//				level[midiChannel] = parameters.getRawParameterValue    (paramID << channelName << "Level");
	//				paramID.clear();
	//				paramID << "p";
	//	
	//				isChannelMuteEnabled[midiChannel] = 
	//					parameters.getRawParameterValue						(paramID << channelName << "Mute");
	//				paramID.clear();
	//				
	//				isSoloEnabled[midiChannel] = 
	//					parameters.getRawParameterValue						(paramID << channelName << "Solo");
	//				paramID.clear();
	//				
	//				paramID << "p";
	//				currentVoice->pan = parameters.getRawParameterValue		(paramID << channelName << "Pan");
	//				paramID.clear();
	//				
	//				paramID << "p";
	//				currentVoice->coarse = parameters.getRawParameterValue	(paramID << channelName << "Coarse"));
	//				paramID.clear();
	//				
	//				paramID << "p";
	//				currentVoice->fine = parameters.getRawParameterValue	(paramID << channelName << "Fine");
	//				paramID.clear();
	//				
	//				paramID << "e";
	////				auto env = currentVoice->env;
	//				auto curAttack = parameters.getRawParameterValue		(paramID << channelName << "Attack");
	////				env->setAttackRate(*curAttack * sampleRate);
	//				paramID.clear();
	//				
	//				paramID << "e";
	//				auto curHold = parameters.getRawParameterValue			(paramID << channelName << "Hold");
	////				env->setDecayRate(*curHold * sampleRate);
	//				paramID.clear();
	//				
	//				paramID << "e";
	//				auto curRelease = parameters.getRawParameterValue		(paramID << channelName << "Release");
	////				env->setReleaseRate(*curRelease * sampleRate);
	//				paramID.clear();
	//			}
	//		}
}
