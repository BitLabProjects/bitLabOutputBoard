#ifndef _OUTPUTBOARD_H_
#define _OUTPUTBOARD_H_

#include "..\bitLabCore\src\os\bitLabCore.h"
#include "..\bitLabCore\src\net\RingNetwork.h"
#include "..\bitLabCore\src\storyboard\Storyboard.h"
#include "..\bitLabCore\src\storyboard\StoryboardPlayer.h"
#include "IOutputs.h"
#include "OutputState.h"

class OutputBoard : public CoreModule
{
public:
  OutputBoard(IOutputs* outputs);

  // --- CoreModule ---
  const char* getName() { return "OutputBoard"; }
  void init(const bitLabCore*);
  void mainLoop();
  void tick(millisec timeDelta);
  // ------------------

private:
  uint32_t hardwareId;
  RingNetwork* ringNetwork;
  DigitalOut led;
  volatile millisec64 time;
  // trimInterval is the number of milliseconds after which a millisecond is added or removed
  volatile millisec trimInterval;
  volatile millisec trimCounter;
  volatile millisec64 timeDeltaForPlay;
  millisec64 timeSinceLastOutputRefresh;

  static const uint32_t OutputCount = 16;
  OutputState outputStates[OutputCount];
  IOutputs* outputs;

  Storyboard storyboard;
  StoryboardPlayer storyboardPlayer;

  void onPacketReceived(RingPacket*, PTxAction*);

  void onSetOutput(const PlayBufferEntry* pbEntry);
};

#endif