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
  if (false)
  {
    /*
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

      if (i < 12)
      {
        pwmOut[i].write(Utils::clamp01(value / 4096.0));
      }
      else
      {
        digitalOut[i - 12] = (value == 0) ? 0 : 1;
      }
    }
  }
    */
}
}

void OutputBoard::onSetOutput(int output, int value, millisec startTime, millisec duration)
{
  if (output >= 1 && output <= 16)
  {
    outputStates[output].set(value, startTime, duration);
  }
}

enum EMsgType
{
  SetLed = 1,
  CreateStoryboard = 2,
  SetTimelineEntries = 3,
  GetStoryboardChecksum = 4,
  TellStoryboardChecksum = 5,
  SetStoryboardTime = 6,
  Play = 7,
  Pause = 8,
  Stop = 9,
  DebugPrint = 255
};

// TODO Validate timelines: entries should be ordered by time
// TODO Calculate storyboard checksum to verify the storyboard upload was successful

void OutputBoard::onPacketReceived(RingPacket *p, PTxAction *pTxAction)
{
  *pTxAction = PTxAction::SendFreePacket;

  //Data content specifies the message type in the first byte, then the following are based on the message type
  auto data_size = p->header.data_size;
  if (data_size < 1)
    return; //Too short

  auto msgType = (EMsgType)p->data[0];
  switch (msgType)
  {
  case EMsgType::SetLed:
    if (data_size < 2)
      return; //Too short

    // Negate because 0 means down, the led is pulled down
    led = !p->data[1];
    break;

  case EMsgType::CreateStoryboard:
  {
    // Data structure:
    // [1] = timelines count
    // [2-5] = total duration in milliseconds
    // Repeat for 'timelines count'
    // [0] = output (starting from 1)
    // [1] = entries capacity

    // Stop if playing
    storyboardPlayer.stop();

    if (data_size < 1 + 1 + 4)
      return; //Too short

    uint8_t timelinesCount = p->data[1];
    millisec totalDuration = *((millisec *)&p->data[2]);
    if (timelinesCount > 32)
    {
      // TODO Signal: can't have too many timelines, the data size is 256 bytes
      // We use 32 here but the theoretical max is (256 - 1 - 4) / 2 = 125
      timelinesCount = 32;
    }

    storyboard.create(timelinesCount, totalDuration);
    for (int i = 0; i < timelinesCount; i++)
    {
      auto offset = 6 + i * 2;
      uint8_t output = p->data[offset + 0];
      uint8_t entriesCapacity = p->data[offset + 1];
      storyboard.addTimeline(0, output, entriesCapacity);
    }
  }
  break;

  case EMsgType::SetTimelineEntries:
  {
    // Data structure:
    // [1] = output (starting from 1)
    // [2] = first entry idx
    // [3] = entries count
    // Repeat for 'entries count'
    // [0-3]  = time
    // [4-7]  = value
    // [8-11] = duration

    // Stop if playing
    storyboardPlayer.stop();

    if (data_size < 1 + 1 + 1 + 1)
      return; //Too short

    uint8_t output = p->data[1];
    uint8_t firstEntryIdx = p->data[2];
    uint8_t entriesCount = p->data[3];
    if (entriesCount > 20)
    {
      // TODO Signal: can't have too many timelines, the data size is 256 bytes
      // We use 20 here but the theoretical max is (256 - 1 - 1 - 1) / 12 = 21
      entriesCount = 20;
    }

    auto timeline = storyboard.getTimeline(output);
    if (timeline == NULL)
    {
      //TODO Signal timeline not found
      return;
    }

    auto entryIdx = firstEntryIdx;
    for (int i = 0; i < entriesCount; i++)
    {
      auto offset = 4 + i * 12;
      millisec time = p->getDataInt32(offset + 0);
      int32_t value = p->getDataInt32(offset + 4);
      millisec duration = p->getDataInt32(offset + 8);
      timeline->setEntry(entryIdx, time, value, duration);
      entryIdx += 1;
    }
  }
  break;

  case EMsgType::GetStoryboardChecksum:
  {
    // Data structure: Empty

    uint32_t crc32 = storyboard.calcCrc32(0);

    // Fill header
    p->header.data_size = 1 + 4;
    p->header.control = 1;
    p->header.dst_address = p->header.src_address; // Respond to the sender
    p->header.src_address = ringNetwork->getAddress();
    p->header.ttl = RingNetworkProtocol::ttl_max;
    // Fill data
    p->data[0] = EMsgType::TellStoryboardChecksum;
    *(uint32_t *)(&p->data[1]) = crc32;
    //
    *pTxAction = PTxAction::Send;
  }
  break;

  case EMsgType::SetStoryboardTime:
  {
    // Data structure:
    // [1-4] = storyboard time

    if (data_size < 1 + 4)
      return; //Too short

    millisec storyboardTime = *((millisec *)&p->data[1]);
    // TODO
    //storyboardPlayer.setStoryboardTime(storyboardTime);
  }
  break;

  case EMsgType::Play:
  {
    // Data structure: Empty
    storyboardPlayer.play();
  }
  break;

  case EMsgType::Pause:
  {
    // Data structure: Empty
    storyboardPlayer.pause();
  }
  break;

  case EMsgType::Stop:
  {
    // Data structure: Empty
    storyboardPlayer.stop();
  }
  break;
  

  default:
    // Unknown, discard
    break;
  }
}
