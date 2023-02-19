#pragma once

#include "ReactEditor.h"
#include "juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h"

class CustomEditor : public AudioProcessorEditor {
   public:
    CustomEditor(AudioProcessor& proc) : AudioProcessorEditor(proc), editor(proc) {
        pluginHolder = StandalonePluginHolder::getInstance();
        auto config = pluginHolder->deviceManager.getAudioDeviceSetup();
        config.outputDeviceName = "None";
        pluginHolder->deviceManager.setAudioDeviceSetup(config, true);

        addAndMakeVisible(pairButton);
        addAndMakeVisible(settingsButton);
        addAndMakeVisible(editor);

        setSize(width, height + 50);
        editor.setSize(width, height);
        if (!BluetoothMidiDevicePairingDialogue::isAvailable()) pairButton.setEnabled(false);

        pairButton.onClick = [] {
            RuntimePermissions::request(RuntimePermissions::bluetoothMidi, [](bool wasGranted) {
                if (wasGranted) BluetoothMidiDevicePairingDialogue::open();
            });
        };
        settingsButton.onClick = [this] {
            bool isModal = settings != nullptr && settings->isCurrentlyModal();
            if (isModal) {
                settings->exitModalState();
            } else {
                auto dm =
                    new AudioDeviceSelectorComponent(pluginHolder->deviceManager, 0, 2, 0, 2, true, true, true, true);
                settings = new SettingsOverlay(dm);
                addChildComponent(settings);
                settings->enterModalState(false, nullptr, true);
            }
        };
    }
    ~CustomEditor() {}

    void resized() {
        auto r = getLocalBounds();
        auto currentWidth = r.getWidth();
        auto currentHeight = r.getHeight();

        editor.setBounds(r.removeFromBottom(height));
        pairButton.setBounds(0, 0, currentWidth / 2, 50);
        settingsButton.setBounds(currentWidth / 2, 0, currentWidth / 2, 50);
    }
    void paint(Graphics& g) { g.fillAll(Colours::transparentWhite); }

   private:
    class SettingsOverlay : public Component {
       public:
        SettingsOverlay(Component* settingsComponent) {
            settings = settingsComponent;
            int width = getParentWidth();
            int height = getParentHeight();

            setBounds(0, 0, width, height);
            setSize(width, height);

            addChildComponent(settingsComponent);
            settingsComponent->setAlwaysOnTop(true);
            settingsComponent->setBounds(getOverlayBounds());
            settingsComponent->toFront(true);
            addAndMakeVisible(settingsComponent);
            setVisible(true);
            setOpaque(false);
        }
        void paint(Graphics& g) override {
            g.fillAll(Colours::black.withAlpha(0.6f));
            Rectangle<int> overlayBounds = getOverlayBounds();
            settings->setBounds(overlayBounds);
            g.setColour(Colour::fromRGB(26, 30, 35));
            g.fillRect(overlayBounds);
        }

        Rectangle<int> getOverlayBounds() const noexcept {
            if (bounds.isEmpty()) {
                const int pw = getParentWidth();
                const int ph = getParentHeight();
                return Rectangle<int>(pw, ph).withSizeKeepingCentre(jmin(400, pw - 14), jmin(300, ph - 40));
            }

            return bounds.withZeroOrigin();
        }
        void resized() override {}
        void inputAttemptWhenModal() override { exitModalState(0); }
        void mouseDown(const MouseEvent&) override { exitModalState(0); }

       private:
        Rectangle<int> bounds;
        Component* settings;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsOverlay)
    };

    ReactEditor editor;
    int width = 550;
    int height = 750;
    StandalonePluginHolder* pluginHolder;
    TextButton pairButton{"Bluetooth"};
    TextButton settingsButton{"Settings"};
    SettingsOverlay* settings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomEditor)
};
