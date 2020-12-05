
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
    void run()
    {
        while (!threadShouldExit())
        {
            if (!isPathSet) {
                setPath();
                readFromPath(path);
            }
            wait(500);
        }
    }

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
        Logger::getCurrentLogger()->writeToLog(msg);

        isPathSet = true;
    }
    //cartella kit
        // Cartella instrument
            //	cartella articulation
                //	file wav ordinati per mic velocity e indice

    // in base a cosa trovo dentro la cartella mi ricavo num velocity ranges e round robin 


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


//==========================================================================================================================

class DrumVoice : public SynthesiserVoice
{
public:
    DrumVoice()
    { }

    ~DrumVoice()
    { }

    // Check if passed sound is a correct DrumSound type.
    // Return nullptr otherwise.
    bool canPlaySound(SynthesiserSound* sound) override
    {
        return dynamic_cast<DrumSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int /*currentPitchWheelPosition*/) override
    {
        if (auto* sound = dynamic_cast<const DrumSound*> (s))
        {
            prevGain = *level;
            semitones = *coarse;
            cents = *fine;

            // Collego i puntatori ai rispettivi valori
            //auto initVoice = [this] (DrumVoice& voice)
            //{
            //	env = playingSound->getEnvelope();
            //	env->reset();
            //	env->gate(true);
            //};
            // Apro l'inviluppo
            //env = playingSound->getEnvelope();
            //env->reset();
            //env->gate(true);

            attackSamples = roundToInt(attackTime * sound->reader->sampleRate);
            releaseSamples = roundToInt(releaseTime * sound->reader->sampleRate);
            pitchRatio = std::pow(2.0, (semitones + cents * 0.01) / 12.0)
                * sound->reader->sampleRate / getSampleRate();

            sourceSamplePosition = 0.0;
            lgain = 1.0f/*velocity*/;
            rgain = 1.0f/*velocity*/;

            isInAttack = (attackSamples > 0);
            isInRelease = false;


            if (isInAttack)
            {
                attackReleaseLevel = 0.0f;
                attackDelta = (float) (pitchRatio / attackSamples);
            }
            else
            {
                attackReleaseLevel = 1.0f;
                attackDelta = 0.0f;
            }

            if (releaseSamples > 0)
                releaseDelta = (float) (-pitchRatio / releaseSamples);
            else
                releaseDelta = -1.0f;
        }
        else
        {
            jassertfalse; // this object can only play DrumSound objects!
        }

    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            isInAttack = false;
            isInRelease = true;
        }
        else
        {
            /*if (env->getState() == ADSR::env_idle)*/
            clearCurrentNote();
        }
    }

    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (auto* playingSound = static_cast<DrumSound*> (getCurrentlyPlayingSound().get()))
        {
            auto& data = *playingSound->data;
            const float* const inL = data.getReadPointer(0);
            const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

            float* outL = outputBuffer.getWritePointer(0, startSample);
            float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

            auto curPan = static_cast<float>(*pan);

            while (--numSamples >= 0)
            {
                auto pos = (int) sourceSamplePosition;
                auto alpha = (float) (sourceSamplePosition - pos);
                auto invAlpha = 1.0f - alpha;

                // just using a very simple linear interpolation here..
                float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
                float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

                l *= lgain * jmin(1.0f - curPan, 1.0f);
                r *= rgain * jmin(1.0f + curPan, 1.0f);

                if (isInAttack)
                {
                    l *= attackReleaseLevel;
                    r *= attackReleaseLevel;

                    attackReleaseLevel += attackDelta;

                    if (attackReleaseLevel >= 1.0f)
                    {
                        attackReleaseLevel = 1.0f;
                        isInAttack = false;
                    }
                }
                else if (isInRelease)
                {
                    l *= attackReleaseLevel;
                    r *= attackReleaseLevel;

                    attackReleaseLevel += releaseDelta;

                    if (attackReleaseLevel <= 0.0f)
                    {
                        stopNote(0.0f, false);
                        break;
                    }
                }

                // passaggio campione al buffer di uscita
                if (outR != nullptr)
                {
                    *outL++ += l; // * env->process();
                    *outR++ += r; // * env->process();
                }
                else
                {
                    *outL++ += (l + r) * 0.5f; // * env->process();
                }

                sourceSamplePosition += pitchRatio;

                if (sourceSamplePosition > playingSound->lenght)
                {
                    stopNote(0.0f, false);
                    //	env->gate(false);
                    break;
                }
            }

            // Levels
            auto curGain = static_cast<float>(*level);

            if (*muteEnabled) { curGain = 0; }

            if (curGain == prevGain)
                outputBuffer.applyGain(curGain);
            else
            {
                outputBuffer.applyGainRamp(0, outputBuffer.getNumSamples(), prevGain, curGain);
                outputBuffer.applyGainRamp(0, outputBuffer.getNumSamples(), prevGain, curGain);
                prevGain = curGain;
            }
        }
    }

    void pitchWheelMoved(int newPitchWheelValue) override { }

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
    float lgain = 0, rgain = 0, attackReleaseLevel = 0, attackDelta = 0, releaseDelta = 0;
    bool isInAttack = false, isInRelease = false;
    double attackTime, releaseTime;
    int attackSamples = 0, releaseSamples = 0;

    int voiceIndex;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumVoice)
};