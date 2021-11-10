#pragma once

#include <BinaryData.h>

using namespace reactjuce;

class CustomEditor : public AudioProcessorEditor, public AudioProcessorParameter::Listener, public Timer {
   public:
    CustomEditor(AudioProcessor& proc)
        : AudioProcessorEditor(proc), engine(std::make_shared<EcmascriptEngine>()), appRoot(engine), harness(appRoot) {
        auto& params = processor.getParameters();
        paramReadouts.resize(static_cast<size_t>(params.size()));

        for (auto& p : params) {
            if (auto paramWithID = dynamic_cast<AudioProcessorParameterWithID*>(p)) {
                parameters.emplace(paramWithID->paramID, p);
            }

            const auto index = static_cast<size_t>(p->getParameterIndex());
            const auto value = p->getValue();

            paramReadouts[index].value = value;
            paramReadouts[index].dirty = true;

            p->addListener(this);
        }

#if JUCE_DEBUG
        File exeDir = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
        File bundle = exeDir.getChildFile("js/bundle.js");

        jassert(bundle.existsAsFile());
        bundleFile = bundle;

        harness.onBeforeAll = [this]() { beforeBundleEvaluated(); };
        harness.onAfterAll = [this]() { afterBundleEvaluated(); };

        harness.watch(bundleFile);
        harness.start();
#else
        engine->evaluateInline(String::fromUTF8(BinaryData::bundle_js));
#endif

        addAndMakeVisible(appRoot);

        setSize(400, 200);

        startTimerHz(30);
    }
    ~CustomEditor() {
        for (auto& p : processor.getParameters()) {
            p->removeListener(this);
        }
    }

    void parameterValueChanged(int parameterIndex, float newValue) {
        paramReadouts[static_cast<size_t>(parameterIndex)].value = newValue;
        paramReadouts[static_cast<size_t>(parameterIndex)].dirty = true;
    }
    void parameterGestureChanged(int, bool) {}

    void timerCallback() {
        for (size_t i = 0; i < paramReadouts.size(); ++i) {
            auto& pr = paramReadouts[i];

            if (pr.dirty) {
                const float value = pr.value.load();
                pr.dirty = false;

                const auto& p = processor.getParameters()[(int)i];
                String id = p->getName(100);

                if (auto* x = dynamic_cast<AudioProcessorParameterWithID*>(p)) id = x->paramID;

                float defaultValue = p->getDefaultValue();
                String stringValue = p->getText(value, 0);

                appRoot.dispatchEvent("parameterValueChange", static_cast<int>(i), id, defaultValue, value,
                                      stringValue);
            }
        }
    }

    void resized() { appRoot.setBounds(getLocalBounds()); }
    void paint(Graphics& g) { g.fillAll(Colours::transparentWhite); }

    ReactApplicationRoot& getReactAppRoot() { return appRoot; }

   private:
    void beforeBundleEvaluated() {
        engine->registerNativeMethod("beginParameterChangeGesture", [this](const var::NativeFunctionArgs& args) {
            if (auto it = parameters.find(args.arguments[0].toString()); it != parameters.cend())
                it->second->beginChangeGesture();

            return var::undefined();
        });

        engine->registerNativeMethod("setParameterValueNotifyingHost", [this](const var::NativeFunctionArgs& args) {
            if (auto it = parameters.find(args.arguments[0].toString()); it != parameters.cend())
                it->second->setValueNotifyingHost(args.arguments[1]);

            return var::undefined();
        });

        engine->registerNativeMethod("endParameterChangeGesture", [this](const var::NativeFunctionArgs& args) {
            if (auto it = parameters.find(args.arguments[0].toString()); it != parameters.cend())
                it->second->endChangeGesture();

            return var::undefined();
        });
    }
    void afterBundleEvaluated() {
        for (auto& p : processor.getParameters()) parameterValueChanged(p->getParameterIndex(), p->getValue());
    }

    std::shared_ptr<EcmascriptEngine> engine;
    ReactApplicationRoot appRoot;
    AppHarness harness;

    File bundleFile;

    std::map<String, AudioProcessorParameter*> parameters;

    struct ParameterReadout {
        std::atomic<float> value = 0.0;
        std::atomic<bool> dirty = false;

        ParameterReadout() = default;

        ParameterReadout(const ParameterReadout& other) {
            value = other.value.load();
            dirty = other.dirty.load();
        }
    };

    std::vector<ParameterReadout> paramReadouts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomEditor)
};
