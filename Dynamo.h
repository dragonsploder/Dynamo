#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <vector>

const int kNumPresets = 1;
const int numberOfBuffers = 4;

enum EParams
{
  kWetPC = 0,
  kfeedback,
  kLFORate,
  kLFODepth,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class Dynamo final : public Plugin
{
public:
  Dynamo(const InstanceInfo& info);
  ~Dynamo();

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

  void OnReset() override;

private:
  double mPhase;
  double mPhaseIncrement;

  double mLFO;

  std::vector<double*> mBuffers;
  int mReadIndex[numberOfBuffers];
  int mWriteIndex[numberOfBuffers];
  int mBufferSize;
};
