#include "PluginEditor.h"

namespace {
class ParameterPage final : public juce::Component {
public:
    ParameterPage(juce::AudioProcessorValueTreeState& state, std::initializer_list<std::pair<const char*,const char*>> controls) {
        for (auto c : controls) {
            auto slider = std::make_unique<juce::Slider>();
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
            slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb56cff));
            slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xffe7d7ff));
            auto label = std::make_unique<juce::Label>(); label->setText(c.second, juce::dontSendNotification); label->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*slider); addAndMakeVisible(*label);
            attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, c.first, *slider));
            sliders.push_back(std::move(slider)); labels.push_back(std::move(label));
        }
    }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xff17131f)); }
    void resized() override {
        const int cols=4, cellW=getWidth()/cols, cellH=150;
        for (std::size_t i=0;i<sliders.size();++i) { int x=static_cast<int>(i%cols)*cellW, y=static_cast<int>(i/cols)*cellH; labels[i]->setBounds(x,y+8,cellW,24);sliders[i]->setBounds(x+8,y+30,cellW-16,cellH-34); }
    }
private:
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
};
class InfoPage final : public juce::Component {
public:
    explicit InfoPage(const juce::String& message) { addAndMakeVisible(label);label.setText(message,juce::dontSendNotification);label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,juce::Colour(0xffd8c8e8)); }
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff17131f));}
    void resized() override {label.setBounds(getLocalBounds().reduced(28));}
private:juce::Label label;
};
}
DPxADJUAudioProcessorEditor::DPxADJUAudioProcessorEditor(DPxADJUAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
    setSize(900,650);title.setText("DPxADJU AI  /  MASTERING & VOCAL MIXER",juce::dontSendNotification);title.setFont(juce::Font(22.0f,juce::Font::bold));title.setColour(juce::Label::textColourId,juce::Colour(0xffeee3ff));addAndMakeVisible(title);
    for(auto* b:{&vocalButton,&masterButton,&autoButton,&scanButton,&bypassButton}){addAndMakeVisible(b);b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff332344));b->setColour(juce::TextButton::textColourOffId,juce::Colour(0xfff0e5ff));}
    vocalButton.onClick=[this]{processor.parameters.getParameter("mode")->setValueNotifyingHost(0.0f);};masterButton.onClick=[this]{processor.parameters.getParameter("mode")->setValueNotifyingHost(1.0f);};
    autoButton.onClick=[this]{processor.runAutoAnalysis();status.setText("Analysis applied when sufficient audio has been received.",juce::dontSendNotification);};
    scanButton.onClick=[this]{status.setText("Scanning VST3 plug-ins...",juce::dontSendNotification);auto result=processor.scanWaves();status.setText(result,juce::dontSendNotification);};
    bypassButton.onClick=[this]{auto* v=processor.parameters.getRawParameterValue("bypass");auto* p=processor.parameters.getParameter("bypass");p->setValueNotifyingHost(v->load()>0.5f?0.0f:1.0f);};
    tabs.addTab("VOCAL",juce::Colour(0xff241a30),new ParameterPage(processor.parameters,{{"lowcut","LOW CUT"},{"presence","PRESENCE"},{"air","AIR"},{"threshold","THRESHOLD"},{"ratio","RATIO"},{"deess","DE-ESSER"},{"sat","SATURATION"},{"reverb","REVERB"},{"delay","DELAY"},{"width","STEREO WIDTH"},{"input","INPUT"},{"output","OUTPUT"},{"mix","DRY / WET"}}),true);
    tabs.addTab("MASTER",juce::Colour(0xff241a30),new ParameterPage(processor.parameters,{{"lowcut","LOW CUT"},{"presence","TONE"},{"air","AIR"},{"threshold","THRESHOLD"},{"ratio","RATIO"},{"sat","SATURATION"},{"width","STEREO WIDTH"},{"ceiling","TRUE-PEAK CEILING"},{"input","INPUT"},{"output","OUTPUT"},{"mix","DRY / WET"}}),true);
    tabs.addTab("REFERENCE",juce::Colour(0xff241a30),new InfoPage("REFERENCE MATCHING\n\nReference-file import and feature matching are not implemented in this build.\nArtist names are style descriptors only; no artist voice or secret preset is copied."),true);
    tabs.addTab("BALANCE",juce::Colour(0xff241a30),new InfoPage("VOCAL / MUSIC BALANCER\n\nThe current processor accepts one stereo or mono stream.\nTwo-stem masking analysis and source separation are planned, not active."),true);
    tabs.addTab("WAVES / CHAIN",juce::Colour(0xff241a30),new InfoPage("WAVES SCANNER\n\nScan installed VST3 plug-ins and save JUCE's plug-in description database.\nThis scanner does not instantiate or route Waves processors yet."),true);
    addAndMakeVisible(tabs);status.setColour(juce::Label::textColourId,juce::Colour(0xffc8afd9));status.setText("DSP engine ready · local processing · no cloud AI",juce::dontSendNotification);addAndMakeVisible(status);startTimerHz(8);
}
DPxADJUAudioProcessorEditor::~DPxADJUAudioProcessorEditor(){stopTimer();}
void DPxADJUAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xff0e0b13));g.setColour(juce::Colour(0xff8f49c9));g.fillRect(0,74,getWidth(),2);}
void DPxADJUAudioProcessorEditor::resized(){auto r=getLocalBounds().reduced(18);title.setBounds(r.removeFromTop(42));r.removeFromTop(8);auto top=r.removeFromTop(36);vocalButton.setBounds(top.removeFromLeft(130));top.removeFromLeft(8);masterButton.setBounds(top.removeFromLeft(130));top.removeFromLeft(8);autoButton.setBounds(top.removeFromLeft(160));top.removeFromLeft(8);scanButton.setBounds(top.removeFromLeft(150));top.removeFromLeft(8);bypassButton.setBounds(top.removeFromLeft(90));r.removeFromTop(10);status.setBounds(r.removeFromBottom(26));r.removeFromBottom(4);tabs.setBounds(r);}
void DPxADJUAudioProcessorEditor::timerCallback(){auto m=processor.getMetrics();status.setText("INTEGRATED EST. " + juce::String(m.estimatedLufs,1) + " LUFS   |   PEAK " + juce::String(m.peakDb,1) + " dBFS   |   CREST " + juce::String(m.crestDb,1) + " dB",juce::dontSendNotification);}
