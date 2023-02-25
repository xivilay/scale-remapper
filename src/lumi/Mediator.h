#pragma once

#include "CustomProgram.h"

using namespace roli;

class Mediator : private TopologySource::Listener, Block::ProgramEventListener, Block::ProgramLoadedListener {
   public:
    Mediator(reactjuce::ReactApplicationRoot& root, AudioProcessor& proc) : processor(proc) {
        appRoot = &root;
        pts.addListener(this);
    };
    ~Mediator() {
        if (b != nullptr) {
            b->setProgram(nullptr);
            b->removeProgramLoadedListener(this);
            b->removeProgramEventListener(this);
            b = nullptr;
        }

        pts.removeListener(this);
    }
    void sendCommand(int a) {
        if (b != nullptr) {
            Block::ProgramEventMessage e;

            e.values[0] = a;
            b->sendProgramEvent(e);
        }
    };

   private:
    void handleProgramEvent(Block&, const Block::ProgramEventMessage& event) {
        int messageId = event.values[0];
        int messageValue = event.values[1];
        if (messageId == octaveId) {
            float value = static_cast<float>(messageValue + octaveShift) / octavesCount;
            processor.setParameterNotifyingHost(octaveId, value);
        }
        if (messageId == colorModeId) {
            appRoot->dispatchEvent("uiSettingsChange", colorModeId, messageValue);
        }
    };
    void handleProgramLoaded(Block& block) {
        appRoot->dispatchEvent("requestComputedKeysData");
        block.removeProgramLoadedListener(this);
    };
    void topologyChanged() override {
        auto currentTopology = pts.getCurrentTopology();
        for (auto& block : currentTopology.blocks) {
            if (block->getType() == Block::lumiKeysBlock) {
                b = block;
                block->setProgram(std::make_unique<CustomProgram>(*block));
                block->addProgramLoadedListener(this);
                block->addProgramEventListener(this);
                return;
            }
        }
    };
    PhysicalTopologySource pts;
    Block::Ptr b;
    reactjuce::ReactApplicationRoot* appRoot;
    AudioProcessor& processor;

    const int octaveShift = 4;
    const int octaveId = 4;
    const int octavesCount = 10;
    const int colorModeId = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mediator);
};
