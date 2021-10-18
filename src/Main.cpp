#include <JuceHeader.h>

#include "MidiScaleRemapper.h"
#include "MidiLoggerPluginDemo.h"

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new MidiScaleRemapper(); }
