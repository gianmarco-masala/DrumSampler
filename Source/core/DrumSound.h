
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "DrumSynth.h"

class DrumSound
    : public SynthesiserSound
    , private Thread
{
public:
    DrumSound() : Thread("DrumSound Thread")
    {
        formatManager.registerBasicFormats();
        startThread();
    }

    ~DrumSound()
    {
        stopThread(4000);
    }

    void setName(String name) { chName = name; };

    void setIndex(int i) { index = i; }

    void setVelocityRange(Range<float> range)
    {
        auto start = range.getStart();
        auto end = range.getEnd();

        velocity.setStart(start);
        velocity.setEnd(end);
    }

    void setMidiNote(int noteToSet)
    {
        playingMidiNote = noteToSet;
        return;
    }

    bool appliesToNote(int midiNoteNumber) override
    {
        if (midiNoteNumber == playingMidiNote)
            return true;
        else
            return false;
    }

    bool appliesTo(int midiNoteNumber, float vel)
    {
        bool appliesToMidiNote = appliesToNote(midiNoteNumber);
        bool isInVelocityRange = this->velocity.contains(vel);

        return appliesToMidiNote && isInVelocityRange;
    }

    bool appliesToChannel(int midiChannel) override { return true; }

    File workingDirectory;

private:
    /*
    * Handles thread execution inside a continous loop.
    */
    void run()
    {
        while (!threadShouldExit())
        {
            if (!isPathSet)
            {
                setPath();
                readFromPath(path);
            }
            wait(500);
        }
    }

    /*
    * Sets the reference path for this sound.
    */
    void setPath()
    {
        String msg;
        auto parentDir = workingDirectory.getCurrentWorkingDirectory();
        auto dir = parentDir.getChildFile("../../../../../Resources/Samples");

        // Formato nome file: NomePezzo_articolazione_indice_velocity.estensione
        // NomePezzo: Kick, Snare, Hi-Hat ...
        // indice: 1, 2, 3 ...
        // velocity: il limite inferiore del range
        // estensione: wav

        path << dir.getFullPathName() << dir.getSeparatorChar()
            << chName << "_"
            << index << "_"
            << roundFloatToInt(velocity.getStart() * 127)
            << ".aif";

        msg << "\nLoading Sample: \n" << path << "\n";
        DBG(msg);

        isPathSet = true;
    }
    //cartella kit
        // Cartella instrument
            //	cartella articulation
                //	file wav ordinati per mic velocity e indice

    // in base a cosa trovo dentro la cartella mi ricavo num velocity ranges e round robin 


    /*
    * Reads a file from given path and loads it to a local AudioBuffer<float>
    */
    void readFromPath(String pathToOpen)
    {
        if (pathToOpen.isNotEmpty())
        {
            File file(pathToOpen);
            reader.reset(formatManager.createReaderFor(file));

            if (reader.get() != nullptr)
            {
                if (reader->sampleRate > 0 && reader->lengthInSamples > 0)
                {
                    lenght = (int) reader->lengthInSamples;

                    data.reset(new AudioBuffer<float>(jmin(2, (int) reader->numChannels), lenght + 4));

                    reader->read(data.get(),
                        0,
                        lenght,
                        0,
                        true,
                        true);
                }
            }
            else
            {
                jassertfalse; // File was not found!
            }
        }
    }

    friend class DrumVoice;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReader> reader;
    std::unique_ptr<AudioBuffer<float>> data;
    BigInteger midiNotes;
    Range<float> velocity;
    String path, chName;
    int index;
    int playingMidiNote = 0;
    int midiRootNote = 60;
    int maxSampleLengthSeconds = 30;
    int lenght = 0;
    bool isPathSet = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSound)
};



class DrumVoice : public SynthesiserVoice
{
public:
    DrumVoice() { }

    ~DrumVoice() { }

    /* 
    * Checks if given sound has correct DrumSound type.
    * Returns nullptr otherwise.
    * Overrides juce:Synthesiser method
    */
    bool canPlaySound(SynthesiserSound* sound) override { return dynamic_cast<DrumSound*> (sound) != nullptr; }

    /*
    * Prepares synth to start a note setting up values
    * Overrides juce:Synthesiser method
    */
    void startNote(int /*midiNoteNumber*/, float velocity, SynthesiserSound* s, int /*currentPitchWheelPosition*/) override
    {
        if (auto* sound = dynamic_cast<const DrumSound*> (s))
        {
            semitones = *coarse;
            cents = *fine;
            sourceSamplePosition = 0.0;
            pitchRatio = std::pow(2.0, (semitones + cents * 0.01) / 12.0)
                * sound->reader->sampleRate / getSampleRate();
        }
        else
        {
            jassertfalse; // this object can only play DrumSound objects!
        }

    }

    /*
    * Stops the current playing note. 
    * We're actually not using this because it does not fit plugin's goal
    */
    void stopNote(float velocity, bool allowTailOff) override
    {
        ignoreUnused(velocity, allowTailOff);
        clearCurrentNote();
    }

    /*
    * Renders the current sample block for this synth.
    * Overrides juce:Synthesiser method
    */
    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (auto* playingSound = static_cast<DrumSound*> (getCurrentlyPlayingSound().get()))
        {
            auto& data = *playingSound->data;
            isMuteEnabled = *muteEnabled > 0.5f ? true : false;
            isSoloEnabled = *soloEnabled > 0.5f ? true : false;
            auto curPan = pan->load();
            auto curGain = isMuteEnabled ? 0.0f : level->load();

            const float* const inL = data.getReadPointer(0);
            const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

            float* outL = outputBuffer.getWritePointer(0, startSample);
            float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

            // Fill buffer with samples
            while (--numSamples >= 0)
            {
                auto pos = (int) sourceSamplePosition;
                auto alpha = (float) (sourceSamplePosition - pos);
                auto invAlpha = 1.0f - alpha;

                // just using a very simple linear interpolation here..
                float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
                float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

                if (outR != nullptr)
                {
                    *outL++ += l * jmin(1.0f - curPan, 1.0f);
                    *outR++ += r * jmin(1.0f + curPan, 1.0f);
                }
                else
                {
                    *outL++ += (l + r) * 0.5f;
                }

                // Increment sample position and break
                // if we reached sound total length
                sourceSamplePosition += pitchRatio;
                if (sourceSamplePosition >= playingSound->lenght)
                {
                    stopNote(0.0f, false);
                    break;
                }
            }

            // Level
            if (curGain == prevGain)
            {
                outputBuffer.applyGain(curGain);
            }
            else
            {
                // Creates fades between audio blocks 
                // if level param is changing
                outputBuffer.applyGainRamp(0, outputBuffer.getNumSamples(), prevGain, curGain);
                prevGain = curGain;
            }
        }
    }

    /*
    * Handle pitch wheel control.
    * We're actually not using this because it does not fit plugin's goal
    */
    void pitchWheelMoved(int newPitchWheelValue) override { }

    /*
    * Handle incoming controller messages.
    */
    void controllerMoved(int controllerNumber, int newControllerValue) { }

    std::atomic<float>* level;
    std::atomic<float>* pan;
    std::atomic<float>* coarse;
    std::atomic<float>* fine;
    std::atomic<float>* muteEnabled;
    std::atomic<float>* soloEnabled;
    float prevGain;

private:
    float semitones = 0.0;
    float cents = 0.0;
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    bool isMuteEnabled = false;
    bool isSoloEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumVoice)
};