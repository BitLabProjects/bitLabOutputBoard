#include "bitLabCore\src\os\bitLabCore.h"
#include "bitLabCore\src\net\RingNetwork.h"
#include "modules\OutputBoard.h"


int main()
{
  bitLabCore core;
  core.init();
  core.addModule(new RingNetwork(PB_6, PB_7, core.getHardwareId(), false));
  core.addModule(new OutputBoard());
  core.run();
  
  /*
  Serial serial(PB_6, PB_7);
  serial.baud(1200);
  DigitalOut myled(PC_13);
  while (1)
  {
    myled = 1;
    wait(0.4);
    serial.puts("Ciao mondo!\r\n");
    myled = 0;
    wait(0.4);
  }*/
}
