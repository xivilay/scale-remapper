#include <JuceHeader.h>

#include "ScaleRemapper.h"

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new MidiScaleRemapper(); }
