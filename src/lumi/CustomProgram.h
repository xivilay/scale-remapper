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
        const int colorModeId = 64;
        int dimKeyColor;
        int keyColor;
        int rootColor;
        int root;
        int octave;
        int state;
        int keysState;
        int colorMode;
        int fullColors[12];
        int whiteKeys[14];
        int remapIsOn;
    
        void initialise() {
            setUseDefaultKeyHandler(true, false);
            keyColor = getLocalConfig(globalKeyColor);
            rootColor = getLocalConfig(rootKeyColor);
            root = getLocalConfig(rootId);
            octave = getLocalConfig(octaveId);
            sendMessageToHost(octaveId, octave, 0);
            initColors();
            initWhiteKeys();
        }
        void initWhiteKeys() {
            int whiteKeyIndex = 0;
            for (int i = 0; i < 24; i++) {
                if (isWhiteKey(i)) {
                    whiteKeys[whiteKeyIndex] = i;
                    whiteKeyIndex++;
                }
            }
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
            dimKeyColor = makeARGB(255, 0, 0, 0);
        }
        bool isWhiteKey(int noteIndex) {
            int i = noteIndex % 12;
            if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) return false;
            return true;
        }
        int getLastBits (int k, int n) {
            return k & ((1 << n) - 1);
        }
        int getMidBits (int k, int m, int n) {
            return getLastBits(k >> m, n);
        }
        void renderKeys() {
            if (remapIsOn == 0) {
                for (int i = 0; i < 12; i++) {
                    int highlighted = (keysState >> i) & 1;
                    int num = 12 - 1 - i;
                    int color;
                    if (colorMode == 1) {
                        color = fullColors[num];
                    } else {
                        color = num == root ? rootColor : keyColor;
                    }
                    int fill = highlighted == 1 ? color : dimKeyColor;
                    fillPixel(fill, num, 0);
                    fillPixel(fill, num + 12, 0);
                }
            } else {
                int selectedCount = 0;
                for (int i = 0; i < 12; i++) {
                    selectedCount += ((keysState >> i) & 1);
                }
                int scaleIndex = 0;
                for (int i = root; i < 12 + root; i++) {
                    int highlighted = (keysState >> (11 - i%12)) & 1;
                    int num = (12+i) % 12;
                    
                    if (highlighted == 1) {
                        int w = scaleIndex;
                        int color;
                        while (w <= 13) {
                            if (colorMode == 1) {
                                color = fullColors[num];
                            } else {
                                color = num == root ? rootColor : keyColor;
                            }
                            fillPixel(color, whiteKeys[w], 0);
                            w += selectedCount;
                        }
                        scaleIndex++;
                    }
                    if (!isWhiteKey(i-root)) {
                        fillPixel(dimKeyColor, i-root, 0);
                        fillPixel(dimKeyColor, i-root + 12, 0);
                    }
                }
            }
        }
        void handleMessage(int a, int b, int c) {
            if (state == a) return;
            state = a;
            keysState =  getMidBits(a, 0, 12);
            root =       getMidBits(a, 12, 4);
            colorMode =  getMidBits(a, 12+4, 1);
            remapIsOn =  getMidBits(a, 12+4+1, 1);
            renderKeys();
        }
        void handleButtonDown (int index) {
            if (index == 1 || index == 2) {
                int nextOctave;
                if (index == 1) {
                    nextOctave = --octave;
                }
                if (index == 2) {
                    nextOctave = ++octave;
                }
                if (nextOctave < -4 || nextOctave > 8) return;
                setLocalConfig(octaveId, nextOctave);
                sendMessageToHost(octaveId, nextOctave, 0);
            }
        }

        void handleButtonUp (int index) {
            if (index == 0) {
                colorMode = colorMode ^ 1;
                sendMessageToHost(colorModeId, colorMode, 0);
            }
        }
        )littlefoot";
    }

   private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomProgram)
};
