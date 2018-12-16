#include "Outputs16DigitalPwm.h"

#include "..\bitLabCore\src\os\os.h"

Outputs16DigitalPwm::Outputs16DigitalPwm() : outputs({{PB_11}, {PB_10}, {PB_1}, {PB_0}, {PA_7}, {PA_6}, {PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}, {PA_5}, {PA_4}, {PA_3}, {PA_2}}),
                                 main_crossover(D5)
                                 //led(PC_13)
{
  input50HzIsStable = 0;
  ticksSinceZeroCross = 0;
  lastZeroCrossDurationInTicks = 0;
  zeroCrossesCount = 0;

  for (uint32_t idxOutput = 0; idxOutput < outputsCount; idxOutput++)
  {
    outputValues[idxOutput] = 0;
  }

  if (!simulateVAC)
    main_crossover.rise(callback(this, &Outputs16DigitalPwm::main_crossover_rise));
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

  //Somehow using a ticker for simulation gives wrong timings...
  if (simulateVAC)
  {
    if (ticksSinceZeroCross == nominalTicksBetweenZeroCrosses)
    {
      main_crossover_rise();
    }
  }

  // set/reset each out based on percent
  for (uint32_t idxOutput = 0; idxOutput < outputsCount; idxOutput++)
  {
    int valueToSet;
    // ">> 12" is equal to "/ 4096"
    int low_ticks = (nominalTicksBetweenZeroCrosses * (4096 - (outputValues[idxOutput]))) >> 12;

    if (simulateVAC)
    {
      if (ticksSinceZeroCross >= low_ticks)
        valueToSet = 1;
      else
        valueToSet = 0;
    }
    else
    {
      // pulse for TRIAC activation
      if ((ticksSinceZeroCross < low_ticks) || (ticksSinceZeroCross > (low_ticks + gateOnTimeTicks)))
        valueToSet = 1;
      else
        valueToSet = 0;
    }

    outputs[idxOutput] = valueToSet;
  }
}

void Outputs16DigitalPwm::main_crossover_rise()
{
  lastZeroCrossDurationInTicks = ticksSinceZeroCross;
  ticksSinceZeroCross = 0;

  /*
  //NOMINAL_100HZ_TICKS_PER_RISE is twice the nominal 50Hz duration in ticks
  //twice because we have 100 zero crossing for a 50Hz sinusoidal wave
  //Force all outputs to zero if the last measured duration is more than 20% off than the nominal one
  //This detects the condition where we don't have a stable 50Hz sinusoidal wave
  input50HzIsStable = true || Utils::absDiff(lastZeroCrossDurationInTicks, nominalTicksBetweenZeroCrosses) < nominalTicksBetweenZeroCrossesMaxDelta;
  */

  zeroCrossesCount += 1;
  if (zeroCrossesCount == nominalZeroCrossPerSecond)
  {
    zeroCrossesCount = 0;
    //led = !led;
  }
}