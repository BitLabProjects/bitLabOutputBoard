#include "OutputBoard.h"

OutputBoard::OutputBoard(IOutputs *outputs) : led(PC_13),
                                              outputs(outputs),
                                              storyboard(),
                                              storyboardPlayer(&storyboard, callback(this, &OutputBoard::onSetOutput))
{
  //led = 0; //0 means on, the led is pulled down
  time = 0;
  timeDeltaForPlay = 0;
  timeSinceLastOutputRefresh = 0;

  for (uint32_t i = 0; i < OutputCount; i++)
  {
    outputStates[i].reset();
  }
}

void OutputBoard::init(const bitLabCore *core)
{
  if (true)
  {
    // TODO Understand if this works and is correct
    // TIM4_IRQn is the timer used by mbed Ticker in STM32F1
    // see framework-mbed\targets\TARGET_STM\TARGET_STM32F1\TARGET_NUCLEO_F103RB\device\hal_tick.h
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_SetPriority(TIM4_IRQn, 7, 0);
  }

  hardwareId = core->getHardwareId();
  ringNetwork = (RingNetwork *)core->findModule("RingNetwork");
  if (ringNetwork != NULL)
  {
    ringNetwork->attachOnPacketReceived(callback(this, &OutputBoard::onPacketReceived));
  }
}

void OutputBoard::mainLoop()
{
  if (time > 1000)
  {
    time = 0;
    led = !led;
  }

  storyboardPlayer.fillPlayBuffer();
}

void OutputBoard::tick(millisec timeDelta)
{
  if (timeDelta == 0)
    return;

  time += timeDelta;
  timeDeltaForPlay += timeDelta;

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

      outputs->SetOutput(i, value);
    }
  }
}

void OutputBoard::onSetOutput(const PlayBufferEntry *pbEntry)
{
  int outputId = pbEntry->outputId;
  if (outputId >= 1 && outputId <= 16)
  {
    outputStates[outputId - 1].set(pbEntry->entry.value,
                                   pbEntry->entry.time,
                                   pbEntry->entry.duration);
  }
}

enum EMsgType
{
  SetLed = 1,
  CreateStoryboard = 2,
  SetTimelineEntries = 3,
  GetState = 4,
  TellState = 5,
  SetStoryboardTime = 6,
  Play = 7,
  Pause = 8,
  Stop = 9,
  SetOutput = 10,
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
    //led = !p->data[1];
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
    millisec totalDuration = p->getDataInt32(2);
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
      storyboard.addTimeline(hardwareId, output, entriesCapacity);
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
      timeline->add(time, value, duration);
      entryIdx += 1;
    }
  }
  break;

  case EMsgType::GetState:
  {
    // Data structure: Empty

    uint32_t crc32 = storyboard.calcCrc32(0);

    // Fill header
    p->header.data_size = 1 + 4 + 4;
    p->header.control = 1;
    p->header.dst_address = p->header.src_address; // Respond to the sender
    p->header.src_address = ringNetwork->getAddress();
    p->header.ttl = RingNetworkProtocol::ttl_max;
    // Fill data
    p->data[0] = EMsgType::TellState;
    p->setDataUInt32(1, crc32);
    p->setDataUInt32(1 + 4, storyboardPlayer.getStoryboardTime());
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

  case EMsgType::SetOutput:
  {
    // Data structure:
    // [1] = OutputId
    // [2-5] = value

    if (storyboardPlayer.isPlaying())
      return; // Can't set output manually when playing

    if (data_size < 1 + 1 + 4)
      return; // Too short

    uint8_t outputId = p->data[1];
    uint32_t value = p->getDataUInt32(2);

    if (outputId < 1 || outputId > OutputCount)
      return; // Invalid output

    // Use a negative start time so it will be applied: storyboard time is always positive
    outputStates[outputId - 1].set(value, -1, 0);
  }
  break;

  default:
    // Unknown, discard
    break;
  }
}
