#ifndef _OUTPUTS12PWM4DIGITAL_H_
#define _OUTPUTS12PWM4DIGITAL_H_

#include "mbed.h"
#include "IOutputs.h"
#include "FastPWM.h"

class Outputs12Pwm4Digital: public IOutputs {
public:
  Outputs12Pwm4Digital();
  void setOutput(uint32_t idxOutput, uint32_t value);

private:
  DigitalOut pwmOut[12];
  DigitalOut digitalOut[4];
};

#endif