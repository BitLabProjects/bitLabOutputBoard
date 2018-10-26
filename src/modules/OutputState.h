#ifndef _OUTPUTSTATE_H_
#define _OUTPUTSTATE_H_

#include "..\bitLabCore\src\os\types.h"
#include "..\bitLabCore\src\utils.h"

struct OutputState
{
  int value;
  int from;
  int to;
  millisec startTime;
  millisec duration;

  inline void reset()
  {
    value = 0;
    from = 0;
    to = 0;
    startTime = 0;
    duration = 0;
  }
  inline void set(int newTo, millisec newStartTime, millisec newDuration)
  {
    from = value;
    to = Utils::clamp(newTo, 0, 4096);
    startTime = newStartTime;
    duration = Utils::clamp(newDuration, 0, 60 * 1000);
  }
  void update(int time)
  {
    int timeSinceStart = time - startTime;
    if (timeSinceStart < 0)
    {
      value = from;
    }
    else if (timeSinceStart >= duration)
    {
      value = to;
    }
    else
    {
      /*
      delta = 60            [0 - 4_096]
      duration = 1500       [0 - 60_000]
      timeSinceStart = 700  [0 - 60_000]
      deltaSinceStart = delta * timeSinceStart / duration
      We don't overflow multiplicating before dividing because
      delta * timeSinceStart <= 4_096 * 60_000 <= 245_760_000
      */

      int delta = to - from;
      value = from + (delta * timeSinceStart) / duration;

      //Slow because floating point is used
      //float t = (((float)time) - startTime) / duration;
      //t = Utils::clamp01(t); //Don't go above 1, so the result is between 'from' and 'to' values
      //value = from + (int)(delta * t);
    }
  }
};

#endif