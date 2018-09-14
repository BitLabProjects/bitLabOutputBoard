#include "bitLabCore\src\os\bitLabCore.h"
#include "bitLabCore\src\net\RingNetwork.h"

bitLabCore core;

int main()
{
  core.init();
  core.addModule(new RingNetwork(A9, A10));
  core.run();

  /*
  DigitalOut myled(A0);
  while (1)
  {
    myled = 1;
    wait(0.4);
    myled = 0;
    wait(0.4);
  }
  */
}
