#include "OutputBoard.h"

OutputBoard::OutputBoard() : led(PC_13),
                             pwmOut({{PA_8}, {PA_9}, {PA_10}, {PA_11}, {PA_15}, {PB_3}, {PB_4}, {PB_5}, {PB_11}, {PB_10}, {PB_1}, {PB_0}}),
                             digitalOut({{PB_12}, {PB_13}, {PB_14}, {PB_15}}),
                             storyboard(),
                             storyboardPlayer(&storyboard, callback(this, &OutputBoard::onSetOutput))
{
  led = 0; //0 means on, the led is pulled down
  time = 0;
  timeSinceLastOutputRefresh = 0;

  for (uint32_t i = 0; i < OutputCount; i++)
  {
    outputStates[i].reset();
  }
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
  if (time > 3000)
  {
    time = 0;
    led = !led;
  }

  storyboardPlayer.fillPlayBuffer();
}

void OutputBoard::tick(millisec timeDelta)
{
  time += timeDelta;

  // Advance the storyboard
  storyboardPlayer.advance(timeDelta);

  // Update outputs at 60Hz
  timeSinceLastOutputRefresh += timeDelta;
  if (timeSinceLastOutputRefresh > 1000 / 60)
  {
    timeSinceLastOutputRefresh = 0;

    auto storyboardTime = storyboardPlayer.getStoryboardTime();
    for (uint32_t i = 0; i < OutputCount; i++)
    {
      outputStates[i].update(storyboardTime);
      int value = outputStates[i].value;

      if (i < 12) {
        pwmOut[i].write(Utils::clamp01(value / 4096.0));
      } else {
        digitalOut[i - 12] = (value == 0) ? 0 : 1;
      }
    }
  }
}

void OutputBoard::onSetOutput(int output, int value, millisec startTime, millisec duration)
{
  if (output >= 1 && output <= 16)
  {
    outputStates[output].set(value, startTime, duration);
  }
}

void OutputBoard::onPacketReceived(RingPacket *p, PTxAction *pTxAction)
{
  *pTxAction = PTxAction::SendFreePacket;

  //Data content specifies the message type in the first byte, then the following are based on the message type
  auto data_size = p->header.data_size;
  if (data_size < 1)
    return; //Too short

  auto msgType = p->data[0];
  if (msgType == 1)
  { //Set output
    if (data_size < 2)
      return; //Too short

    // Negate because 0 means down, the led is pulled down
    led = !p->data[1];
    return;
  }
  else
  {
    // Unknown message type, discard
    return;
  }
}
