#pragma once

#include "CustomEditor.h"

AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    const int scaleLength = 12;
    const int defaultTonics = 7;
    const float defaultIndex = 0.0f;
    const float defaultMode = 0.17f;
    const Array<int> defaultScaleIntervals = {2, 2, 1, 2, 2, 2, 1};

    AudioProcessorValueTreeState::ParameterLayout p;

    p.add(std::make_unique<AudioParameterBool>("transformEnabled", "Enable transform", true));
    p.add(std::make_unique<AudioParameterInt>("tonics", "Tonics Count", 1, scaleLength, defaultTonics));
    p.add(std::make_unique<AudioParameterFloat>("index", "Scale Index", 0.0f, 1.0f, defaultIndex));
    p.add(std::make_unique<AudioParameterFloat>("mode", "Mode Index", 0.0f, 1.0f, defaultMode));
    p.add(std::make_unique<AudioParameterInt>("baseOctave", "Base Octave", 0, 10, 4));
    p.add(std::make_unique<AudioParameterInt>("root", "Root Note", 0, 11, 0));

    for (size_t i = 0; i < scaleLength; i++) {
        auto istr = std::to_string(i);
        auto defaultInterval = defaultScaleIntervals[i];
        p.add(std::make_unique<AudioParameterInt>("interval" + istr, "Scale Interval " + istr, 1, scaleLength, defaultInterval));
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
            MidiBuffer nextMessages = transformMidi(midiMessages);
            midiMessages.swapWith(nextMessages);
        }
    }

    AudioProcessorEditor *createEditor() {
        auto *editor = new CustomEditor(*this);
        editor->setSize(P_WIDTH, P_HEIGHT);
        return editor;
    }

    const String getName() const override { return ProjectInfo::projectName; }

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
        if (isScaleChanged()) {
            createNotesMap();
        }

        return notesMap[noteIndex];
    }

    bool shouldMute(int noteIndex) { return !isWhiteNote(noteIndex); }

    bool isWhiteNote(int noteIndex) {
        int i = noteIndex % scaleLength;
        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) return false;
        return true;
    }

    bool isScaleChanged() {
        // TODO do not call remap if no audio param was changed
        return true;
    }

    void createNotesMap() {
        notesMap.clear();

        int tonics = dynamic_cast<AudioParameterInt *>(parameters.getParameter("tonics"))->get();
        int octave = dynamic_cast<AudioParameterInt *>(parameters.getParameter("baseOctave"))->get();
        int root = dynamic_cast<AudioParameterInt *>(parameters.getParameter("root"))->get();

        const int minNote = 0;
        const int maxNote = 127;
        const int baseNote = octave * scaleLength;

        int intervalsSum = 0;
        Array<int> intervals = {};
        for (int i = 0; i < tonics - 1; i++) {
            auto istr = std::to_string(i);
            int val = dynamic_cast<AudioParameterInt *>(parameters.getParameter("interval" + istr))->get();
            intervalsSum += val;
            if (intervalsSum > scaleLength) {
                intervalsSum -= val;
                break;
            }
            intervals.insert(i, val);
        }

        if (intervalsSum < scaleLength) {
            intervals.insert(intervals.size(), scaleLength - intervalsSum);
        }

        const int tones = intervals.size();

        int intervalIndex;

        notesMap.set(baseNote, baseNote + root);

        intervalIndex = tones - 1;
        int prevNote = baseNote;
        for (int i = baseNote - 1; i >= minNote; i--) {
            if (isWhiteNote(i)) {
                int shift = intervals[intervalIndex];
                int mappedValue = prevNote - shift;
                if (mappedValue < minNote) {
                    break;
                }
                notesMap.set(i, mappedValue + root);
                prevNote = mappedValue;
                intervalIndex = intervalIndex - 1 < 0 ? tones - 1 : intervalIndex - 1;
            }
        }

        intervalIndex = 0;
        int nextNote = baseNote;
        for (int i = baseNote + 1; i <= maxNote; i++) {
            if (isWhiteNote(i)) {
                int shift = intervals[intervalIndex];
                int mappedValue = nextNote + shift;
                if (mappedValue > maxNote) {
                    break;
                }
                notesMap.set(i, mappedValue + root);
                nextNote = mappedValue;
                intervalIndex = intervalIndex + 1 > tones - 1 ? 0 : intervalIndex + 1;
            }
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
    HashMap<int, int> notesMap;

    AudioProcessorValueTreeState parameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiScaleRemapper)
};
