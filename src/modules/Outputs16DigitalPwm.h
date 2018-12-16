#ifndef _OUTPUTS16DIGITALPWM_H_
#define _OUTPUTS16DIGITALPWM_H_

#include "mbed.h"
#include "PinNames.h"
#include "..\bitLabCore\src\utils.h"
#include "IOutputs.h"

class Outputs16DigitalPwm: public IOutputs
{
public:
  Outputs16DigitalPwm();

  void setOutput(uint32_t idxOutput, uint32_t value);
  void onTick();
  bool getInput50HzIsStable() { return input50HzIsStable; }
  float getMeasured50HzFrequency()
  {
    return ((float)ticksPerSecond) / (lastZeroCrossDurationInTicks * 2.0f);
  }

private:
  static const uint32_t ticksPerSecond = 10 * 1000;
  static const uint32_t nominalZeroCrossPerSecond = 100;
  static const uint32_t gateOnTimeTicks = 10;
  static const uint32_t nominalTicksBetweenZeroCrosses = ticksPerSecond / nominalZeroCrossPerSecond;
  static const uint32_t nominalTicksBetweenZeroCrossesMaxDelta = nominalTicksBetweenZeroCrosses / 5;
  static const bool simulateVAC = true;

  static const uint32_t outputsCount = 16;
  DigitalOut outputs[outputsCount];
  uint32_t outputValues[outputsCount];

  InterruptIn main_crossover;
  //DigitalOut led;

  bool input50HzIsStable;
  int zeroCrossesCount;
  int ticksSinceZeroCross;
  int lastZeroCrossDurationInTicks;

  void main_crossover_rise();
};

#endif