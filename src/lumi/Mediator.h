#pragma once

#include "CustomProgram.h"

using namespace roli;

class Mediator : private TopologySource::Listener, Block::ProgramEventListener, Block::ProgramLoadedListener {
   public:
    Mediator(reactjuce::ReactApplicationRoot& root) {
        appRoot = &root;
        pts.addListener(this);
    };
    void onQuit() {
        if (b != nullptr) {
            b->setProgram(nullptr);
        }
    };
    void sendCommand(int a) {
        if (b != nullptr) {
            Block::ProgramEventMessage e;

            e.values[0] = a;
            b->sendProgramEvent(e);
        }
    };

   private:
    void handleProgramEvent(Block& source, const Block::ProgramEventMessage& event){
        // TODO handle responses event.values[0]
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
                p = block->getProgram();
                block->setProgram(std::make_unique<CustomProgram>(*block));
                block->addProgramLoadedListener(this);
                block->addProgramEventListener(this);
                return;
            }
        }
    };
    PhysicalTopologySource pts;
    Block::Program* p;
    Block* b = nullptr;
    reactjuce::ReactApplicationRoot* appRoot;
};
