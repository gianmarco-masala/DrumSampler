#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrumSound.h"


class DrumSynth : public Synthesiser
{
public:
    enum
    {
        maxSoundsPerRange = 1,
        maxVoices = 100,
        numVelocityRange = 2
    };

    //==============================================================================
    DrumSynth(
        AudioProcessorValueTreeState& vts,
        const String name,
        const int defaultNote
    )
        : parameters(vts)
    {
        chName = name;
        note = defaultNote;

        for (int i = maxVoices; --i >= 0;)
            addVoice(new DrumVoice());

        addSounds();
    }

    ~DrumSynth()
    { }

    /*
    * Handles incoming midi events inside the midi buffer
    * passed by the processor.
    * Override of juce:Synthesiser method.
    */
    void handleMidiEvent(const MidiMessage& m) override
    {
        const int channel = m.getChannel();

        if (m.isNoteOn())
        {
            if (*learnEnabled)
            {
                String paramId = "p";
                midiLearn(m);
                parameters.getParameter(paramId << chName << "Learn")->setValueNotifyingHost(false);
            }
            else
            {
                noteOn(channel, m.getNoteNumber(), m.getFloatVelocity());
                voicesToLog(m.getNoteNumber(), m.getFloatVelocity());
            }
        }
        else if (m.isNoteOff())
        {
            //DBG("is note off");
            //noteOff(channel, m.getNoteNumber(), m.getFloatVelocity(), true);
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
            //handleAftertouch(channel, m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isChannelPressure())
        {
            //handleChannelPressure(channel, m.getChannelPressureValue());
        }
        else if (m.isController())
        {
            //handleController(channel, m.getControllerNumber(), m.getControllerValue());
        }
        else if (m.isProgramChange())
        {
            //handleProgramChange(channel, m.getProgramChangeNumber());
        }
    }

    /*
    * Triggers a note on message.
    * Override of juce:Synthesiser method.
    */
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
                auto voice = findFreeVoice(sound, midiChannel, midiNoteNumber, shouldStealNotes);
                startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
            }
        }
    }

    /*
    * Updates- each sound's playing note with given midi note.
    */
    void midiLearn(const MidiMessage& msg)
    {
        note = msg.getNoteNumber();
        DBG(note);
        for (auto* soundSource : sounds)
        {
            auto* const sound = static_cast<DrumSound* const> (soundSource);
            sound->setMidiNote(note);
        }
    }

    /*
    * Links midi learn with the pointer passed as argument.
    * Used to attach processor parameter.
    */
    void attachMidiLearn(std::atomic<float>* ptr) { learnEnabled = ptr; }

private:
    /*
    * Adds new sounds to this synth.
    * Each sound will start a thread and read a file from disk
    */
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

            for (auto i = 1; i <= maxSoundsPerRange; i++)
            {
                addSound(sound = new DrumSound());
                sound->setName(chName);
                sound->setVelocityRange(velocityRange);
                sound->setIndex(i);
                sound->setMidiNote(note);
            }
            start += lenght;
            if (start > velocityTot)
                break;
        }
    }

    /*
    * Prints active voices preceeded by a "note on" flag.
    * For debugging purposes only.
    */
    void voicesToLog(int midiNote, float velocity)
    {
        if (velocity > 0.0f)
            DBG("                     <<<<<< note on");

        int voiceIndex;
        int numVoices = getNumVoices();
        String msg;

        // Print all active voices
        Array<int> activeVoices;
        for (int i = 0; i < numVoices; i++)
        {
            voice = voices[i];
            if (voice->isVoiceActive())
            {
                activeVoices.add(i);
            }
        }

        msg << "Active voices: ";

        for (auto index : activeVoices)
            msg << index << ", ";

        msg << "\n";

        DBG(msg);

        // Print the last active voice
        //for (int i = numVoices - 1; i >= 0; i--)
        //{
        //    voice = voices[i];
        //    if (voice->isVoiceActive()) {
        //        voiceIndex = i;
        //        msg << "voice: " << voiceIndex << ", startNote: " << midiNote << "\n";
        //        DBG(msg);
        //        break;
        //    }
        //}
    }

    AudioProcessorValueTreeState& parameters;
    DrumSound* sound;
    SynthesiserVoice* voice;
    std::unique_ptr<File> file;
    std::atomic<float>* learnEnabled;
    bool shouldStealNotes = true;
    bool mutingEnabled = false;
    String chName;
    int note;

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