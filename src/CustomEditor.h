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

            const auto index = static_cast<size_t>(p->getParameterIndex());
            const auto value = p->getValue();

            paramReadouts[index].value = value;
            paramReadouts[index].dirty = true;

            p->addListener(this);
        }

        File exeDir = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
#if JUCE_DEBUG
        
        File bundle = exeDir.getChildFile("js/bundle.js");

        jassert(bundle.existsAsFile());
        bundleFile = bundle;

        harness.onBeforeAll = [this]() { beforeBundleEvaluated(); };
        harness.onAfterAll = [this]() { afterBundleEvaluated(); };

        harness.watch(bundleFile);
        harness.start();
#else
        beforeBundleEvaluated();
        engine->evaluateInline(String::fromUTF8(BinaryData::bundle_js));
        afterBundleEvaluated();
#endif

        File localScales = exeDir.getChildFile("scales.txt");
        if (localScales.existsAsFile()) {
            auto fileText = localScales.loadFileAsString();
            appRoot.dispatchEvent("getLocalScales", fileText);
        }

        addAndMakeVisible(appRoot);

        setSize(550, 750);

        startTimerHz(30);
    }
    ~CustomEditor() {
        for (auto& p : processor.getParameters()) {
            p->removeListener(this);
        }
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
