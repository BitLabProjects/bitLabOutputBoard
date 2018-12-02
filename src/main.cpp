#include "bitLabCore\src\os\bitLabCore.h"
#include "bitLabCore\src\net\RingNetwork.h"
#include "modules\OutputBoard.h"

bitLabCore core;
RingNetwork rn(PB_6, PB_7, false);

#ifdef TRIAC
#include "modules\Outputs8Triac.h"
Outputs8Triac outputs;
#else
#include "modules\Outputs12Pwm4Digital.h"
Outputs12Pwm4Digital outputs;
#endif
OutputBoard ob(&outputs);

int main()
{
  core.init();
  core.addModule(&rn);
  core.addModule(&ob);
  core.run();
}
