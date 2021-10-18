#pragma once

#include <array>

class MidiScaleRemapper : public AudioProcessor {
   public:
    MidiScaleRemapper() : AudioProcessor(BusesProperties()) {
        addParameter(noteOnVel = new AudioParameterFloat("volume", "Midi volume", 0.0, 1.0, 0.5));
    }

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}

    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) override {
        buffer.clear();

        midiMessages.swapWith(transformMidi(midiMessages));
    }

    AudioProcessorEditor *createEditor() override { return new GenericAudioProcessorEditor(*this); }

    const String getName() const override { return "MidiScaleRemapper"; }

    bool hasEditor() const override { return true; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int, const String &) override {}

    double getTailLengthSeconds() const override { return 0; }

    void getStateInformation(MemoryBlock &destData) override {
        MemoryOutputStream(destData, true).writeFloat(*noteOnVel);
    }

    void setStateInformation(const void *data, int sizeInBytes) override {
        noteOnVel->setValueNotifyingHost(MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readFloat());
    }

   private:
    int getTransformedNote(int noteIndex) {
        auto noteTransformation = getNoteTransformation(noteIndex);
        return noteIndex + noteTransformation;
    }

    bool shouldTransform(int noteIndex) {
        auto size = scaleTransformation.size();
        auto noteTransformation = getNoteTransformation(noteIndex);
        return noteTransformation != size;
    }

    int getNoteTransformation(int noteIndex) {
        auto size = scaleTransformation.size();
        auto noteInOctaveIndex = noteIndex % size;
        return scaleTransformation[noteInOctaveIndex];
    }

    MidiBuffer transformMidi(MidiBuffer midiMessages) {
        MidiBuffer processedMidi;

        for (const auto metadata : midiMessages) {
            auto message = metadata.getMessage();
            const auto vel = message.getVelocity();
            const auto time = metadata.samplePosition;

            const auto isNoteOn = message.isNoteOn();
            const auto isNoteOff = message.isNoteOff();
            const auto isNote = isNoteOn || isNoteOff;

            if (!isNote) {
                processedMidi.addEvent(message, time);
            } else {
                const auto num = message.getNoteNumber();
                uint8 nextVel = (uint8)(vel * *noteOnVel);

                if (shouldTransform(num)) {
                    const auto transformed = getTransformedNote(num);
                    if (isNoteOn) {
                        message = MidiMessage::noteOn(message.getChannel(), transformed, nextVel);
                    }
                    if (isNoteOff) {
                        message = MidiMessage::noteOff(message.getChannel(), transformed, nextVel);
                    }
                    processedMidi.addEvent(message, time);
                }
            }
        }
        return processedMidi;
    }

    AudioParameterFloat *noteOnVel;

    std::array<int, 12> scaleTransformation{0, 12, -1, 12, 0, 1, 12, 0, 12, 0, 12, -1};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiScaleRemapper)
};
