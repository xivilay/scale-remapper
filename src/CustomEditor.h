#pragma once

#include "ReactEditor.h"

using namespace reactjuce;

class CustomEditor : public AudioProcessorEditor {
   public:
    CustomEditor(AudioProcessor& proc) : AudioProcessorEditor(proc), editor(proc) {
        addAndMakeVisible(pairButton);
        addAndMakeVisible(editor);
        setSize(width, height + 50);
        editor.setSize(width, height);
        if (!BluetoothMidiDevicePairingDialogue::isAvailable()) pairButton.setEnabled(false);

        
        pairButton.onClick = [] {
            RuntimePermissions::request(RuntimePermissions::bluetoothMidi, [](bool wasGranted) {
                if (wasGranted) BluetoothMidiDevicePairingDialogue::open();
            });
        };
    }
    ~CustomEditor() {}

    void resized() {
        auto r = getLocalBounds();

        editor.setBounds(r.removeFromBottom(height));
        pairButton.setBounds(r.removeFromBottom(height + 50));
    }
    void paint(Graphics& g) { g.fillAll(Colours::transparentWhite); }

   private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomEditor)
    ReactEditor editor;
    int width = 550;
    int height = 750;
    TextButton pairButton{"Bluetooth"};
};
