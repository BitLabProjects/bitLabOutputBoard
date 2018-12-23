#ifndef _OUTPUTS2DIGITAL14PWM_H_
#define _OUTPUTS2DIGITAL14PWM_H_

#include "mbed.h"
#include "PinNames.h"
#include "..\bitLabCore\src\utils.h"
#include "IOutputs.h"
#include "FastPWM.h"

class Outputs2Digital14Pwm: public IOutputs
{
public:
  Outputs2Digital14Pwm();

  void setOutput(uint32_t idxOutput, uint32_t value);

private:
  static const uint32_t ticksPerSecond = 10 * 1000;
  static const uint32_t nominalZeroCrossPerSecond = 100;
  static const uint32_t gateOnTimeTicks = 10;
  static const uint32_t nominalTicksBetweenZeroCrosses = ticksPerSecond / nominalZeroCrossPerSecond;
  static const uint32_t nominalTicksBetweenZeroCrossesMaxDelta = nominalTicksBetweenZeroCrosses / 5;

  static const uint32_t outputsCount = 16;
  static const uint32_t outputsDigitalCount = 6;
  static const uint32_t outputsPwmCount = 10;
  static const uint32_t outputsPwm_PeriodUs = 1000;
  DigitalOut outputsDigital[outputsDigitalCount];
  PwmOut outputsPwm[outputsPwmCount];
  uint32_t outputValues[outputsCount];

  int ticksSinceZeroCross;
};

#endif