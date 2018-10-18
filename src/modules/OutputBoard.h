#ifndef _OUTPUTBOARD_H_
#define _OUTPUTBOARD_H_

#include "..\bitLabCore\src\os\bitLabCore.h"
#include "..\bitLabCore\src\net\RingNetwork.h"

class OutputBoard : public CoreModule
{
public:
  OutputBoard();

  // --- CoreModule ---
  const char* getName() { return "OutputBoard"; }
  void init(const bitLabCore*);
  void mainLoop();
  void tick(millisec64 timeDelta);
  // ------------------

private:
  RingNetwork* ringNetwork;
  DigitalOut led;
  millisec64 time;

  void onPacketReceived(RingPacket*, PTxAction*);
};

#endif