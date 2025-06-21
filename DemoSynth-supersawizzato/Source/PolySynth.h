/*
  ==============================================================================

    MonoSynth.h
    Created: 5 Dec 2022 7:42:27am
    Author:  Giorgio

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define DEFAULT_STEAL 2;

class KeysHistory
{
public:
    KeysHistory() {}
    ~KeysHistory() {}

    void press(const MidiMessage m)
    {
        lastVelocity = m.getFloatVelocity();
        keysPressed.push_back(m.getNoteNumber());
    }

    void release(const MidiMessage m)
    {
        auto releasedKeys = std::find(keysPressed.begin(), keysPressed.end(), m.getNoteNumber());

        if (any())
        {
            if (releasedKeys != std::end(keysPressed))
                keysPressed.erase(releasedKeys);
            else
            {
                DBG("Released key not found in keypressed log, releasing all keys");
                keysPressed.clear();
            }
        }
    }

    bool newEventReady(const int threshold)
    {
        return (keysPressed.size() <= threshold) || (lastEventProvided != getCandidate());
    }

    int getNewEvent()
    {
        return lastEventProvided = getCandidate();
    }

    float getVelocity()
    {
        return lastVelocity;
    }

    void releaseAll()
    {
        if (any()) keysPressed.clear();
    }

    bool any()
    {
        return !keysPressed.empty();
    }

    void setStealing(int criterion)
    {
        stealCriterion = criterion;
    }

private:

    int getCandidate()
    {
        int nn = 0;

        switch (stealCriterion)
        {
        case 0: // "Lowest"
            nn = *std::min_element(keysPressed.begin(), keysPressed.end());
            break;
        case 1: // "Highest"
            nn = *std::max_element(keysPressed.begin(), keysPressed.end());
            break;
        case 2: // "Oldest"
            nn = keysPressed.front();
            break;
        case 3: // "Newest"
            nn = keysPressed.back();
            break;
        default:
            break;
        }

        return nn;
    }

    std::vector<int> keysPressed;
    int lastEventProvided = -1000;
    float lastVelocity = 1.0f;
    int stealCriterion = DEFAULT_STEAL;
};

class PolySynthesiser : public Synthesiser
{
public:
    PolySynthesiser() = default;
    ~PolySynthesiser() = default;

    void setStealing(int criterion)
    {
        stealCriterion = criterion;
    }

    void noteOn(int midiChannel, int midiNoteNumber, float velocity) override
    {
        // First, find a free voice
        SynthesiserVoice* voice = findFreeVoice(getSound(0).get(), false, midiNoteNumber, midiChannel);

        if (voice == nullptr)
        {
            // All voices are used — choose one to steal
            voice = selectVoiceToSteal();
        }

        if (voice != nullptr)
        {
            stopVoice(voice, 1.0f, true); // Hard cut previous note
            startVoice(voice, getSound(0).get(), midiChannel, midiNoteNumber, velocity);
        }
    }

    void panic()
    {
        allNotesOff(0, false);
    }

private:
    int stealCriterion = 2; // Default to "Oldest"

    SynthesiserVoice* selectVoiceToSteal()
    {
        int targetNote = -1;
        SynthesiserVoice* selected = nullptr;

        std::vector<std::pair<int, SynthesiserVoice*>> candidates;

        for (auto* v : voices)
        {
            if (v->isVoiceActive())
            {
                candidates.emplace_back(v->getCurrentlyPlayingNote(), v);
            }
        }

        if (candidates.empty()) return nullptr;

        switch (stealCriterion)
        {
        case 0: // Lowest
            selected = std::min_element(candidates.begin(), candidates.end(),
                [](auto& a, auto& b) { return a.first < b.first; })->second;
            break;

        case 1: // Highest
            selected = std::max_element(candidates.begin(), candidates.end(),
                [](auto& a, auto& b) { return a.first < b.first; })->second;
            break;

        case 2: // Oldest
            for (auto* v : voices)
            {
                if (v->isVoiceActive())
                    return v; // return the first active voice (oldest)
            }
            break;

        case 3: // Newest
                for (int i = voices.size(); --i >= 0;)
                {
                    auto* v = voices[i];
                    if (v->isVoiceActive())
                        return v; // Newest (last active voice)
                }

            break;
        }

        return selected;
    }
};

class MonoSynthesiser : public PolySynthesiser
{
public:
    MonoSynthesiser()  {}
    ~MonoSynthesiser() {}

    void handleMidiEvent(const MidiMessage& m) override
    {
        const int channel = m.getChannel();

        if (m.isNoteOn())
        {
            pressedKeys.press(m);
            //noteOn(channel, m.getNoteNumber(), m.getFloatVelocity());
            if (pressedKeys.newEventReady(1))
                noteOn(channel, pressedKeys.getNewEvent(), pressedKeys.getVelocity());
        }
        else if (m.isNoteOff())
        {
            pressedKeys.release(m);
            noteOff(channel, m.getNoteNumber(), m.getFloatVelocity(), true);

            if (pressedKeys.any() && pressedKeys.newEventReady(0))
                noteOn(channel, pressedKeys.getNewEvent(), pressedKeys.getVelocity());
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
        {
            allNotesOff(channel, true);
            pressedKeys.releaseAll();
        }
        else if (m.isPitchWheel())
        {
            const int wheelPos = m.getPitchWheelValue();
            lastPitchWheelValues[channel - 1] = wheelPos;
            handlePitchWheel(channel, wheelPos);
        }
        else if (m.isAftertouch())
        {
            handleAftertouch(channel, m.getNoteNumber(), m.getAfterTouchValue());
        }
        else if (m.isChannelPressure())
        {
            handleChannelPressure(channel, m.getChannelPressureValue());
        }
        else if (m.isController())
        {
            handleController(channel, m.getControllerNumber(), m.getControllerValue());
        }
        else if (m.isProgramChange())
        {
            handleProgramChange(channel, m.getProgramChangeNumber());
        }
    }

    void panic()
    {
        allNotesOff(0, false);
        pressedKeys.releaseAll();
    }

    void setStealing(int criterion)
    {
        pressedKeys.setStealing(criterion);
    }

private:

    KeysHistory pressedKeys;

};


