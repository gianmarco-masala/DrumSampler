#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrumSound.h"


class DrumSynth : public Synthesiser
{
public:
    enum
    {
        maxSoundsPerRange = 1,
        maxVoices = 10,
        numVelocityRange = 8
    };

    //==============================================================================
    DrumSynth(String name)
    {
        chName = name;
        //note = nota predefinita;

        for (int i = maxVoices; --i >= 0;)
            addVoice(new DrumVoice());

        addSounds();
    }

    ~DrumSynth()
    { }

    void handleMidiEvent(const MidiMessage& m) override
    {
        const int channel = m.getChannel();

        if (m.isNoteOn())
        {
            if (isMidiLearning)
            {
                midiLearn(m);
            }
            else
            {
                noteOn(channel, m.getNoteNumber(), m.getFloatVelocity());
                voicesToLog(m.getNoteNumber(), m.getFloatVelocity());
            }
        }
        else if (m.isNoteOff())
        {
            //	noteOff(channel, m.getNoteNumber(), m.getFloatVelocity(), true);
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            allNotesOff(channel, true);
        }
        else if (m.isPitchWheel())
        {
            //const int wheelPos = m.getPitchWheelValue();
            //lastPitchWheelValues[channel - 1] = wheelPos;
            //handlePitchWheel(channel, wheelPos);
        }
        else if (m.isAftertouch())
        {
            //			handleAftertouch(channel, m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isChannelPressure())
        {
            //			handleChannelPressure(channel, m.getChannelPressureValue());
        }
        else if (m.isController())
        {
            //			handleController(channel, m.getControllerNumber(), m.getControllerValue());
        }
        else if (m.isProgramChange())
        {
            //			handleProgramChange(channel, m.getProgramChangeNumber());
        }
    }

    void noteOn(const int midiChannel,
                 const int midiNoteNumber,
                 const float velocity)	override
    {
        const ScopedLock sl(lock);

        for (auto* soundSource : sounds)
        {
            auto* const sound = static_cast<DrumSound* const> (soundSource);

            if (sound->appliesTo(midiNoteNumber, velocity) && sound->appliesToChannel(midiChannel))
            {
                // If hitting a note that's still ringing, stop it first (it could be
                // still playing because of the sustain or sostenuto pedal).
                for (auto* voice : voices)
                    if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel(midiChannel))
                        stopVoice(voice, 1.0f, true);

                startVoice(findFreeVoice(sound, midiChannel, midiNoteNumber, shouldStealNotes),
                           sound, midiChannel, midiNoteNumber, velocity);
            }
        }
    }

    void midiLearn(const MidiMessage& msg)
    {
        note = msg.getNoteNumber();
        DBG(note);
        for (auto* soundSource : sounds)
        {
            auto* const sound = static_cast<DrumSound* const> (soundSource);
            sound->setMidiNote(note);
        }
        isMidiLearning = false;
    }

    void setMutingEnabled(const bool shouldBeEnabled) { mutingEnabled = shouldBeEnabled; }

    bool isMidiLearning = false;

private:
    void addSounds()
    {
        Range<float> velocityRange;
        float velocityTot = 1.0f;
        float start = 1.0f / 128.0f;
        float const lenght = velocityTot / numVelocityRange;

        for (float range = 1; range <= numVelocityRange; range++)
        {
            velocityRange.setStart(start);
            velocityRange.setEnd(start + lenght);

            for (index = 1; index <= maxSoundsPerRange; index++) {
                addSound(sound = new DrumSound());
                sound->setName(chName);
                sound->setVelocityRange(velocityRange);
                sound->setIndex(index);
                sound->setMidiNote(note);
                //	sound->workingDirectory = file->getCurrentWorkingDirectory();
            }
            start += lenght;
            if (start > velocityTot)
                break;
        }
    }

    void voicesToLog(int midiNote, float velocity)
    {
        //		if(velocity > 0.0f)
        //			Logger::getCurrentLogger()->writeToLog("                     <<<<<< tasto premuto");

        int voiceIndex;
        auto numVoices = getNumVoices();
        String msg;

        for (int i = numVoices - 1; i >= 0; i--)
        {
            voice = voices[i];
            auto voiceActive = voice->isVoiceActive();
            if (voiceActive) {
                voiceIndex = i;
                msg << "voice: " << voiceIndex << ", startNote: " << midiNote << "\n";
                Logger::getCurrentLogger()->writeToLog(msg);
                break;
            }
        }
    }

    DrumSound* sound;
    SynthesiserVoice* voice;
    std::unique_ptr<File> file;
    bool shouldStealNotes = true;
    bool mutingEnabled = false;
    String chName;
    int index, note;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSynth)
};

//kit
//	instrument
//		polyphony voice management
//			articulations
//				velocity mapping
//				round robin mapping (random exclusive)
//				microphone mapping (rientri)

// informarsi su lettura file interleaved (file unico dal quale fare dispatch delle voci) come metodo alternativo ai tanti file di rientri.

// (FATTO) impostare schema

// testare i metodi man mano

// ottimizzazione uso cpu