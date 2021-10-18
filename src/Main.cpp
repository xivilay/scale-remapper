#include <JuceHeader.h>

#include "MidiScaleRemapper.h"

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new MidiScaleRemapper(); }
