#include "WavesScanner.h"

juce::String WavesScanner::getDatabasePath() const {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("DPxADJU AI");
    dir.createDirectory();
    return dir.getChildFile("plugin-database.xml").getFullPathName();
}
juce::String WavesScanner::scanInstalledVST3() {
    juce::AudioPluginFormatManager formats;
    formats.addDefaultFormats();
    juce::FileSearchPath paths;
    paths.add(juce::File("C:/Program Files/Common Files/VST3"));
    paths.add(juce::File("C:/Program Files (x86)/Common Files/VST3"));
    paths.add(juce::File("C:/Program Files/Steinberg/VST3"));
    auto dbFile = juce::File(getDatabasePath());
    if (auto xml = juce::XmlDocument::parse(dbFile)) knownPlugins.recreateFromXml(*xml);
    int countBefore = knownPlugins.getNumTypes();
    for (int i = 0; i < formats.getNumFormats(); ++i) {
        auto* format = formats.getFormat(i);
        if (format == nullptr || !format->getName().containsIgnoreCase("VST3")) continue;
        juce::PluginDirectoryScanner scanner(knownPlugins, *format, paths, true, juce::File(), false);
        juce::String current;
        while (scanner.scanNextFile(true, current)) {}
    }
    if (auto xml = knownPlugins.createXml()) xml->writeTo(dbFile);
    juce::StringArray wavesNames;
    for (const auto& desc : knownPlugins.getTypes()) {
        if (desc.manufacturerName.containsIgnoreCase("Waves") || desc.name.containsIgnoreCase("Waves"))
            wavesNames.addIfNotAlreadyThere(desc.name);
    }
    juce::String summary = "VST3 scan complete. " + juce::String(knownPlugins.getNumTypes())
        + " plug-ins in database (" + juce::String(knownPlugins.getNumTypes() - countBefore)
        + " new). Waves matches: " + juce::String(wavesNames.size()) + ".";
    if (!wavesNames.isEmpty()) summary += "\n" + wavesNames.joinIntoString(", ");
    summary += "\nVST2 is not scanned by this build; scanning does not load or process plug-ins.";
    return summary;
}
