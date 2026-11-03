#include "DspEngine.h"
#include <algorithm>
#include <cmath>

namespace dpx {
namespace { constexpr float pi=3.14159265358979323846f; constexpr float tiny=1.0e-12f;
float clampf(float x,float lo,float hi) noexcept { return std::max(lo,std::min(hi,x)); }
void highpass(float sr,float hz,float& b0,float& b1,float& b2,float& a1,float& a2) noexcept {
    const float w=2*pi*clampf(hz,10.0f,0.45f*sr)/sr, c=std::cos(w), s=std::sin(w), alpha=s/(2*0.70710678f), a0=1+alpha;
    b0=(1+c)*0.5f/a0; b1=-(1+c)/a0; b2=b0; a1=-2*c/a0; a2=(1-alpha)/a0;
}
void peakEq(float sr,float hz,float db,float& b0,float& b1,float& b2,float& a1,float& a2) noexcept {
    const float A=std::pow(10.0f,db/40.0f), w=2*pi*clampf(hz,20.0f,0.45f*sr)/sr, c=std::cos(w), s=std::sin(w), alpha=s/(2*0.8f), a0=1+alpha/A;
    b0=(1+alpha*A)/a0; b1=(-2*c)/a0; b2=(1-alpha*A)/a0; a1=(-2*c)/a0; a2=(1-alpha/A)/a0;
}
}
float DspEngine::Biquad::tick(float x) noexcept { const float y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y; return y; }
float DspEngine::dbToGain(float db) noexcept { return std::pow(10.0f,db/20.0f); }
float DspEngine::gainToDb(float g) noexcept { return 20.0f*std::log10(std::max(g,1.0e-6f)); }
void DspEngine::prepare(double sampleRate,std::size_t maxBlockSize) {
    sr_=std::max(8000.0,sampleRate); const auto n=static_cast<std::size_t>(sr_*0.75);
    delayBufferL_.assign(n,0.0f); delayBufferR_.assign(n,0.0f); delayWrite_=0; delaySamples_=static_cast<std::size_t>(sr_*0.28); updateFilters(); reset(); (void)maxBlockSize;
}
void DspEngine::reset() noexcept { lowL_.reset();lowR_.reset();presenceL_.reset();presenceR_.reset();airL_.reset();airR_.reset();compEnvL_=compEnvR_=deEssEnvL_=deEssEnvR_=0;delayStateL_=delayStateR_=reverbStateL_=reverbStateR_=0;std::fill(delayBufferL_.begin(),delayBufferL_.end(),0);std::fill(delayBufferR_.begin(),delayBufferR_.end(),0);delayWrite_=0;sumSq_=sumLeft_=sumRight_=sumCross_=0;peak_=low_=mid_=high_=sib_=0;metrics_=Metrics{}; }
void DspEngine::updateFilters() noexcept {
    if (cachedLowCut_ == settings_.lowCutHz && cachedPresence_ == settings_.presenceDb && cachedAir_ == settings_.airDb) return;
    cachedLowCut_ = settings_.lowCutHz; cachedPresence_ = settings_.presenceDb; cachedAir_ = settings_.airDb;
    float b0,b1,b2,a1,a2; highpass(static_cast<float>(sr_),settings_.lowCutHz,b0,b1,b2,a1,a2);
    lowL_={b0,b1,b2,a1,a2}; lowR_=lowL_; peakEq(static_cast<float>(sr_),3200,settings_.presenceDb,b0,b1,b2,a1,a2);presenceL_={b0,b1,b2,a1,a2};presenceR_=presenceL_;peakEq(static_cast<float>(sr_),10500,settings_.airDb,b0,b1,b2,a1,a2);airL_={b0,b1,b2,a1,a2};airR_=airL_;
    delaySamples_=std::clamp<std::size_t>(static_cast<std::size_t>(sr_*0.28),1,delayBufferL_.empty()?1:delayBufferL_.size()-1);
}
void DspEngine::analyse(const float* l,const float* r,std::size_t n) noexcept {
    if(!l||!r||n==0)return;
    const float aLow=std::exp(-2*pi*180.0f/static_cast<float>(sr_)),aHigh=std::exp(-2*pi*4500.0f/static_cast<float>(sr_)); float lpL=0,lpR=0,hpL=0,hpR=0;
    for(std::size_t i=0;i<n;++i){float x=l[i],y=r[i],m=0.5f*(x+y);sumSq_+=0.5*(double(x)*x+double(y)*y);sumLeft_+=double(x)*x;sumRight_+=double(y)*y;sumCross_+=double(x)*y;peak_=std::max(peak_,std::max(std::abs(x),std::abs(y)));lpL=aLow*lpL+(1-aLow)*x;lpR=aLow*lpR+(1-aLow)*y;float lo=0.5f*(lpL+lpR);hpL=aHigh*hpL+(1-aHigh)*x;hpR=aHigh*hpR+(1-aHigh)*y;float hi=0.5f*((x-hpL)+(y-hpR));low_+=lo*lo;high_+=hi*hi;mid_+=(m-lo-hi)*(m-lo-hi);sib_+=hi*hi;}
    metrics_.samples+=n;const double count=double(metrics_.samples);const float rms=std::sqrt(static_cast<float>(sumSq_/count));metrics_.rmsDb=gainToDb(rms);metrics_.peakDb=gainToDb(peak_);metrics_.crestDb=metrics_.peakDb-metrics_.rmsDb;const float total=static_cast<float>(low_+mid_+high_+tiny);metrics_.lowEnergy=low_/total;metrics_.midEnergy=mid_/total;metrics_.highEnergy=high_/total;metrics_.sibilance=clampf(metrics_.highEnergy*2.0f,0,1);metrics_.stereoCorrelation=static_cast<float>(sumCross_/(std::sqrt(sumLeft_*sumRight_)+tiny));metrics_.estimatedLufs=metrics_.rmsDb-0.7f;
}
AnalysisResult DspEngine::makeSuggestion(Mode mode,Style style) const noexcept { return makeSuggestion(mode, style, metrics_); }
AnalysisResult DspEngine::makeSuggestion(Mode mode,Style style,const Metrics& snapshot) const noexcept {
    AnalysisResult out;out.metrics=snapshot;out.suggested=Settings{};out.suggested.mode=mode;out.suggested.style=style;
    const auto& metrics_ = snapshot;
    if(metrics_.samples==0)return out;
    auto& s=out.suggested;
    if(mode==Mode::vocal){s.lowCutHz=metrics_.lowEnergy>0.35f?95.0f:75.0f;s.presenceDb=clampf(2.2f-(metrics_.midEnergy>0.5f?1.0f:0.0f),0.0f,3.0f);s.airDb=metrics_.highEnergy<0.18f?1.8f:0.3f;s.thresholdDb=clampf(metrics_.rmsDb-9.0f,-30.0f,-10.0f);s.ratio=metrics_.crestDb>14?3.5f:2.4f;s.deEssAmount=clampf(metrics_.sibilance*1.5f,0.1f,0.8f);s.saturation=0.06f;s.reverb=0.07f;s.stereoWidth=1.0f;}
    else {s.lowCutHz=25.0f;s.presenceDb=metrics_.midEnergy>0.55f?-0.8f:0.6f;s.airDb=metrics_.highEnergy<0.15f?0.8f:0.0f;s.thresholdDb=clampf(metrics_.rmsDb-5.0f,-18.0f,-5.0f);s.ratio=style==Style::dynamic?1.5f:2.0f;s.attackMs=style==Style::dynamic?35.0f:15.0f;s.releaseMs=120.0f;s.saturation=style==Style::warm?0.12f:0.035f;s.stereoWidth=clampf(1.0f+(0.9f-metrics_.stereoCorrelation)*0.12f,0.9f,1.12f);s.limiterCeilingDb=-1.0f;}
    if(style==Style::rnb){s.reverb=0.12f;s.delay=0.08f;s.presenceDb*=0.85f;}if(style==Style::warm){s.airDb*=0.5f;s.saturation=std::max(s.saturation,0.1f);}if(style==Style::streaming){s.limiterCeilingDb=-1.0f;s.ratio=1.8f;}if(style==Style::commercial){s.ratio=2.3f;s.thresholdDb-=1.0f;}out.valid=true;return out;
}
void DspEngine::process(float* l,float* r,std::size_t n) noexcept {
    if(!l||!r||settings_.bypass)return;
    updateFilters(); const float in=dbToGain(settings_.inputGainDb),out=dbToGain(settings_.outputGainDb),wet=clampf(settings_.dryWet,0,1),ceiling=dbToGain(settings_.limiterCeilingDb);
    const float ca=std::exp(-1.0f/(static_cast<float>(sr_)*settings_.attackMs*0.001f)),cr=std::exp(-1.0f/(static_cast<float>(sr_)*settings_.releaseMs*0.001f));const float da=std::exp(-1.0f/(static_cast<float>(sr_)*0.0015f));
    for(std::size_t i=0;i<n;++i){float dl=l[i],dr=r[i],x=dl*in,y=dr*in;if(settings_.mode==Mode::vocal){x=lowL_.tick(x);y=lowR_.tick(y);x=presenceL_.tick(x);y=presenceR_.tick(y);x=airL_.tick(x);y=airR_.tick(y);}
        float hx=x-deEssEnvL_,hy=y-deEssEnvR_;deEssEnvL_=da*deEssEnvL_+(1-da)*x;deEssEnvR_=da*deEssEnvR_+(1-da)*y;float ess=0.5f*(std::abs(hx)+std::abs(hy));float reduction=1.0f-clampf(settings_.deEssAmount*ess*2.0f,0,0.45f);x*=reduction;y*=reduction;
        float envL=std::abs(x),envR=std::abs(y);compEnvL_=(envL>compEnvL_?ca:cr)*compEnvL_+(1-(envL>compEnvL_?ca:cr))*envL;compEnvR_=(envR>compEnvR_?ca:cr)*compEnvR_+(1-(envR>compEnvR_?ca:cr))*envR;float env=std::max(compEnvL_,compEnvR_);float envDb=gainToDb(env);float over=std::max(0.0f,envDb-settings_.thresholdDb);float compDb=-over*(1.0f-1.0f/std::max(1.0f,settings_.ratio));float cg=dbToGain(compDb);x*=cg;y*=cg;
        const float sat=clampf(settings_.saturation,0,0.8f);x=(1-sat)*x+sat*std::tanh(2*x);y=(1-sat)*y+sat*std::tanh(2*y);
        float mid=0.5f*(x+y),side=0.5f*(x-y);side*=clampf(settings_.stereoWidth,0,2);x=mid+side;y=mid-side;
        float delayedL=delayBufferL_.empty()?0:delayBufferL_[(delayWrite_+delayBufferL_.size()-delaySamples_)%delayBufferL_.size()];float delayedR=delayBufferR_.empty()?0:delayBufferR_[(delayWrite_+delayBufferR_.size()-delaySamples_)%delayBufferR_.size()];if(!delayBufferL_.empty()){delayBufferL_[delayWrite_]=x+delayedR*0.18f;delayBufferR_[delayWrite_]=y+delayedL*0.18f;delayWrite_=(delayWrite_+1)%delayBufferL_.size();}
        reverbStateL_=0.82f*reverbStateL_+0.18f*x;reverbStateR_=0.82f*reverbStateR_+0.18f*y;x+=settings_.delay*delayedL+settings_.reverb*(reverbStateR_-x*0.25f);y+=settings_.delay*delayedR+settings_.reverb*(reverbStateL_-y*0.25f);
        if(settings_.mode==Mode::mastering){x=std::tanh(x*1.03f);y=std::tanh(y*1.03f);}x=clampf(x* out,-ceiling,ceiling);y=clampf(y*out,-ceiling,ceiling);l[i]=dl*(1-wet)+x*wet;r[i]=dr*(1-wet)+y*wet;
    }
}
}
