#include "Outputs16DigitalPwm.h"

#include "..\bitLabCore\src\os\os.h"

Outputs16DigitalPwm::Outputs16DigitalPwm() : outputs({{PB_11}, {PB_10}, {PB_1}, {PB_0}, {PA_7}, {PA_6}, {PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}, {PA_5}, {PA_4}, {PA_3}, {PA_2}})
{
  ticksSinceZeroCross = 0;

  for (uint32_t idxOutput = 0; idxOutput < outputsCount; idxOutput++)
  {
    outputValues[idxOutput] = 0;
  }
}

void Outputs16DigitalPwm::setOutput(uint32_t idxOutput, uint32_t value)
{
  if (idxOutput < outputsCount)
  {
    // Avoid borderline values
    if (value < 128)
      value = 0;
    if (value > 3967)
      value = 4096;
    outputValues[idxOutput] = value;
  }
}

void Outputs16DigitalPwm::onTick()
{
  ticksSinceZeroCross += 1;

  if (ticksSinceZeroCross == nominalTicksBetweenZeroCrosses)
  {
    ticksSinceZeroCross = 0;
  }

  // set/reset each out based on percent
  for (uint32_t idxOutput = 0; idxOutput < outputsCount; idxOutput++)
  {
    int valueToSet;
    // ">> 12" is equal to "/ 4096"
    int low_ticks = (nominalTicksBetweenZeroCrosses * (4096 - (outputValues[idxOutput]))) >> 12;

    if (ticksSinceZeroCross >= low_ticks)
      valueToSet = 1;
    else
      valueToSet = 0;

    outputs[idxOutput] = valueToSet;
  }
}