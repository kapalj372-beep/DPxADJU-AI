#include "PluginProcessor.h"
#include "PluginEditor.h"

DPxADJUAudioProcessor::DPxADJUAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  parameters(*this, nullptr, "DPXADJU_STATE", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout DPxADJUAudioProcessor::createParameterLayout() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto choice = [&p](const char* id, const char* name, StringArray choices, int def) { p.push_back(std::make_unique<AudioParameterChoice>(ParameterID{id,1}, name, choices, def)); };
    auto f = [&p](const char* id,const char* name,float lo,float hi,float def,float step=0.01f){p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,NormalisableRange<float>(lo,hi,step),def));};
    choice("mode","Mode",{"Vocal","Mastering"},0); choice("style","Style",{"Pop","R&B","Streaming","Warm","Dynamic","Commercial"},0);
    f("input","Input",-24,24,0);f("output","Output",-24,12,0);f("mix","Dry Wet",0,1,1);f("lowcut","Low Cut",20,300,75,1);f("presence","Presence",-6,6,1.5f);f("air","Air",-6,6,0.5f);f("threshold","Threshold",-48,0,-18);f("ratio","Ratio",1,12,2.5f,0.1f);f("deess","De-esser",0,1,0.25f);f("sat","Saturation",0,0.8f,0.08f);f("reverb","Reverb",0,0.8f,0.08f);f("delay","Delay",0,0.8f,0);f("width","Stereo Width",0,2,1);f("ceiling","Limiter Ceiling",-6,0,-1);
    p.push_back(std::make_unique<AudioParameterBool>(ParameterID{"bypass",1},"Bypass",false));
    return {p.begin(),p.end()};
}
void DPxADJUAudioProcessor::prepareToPlay(double sr,int block){engine.prepare(sr,static_cast<std::size_t>(block));}
void DPxADJUAudioProcessor::releaseResources(){engine.reset();}
bool DPxADJUAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {const auto in=l.getMainInputChannelSet();const auto out=l.getMainOutputChannelSet();return in==out&&(in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());}
void DPxADJUAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;const int n=b.getNumSamples();if(b.getNumChannels()==0)return;
    auto get=[this](const char* id){return parameters.getRawParameterValue(id)->load();};
    dpx::Settings s; s.mode=get("mode")<0.5f?dpx::Mode::vocal:dpx::Mode::mastering;s.style=static_cast<dpx::Style>(juce::jlimit(0,5,static_cast<int>(get("style"))));s.inputGainDb=get("input");s.outputGainDb=get("output");s.dryWet=get("mix");s.lowCutHz=get("lowcut");s.presenceDb=get("presence");s.airDb=get("air");s.thresholdDb=get("threshold");s.ratio=get("ratio");s.deEssAmount=get("deess");s.saturation=get("sat");s.reverb=get("reverb");s.delay=get("delay");s.stereoWidth=get("width");s.limiterCeilingDb=get("ceiling");s.bypass=get("bypass")>0.5f;engine.setSettings(s);
    float* left=b.getWritePointer(0);if(b.getNumChannels()==1){engine.analyse(left,left,static_cast<std::size_t>(n));engine.process(left,left,static_cast<std::size_t>(n));}
    else {float* right=b.getWritePointer(1);engine.analyse(left,right,static_cast<std::size_t>(n));engine.process(left,right,static_cast<std::size_t>(n));}
    const auto& m=engine.metrics();meterRms.store(m.rmsDb,std::memory_order_relaxed);meterPeak.store(m.peakDb,std::memory_order_relaxed);meterCrest.store(m.crestDb,std::memory_order_relaxed);meterLufs.store(m.estimatedLufs,std::memory_order_relaxed);meterLow.store(m.lowEnergy,std::memory_order_relaxed);meterMid.store(m.midEnergy,std::memory_order_relaxed);meterHigh.store(m.highEnergy,std::memory_order_relaxed);meterCorr.store(m.stereoCorrelation,std::memory_order_relaxed);meterSibilance.store(m.sibilance,std::memory_order_relaxed);meterSamples.store(m.samples,std::memory_order_relaxed);
}
juce::AudioProcessorEditor* DPxADJUAudioProcessor::createEditor(){return new DPxADJUAudioProcessorEditor(*this);}
void DPxADJUAudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=parameters.copyState().createXml();copyXmlToBinary(*xml,dest);}
void DPxADJUAudioProcessor::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
dpx::Metrics DPxADJUAudioProcessor::getMetrics() const {dpx::Metrics m;m.rmsDb=meterRms.load(std::memory_order_relaxed);m.peakDb=meterPeak.load(std::memory_order_relaxed);m.crestDb=meterCrest.load(std::memory_order_relaxed);m.estimatedLufs=meterLufs.load(std::memory_order_relaxed);m.lowEnergy=meterLow.load(std::memory_order_relaxed);m.midEnergy=meterMid.load(std::memory_order_relaxed);m.highEnergy=meterHigh.load(std::memory_order_relaxed);m.stereoCorrelation=meterCorr.load(std::memory_order_relaxed);m.sibilance=meterSibilance.load(std::memory_order_relaxed);m.samples=meterSamples.load(std::memory_order_relaxed);return m;}
void DPxADJUAudioProcessor::runAutoAnalysis(){auto mode=parameters.getRawParameterValue("mode")->load()<0.5f?dpx::Mode::vocal:dpx::Mode::mastering;auto style=static_cast<dpx::Style>(juce::jlimit(0,5,static_cast<int>(parameters.getRawParameterValue("style")->load())));auto result=engine.makeSuggestion(mode,style,getMetrics());if(!result.valid)return;const auto& s=result.suggested;auto set=[this](const char* id,float v){if(auto* p=parameters.getParameter(id))p->setValueNotifyingHost(p->convertTo0to1(v));};set("lowcut",s.lowCutHz);set("presence",s.presenceDb);set("air",s.airDb);set("threshold",s.thresholdDb);set("ratio",s.ratio);set("deess",s.deEssAmount);set("sat",s.saturation);set("reverb",s.reverb);set("delay",s.delay);set("width",s.stereoWidth);set("ceiling",s.limiterCeilingDb);}
juce::String DPxADJUAudioProcessor::scanWaves(){return scanner.scanInstalledVST3();}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new DPxADJUAudioProcessor();}
