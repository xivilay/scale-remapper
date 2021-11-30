#pragma once

#include "CustomEditor.h"

AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    AudioProcessorValueTreeState::ParameterLayout p;

    p.add(std::make_unique<AudioParameterBool>("transformEnabled", "Enable transform", true));

    p.add(std::make_unique<AudioParameterFloat>("index", "Scale Index", 0.0f, 1.0f, 0.0f));
    p.add(std::make_unique<AudioParameterFloat>("mode", "Mode Index", 0.0f, 1.0f, 0.17f));

    const int scaleLength = 12;

    for (size_t i = 0; i < scaleLength; i++) {
        auto istr = std::to_string(i);
        p.add(std::make_unique<AudioParameterInt>("transformKey" + istr, "Transform Key " + istr, -scaleLength,
                                                  scaleLength, 0));
    }
    for (size_t i = 0; i < scaleLength; i++) {
        auto istr = std::to_string(i);
        p.add(std::make_unique<AudioParameterBool>("muteKey" + istr, "Mute Key " + istr, false));
    }

    return p;
}

class MidiScaleRemapper : public AudioProcessor {
   public:
    MidiScaleRemapper()
        : AudioProcessor(BusesProperties()), parameters(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}

    void processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) override {
        buffer.clear();
        
        auto transformEnabled = dynamic_cast<AudioParameterBool *>(parameters.getParameter("transformEnabled"))->get();
        if (transformEnabled) {
            midiMessages.swapWith(transformMidi(midiMessages));
        }
    }

    AudioProcessorEditor *createEditor() {
        auto *editor = new CustomEditor(*this);

        editor->setResizable(true, true);
        editor->setResizeLimits(P_WIDTH, P_HEIGHT, P_WIDTH * 2, P_HEIGHT * 2);
        editor->getConstrainer()->setFixedAspectRatio(P_WIDTH / P_HEIGHT);
        editor->setSize(P_WIDTH, P_HEIGHT);

        return editor;
    }

    const String getName() const override { return JucePlugin_Name; }

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
        auto state = parameters.copyState();
        std::unique_ptr<XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }

    void setStateInformation(const void *data, int sizeInBytes) override {
        std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName(parameters.state.getType()))
                parameters.replaceState(ValueTree::fromXml(*xmlState));
    }

   private:
    int getTransformedNote(int noteIndex) {
        auto noteTransformation = getNoteTransformation(noteIndex);
        return noteIndex + noteTransformation;
    }

    bool shouldMute(int noteIndex) {
        auto noteInOctaveIndex = noteIndex % scaleLength;
        auto istr = std::to_string(noteInOctaveIndex);
        return dynamic_cast<AudioParameterBool *>(parameters.getParameter("muteKey" + istr))->get();
    }

    int getNoteTransformation(int noteIndex) {
        auto noteInOctaveIndex = noteIndex % scaleLength;
        auto istr = std::to_string(noteInOctaveIndex);
        return dynamic_cast<AudioParameterInt *>(parameters.getParameter("transformKey" + istr))->get();
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

    AudioProcessorValueTreeState parameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiScaleRemapper)
};
