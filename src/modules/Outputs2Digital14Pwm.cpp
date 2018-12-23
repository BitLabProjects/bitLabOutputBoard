#include "Outputs2Digital14Pwm.h"

#include "..\bitLabCore\src\os\os.h"

// outputsDigital({{PB_11}, {PB_10}}),
// outputsPwm({{PB_1}, {PB_0}, {PA_7}, {PA_6}, {PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}, {PA_5}, {PA_4}, {PA_3}, {PA_2}})
Outputs2Digital14Pwm::Outputs2Digital14Pwm() : outputsDigital({{PB_11}, {PB_10}, {PA_5}, {PA_4}, {PA_3}, {PA_2}}),
                                               outputsPwm({{PB_1}, {PB_0}, {PA_7}, {PA_6}, {PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}})
{
  ticksSinceZeroCross = 0;

  for (uint32_t idxOutput = 0; idxOutput < outputsCount; idxOutput++)
  {
    outputValues[idxOutput] = 0;
  }
  for (uint32_t idxOutput = 0; idxOutput < outputsDigitalCount; idxOutput++)
  {
    outputsDigital[idxOutput] = 0;
  }
  for (uint32_t idxOutput = 0; idxOutput < outputsPwmCount; idxOutput++)
  {
    outputsPwm[idxOutput].period_us(outputsPwm_PeriodUs);
    outputsPwm[idxOutput].pulsewidth_us(0);
  }
}

void Outputs2Digital14Pwm::setOutput(uint32_t idxOutput, uint32_t value)
{
  if (idxOutput >= outputsCount) {
    return;
  }
  if (outputValues[idxOutput] == value) {
    return;
  }
  outputValues[idxOutput] = value;

  if (idxOutput < outputsDigitalCount) {
    outputsDigital[idxOutput] = value >= 2048;
  } else if (idxOutput - outputsDigitalCount < outputsPwmCount ) {
    auto idxOutPwm = (idxOutput - outputsDigitalCount);
    outputsPwm[idxOutPwm].pulsewidth_us((outputsPwm_PeriodUs * value) >> 12);
  }
}
