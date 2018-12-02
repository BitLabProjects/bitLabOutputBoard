#include "Outputs12Pwm4Digital.h"

Outputs12Pwm4Digital::Outputs12Pwm4Digital() : pwmOut({{PB_11}, {PB_10}, {PB_1}, {PB_0}, {PA_7}, {PA_6}, {PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}}),
                                               digitalOut({{PA_5}, {PA_4}, {PA_3}, {PA_2}})
{
}

void Outputs12Pwm4Digital::setOutput(uint32_t idxOutput, uint32_t value)
{
  if (idxOutput < 12)
  {
    //pwmOut[i].write(Utils::clamp01(value / 4096.0));
    pwmOut[idxOutput] = (value < 50) ? 0 : 1;
  }
  else if (idxOutput < 16)
  {
    digitalOut[idxOutput - 12] = (value < 50) ? 0 : 1;
  }
}