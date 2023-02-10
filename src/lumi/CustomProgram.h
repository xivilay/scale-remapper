#pragma once

class CustomProgram : public roli::Block::Program {
   public:
    CustomProgram(roli::Block& b) : Program(b) {}

    String getLittleFootProgram() override {
        return R"littlefoot(
        const int max = 64;
        const int rootId = 27;
        const int globalKeyColor = 34;
        const int rootKeyColor = 35;
        const int octaveId = 4;
        int keyColor;
        int rootColor;
        int root;
        int state;
        int fullColors[12];
    
        void initialise() {
            setUseDefaultKeyHandler(true, false);
            keyColor = getLocalConfig(globalKeyColor);
            rootColor = getLocalConfig(rootKeyColor);
            root = getLocalConfig(rootId);
            initColors();
        }
        void initColors() {
            fullColors[0] = 0xcf3550;
            fullColors[1] = 0xf55333;
            fullColors[2] = 0xfd7033;
            fullColors[3] = 0xffa53e;
            fullColors[4] = 0xffc255;
            fullColors[5] = 0xffff00;
            fullColors[6] = 0xd1d545;
            fullColors[7] = 0x6da951;
            fullColors[8] = 0x2191ce;
            fullColors[9] = 0x4d6db5;
            fullColors[10] = 0x564a9d;
            fullColors[11] = 0x8c55a2;
        }
        void renderKeys() {
            for (int i = 0; i < 12; i++) {
                int highlighted = (state >> i) & 1;
                int num = 12 - 1 - i;
                // int color = num == root ? rootColor : keyColor;
                int color = fullColors[num];
                int fill = highlighted == 1 ? color : makeARGB(255, 0, 0, 0);
                fillPixel(fill, num, 0);
                fillPixel(fill, num + 12, 0);
            }
        }
        void handleMessage(int a, int b, int c) {
            if (state == a) return;
            state = a;
            root = a >> 12;
            int octave = getLocalConfig(octaveId);
            renderKeys();

            sendMessageToHost(root, 0, 0);
        }
        )littlefoot";
    }

   private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomProgram)
};
