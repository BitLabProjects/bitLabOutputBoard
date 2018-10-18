#include "OutputBoard.h"

OutputBoard::OutputBoard(): led(PC_13)
{
  led = 0; //0 means on, the led is pulled down
  time = 0;
}

void OutputBoard::init(const bitLabCore *core)
{
  ringNetwork = (RingNetwork *)core->findModule("RingNetwork");
  if (ringNetwork != NULL)
  {
    ringNetwork->attachOnPacketReceived(callback(this, &OutputBoard::onPacketReceived));
  }
}

void OutputBoard::mainLoop()
{
  if (time > 3000) {
    time = 0;
    led = !led;
  }
}

void OutputBoard::tick(millisec64 timeDelta)
{
  time += timeDelta;
}

void OutputBoard::onPacketReceived(RingPacket* p, PTxAction* pTxAction)
{
  *pTxAction = PTxAction::SendFreePacket;

  //Data content specifies the message type in the first byte, then the following are based on the message type
  auto data_size = p->header.data_size;
  if (data_size < 1) return; //Too short

  auto msgType = p->data[0];
  if (msgType == 1) { //Set output
    if (data_size < 2) return; //Too short

    // Negate because 0 means down, the led is pulled down
    led = !p->data[1];
    return;

  } else {
    // Unknown message type, discard
    return;
  }
}
