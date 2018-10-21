#ifndef _OUTPUTBOARD_H_
#define _OUTPUTBOARD_H_

#include "..\bitLabCore\src\os\bitLabCore.h"
#include "..\bitLabCore\src\net\RingNetwork.h"
#include "..\bitLabCore\src\storyboard\Storyboard.h"
#include "..\bitLabCore\src\storyboard\StoryboardPlayer.h"
#include "FastPWM.h"
#include "OutputState.h"

class OutputBoard : public CoreModule
{
public:
  OutputBoard();

  // --- CoreModule ---
  const char* getName() { return "OutputBoard"; }
  void init(const bitLabCore*);
  void mainLoop();
  void tick(millisec timeDelta);
  // ------------------

private:
  RingNetwork* ringNetwork;
  DigitalOut led;
  millisec64 time;
  millisec64 timeSinceLastOutputRefresh;

  static const uint32_t OutputCount = 16;
  FastPWM pwmOut[12];
  DigitalOut digitalOut[4];
  OutputState outputStates[OutputCount];

  Storyboard storyboard;
  StoryboardPlayer storyboardPlayer;

  void onPacketReceived(RingPacket*, PTxAction*);

  void onSetOutput(int output, int value, millisec startTime, millisec duration);
};

#endif