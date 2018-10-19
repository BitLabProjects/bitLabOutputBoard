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
    to = newTo;
    startTime = newStartTime;
    duration = newDuration;
  }
  inline void update(int time)
  {
    if (duration <= 0)
    {
      value = startTime > time ? from : to;
    }
    else
    {
      int delta = to - from;
      float t = (((float)time) - startTime) / duration;
      t = Utils::clamp01(t); //Don't go above 1, so the result is between 'from' and 'to' values
      value = from + (int)(delta * t);
    }
  }
};

#endif