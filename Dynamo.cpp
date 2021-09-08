#include "Dynamo.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

Dynamo::Dynamo(const InstanceInfo& info)
  : Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kWetPC)->InitDouble("Wet/Dry", 49.0, 0.0, 100.0, 0.01, "%");

  GetParam(kfeedback)->InitDouble("Feedback", 0.0, 0.0, 50.0, 0.01, "%");


  GetParam(kLFORate)->InitDouble("LFO Rate", 1400.0, 10.0, 5000.0, 1.0, ":-)");
  GetParam(kLFODepth)->InitDouble("LFO Depth", 17.3, 5.0, 30.0, 0.01, ":-)");


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {

    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachBackground(MYBKGROUND_FN);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();

    const IBitmap knobBitmap = pGraphics->LoadBitmap(MYPNG_FN, 60);


    pGraphics->AttachControl(new IBKnobControl(b.GetCentredInside(100).GetVShifted(-10).GetHShifted(-130), knobBitmap, kLFORate));
    pGraphics->AttachControl(new IBKnobControl(b.GetCentredInside(100).GetVShifted(-10).GetHShifted(130), knobBitmap, kfeedback));
    pGraphics->AttachControl(new IBKnobControl(b.GetCentredInside(100).GetVShifted(212).GetHShifted(-130), knobBitmap, kLFODepth));
    pGraphics->AttachControl(new IBKnobControl(b.GetCentredInside(100).GetVShifted(212).GetHShifted(130), knobBitmap, kWetPC));
  };
#endif
}

Dynamo::~Dynamo() {
}

void Dynamo::OnReset() {
  mPhase = 0.0;
  mPhaseIncrement = PI / (GetSampleRate());

  mLFO = 0.0f;


  mBufferSize = 2 * GetSampleRate();
  mBuffers.clear();
  for (int i = 0; i < numberOfBuffers; i++) {
    mBuffers.push_back(new double[mBufferSize]);
    memset(mBuffers[i], 0, mBufferSize * sizeof(double));
    mWriteIndex[i] = 0;
    mReadIndex[i] = 0;
  }
}

#if IPLUG_DSP
void Dynamo::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double wet = GetParam(kWetPC)->Value() / 100.0;


  const double lfoRate = GetParam(kLFORate)->Value();
  const double lfoDepth = GetParam(kLFODepth)->Value();



  const double feedback = GetParam(kfeedback)->Value() / 100.0;
  int nChans = NOutChansConnected();

  double delaySam = 0.0;

  if (NOutChansConnected() > numberOfBuffers)
    nChans = numberOfBuffers;

  mPhaseIncrement = (2 * PI) / (lfoRate * (GetSampleRate() / 1000));

  for (int s = 0; s < nFrames; s++) {
    mPhase += mPhaseIncrement;

    if (mPhase >= 2 * PI) {
      mPhase = 0.0;
    }


    for (int i = 0; i < numberOfBuffers; i++) {
      mReadIndex[i] = mWriteIndex[i] - ((int)delaySam);
      if (mReadIndex[i] < 0) {
        mReadIndex[i] += mBufferSize;
      }
    }



    for (int c = 0; c < nChans; c++) {
      double yn = mBuffers[c][mReadIndex[c]];

      mBuffers[c][mWriteIndex[c]] = inputs[c][s] + feedback * yn;

      outputs[c][s] = ((wet * yn + (1 - wet) * inputs[c][s]));

      ++mWriteIndex[c];
      if (mWriteIndex[c] >= mBufferSize) {
        mWriteIndex[c] = 0;
      }

      ++mReadIndex[c];
      if (mReadIndex[c] >= mBufferSize) {
        mReadIndex[c] = 0;
      }
    }
  }
}
#endif
