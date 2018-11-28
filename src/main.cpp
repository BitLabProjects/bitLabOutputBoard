#include "bitLabCore\src\os\bitLabCore.h"
#include "bitLabCore\src\net\RingNetwork.h"
#include "modules\OutputBoard.h"
#include "modules\Outputs12Pwm4Digital.h"

bitLabCore core;
RingNetwork rn(PB_6, PB_7, false);
Outputs12Pwm4Digital output12pwm4digital;
OutputBoard ob(&output12pwm4digital);

int main()
{
  core.init();
  core.addModule(&rn);
  core.addModule(&ob);
  core.run();
}
