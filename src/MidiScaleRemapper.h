#pragma once

class MidiScaleRemapper : public AudioProcessor {
   public:
    MidiScaleRemapper() : AudioProcessor(BusesProperties()) {
        for (size_t i = 0; i < scaleLength; i++) {
            auto istr = std::to_string(i);
            addParameter(
                new AudioParameterInt("transformKey" + istr, "Transform Key " + istr, -scaleLength, scaleLength, 0));
        }
        for (size_t i = 0; i < scaleLength; i++) {
            auto istr = std::to_string(i);
            addParameter(new AudioParameterBool("muteKey" + istr, "Mute Key " + istr, false));
        }

        addParameter(new AudioParameterBool("transformEnabled", "Enable transform", true));

        setTransformation();
    }

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}

    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) override {
        buffer.clear();
        Array params = getParameters();
        auto transformEnabled = dynamic_cast<AudioParameterBool *>(params[scaleLength * 2]);
        if (*transformEnabled) {
            midiMessages.swapWith(transformMidi(midiMessages));
        }
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
        // MemoryOutputStream stream(destData, true);
        // stream.writeFloat(*transformEnabled);
    }

    void setStateInformation(const void *data, int sizeInBytes) override {
        // MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
        // transformEnabled->setValueNotifyingHost(stream.readFloat());
    }

   private:
    int getTransformedNote(int noteIndex) {
        auto noteTransformation = getNoteTransformation(noteIndex);
        return noteIndex + noteTransformation;
    }

    bool shouldMute(int noteIndex) {
        auto noteInOctaveIndex = noteIndex % scaleLength;
        auto muteParamIndex = scaleLength + noteInOctaveIndex;
        Array params = getParameters();
        auto param = dynamic_cast<AudioParameterBool *>(params[muteParamIndex]);
        return param->get();
    }

    int getNoteTransformation(int noteIndex) {
        auto noteInOctaveIndex = noteIndex % scaleLength;
        Array params = getParameters();
        auto param = dynamic_cast<AudioParameterInt *>(params[noteInOctaveIndex]);
        return param->get();
    }

    void setTransformation() {
        Array params = getParameters();
        int scaleTransformation[12] = {0, 12, -1, 12, 0, 1, 12, 0, 12, 0, 12, -1};
        for (size_t i = 0; i < 12; i++) {
            int n = scaleTransformation[i];
            bool shouldMute;
            int transform;
            if (n == 12) {
                shouldMute = true;
                transform = 0;
            } else {
                shouldMute = false;
                transform = n;
            }
            auto muteParam = dynamic_cast<AudioParameterBool *>(params[scaleLength + i]);
            *muteParam = shouldMute;
            auto transformParam = dynamic_cast<AudioParameterInt *>(params[i]);
            *transformParam = transform;
        }
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

                if (!shouldMute(num)) {
                    const auto transformed = getTransformedNote(num);
                    if (isNoteOn) {
                        message = MidiMessage::noteOn(message.getChannel(), transformed, vel);
                    }
                    if (isNoteOff) {
                        message = MidiMessage::noteOff(message.getChannel(), transformed, vel);
                    }
                    processedMidi.addEvent(message, time);
                }
            }
        }
        return processedMidi;
    }

    const int scaleLength = 12;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiScaleRemapper)
};
