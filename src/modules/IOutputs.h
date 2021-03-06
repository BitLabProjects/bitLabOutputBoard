#ifndef _IOUTPUTS_H_
#define _IOUTPUTS_H_

#include "..\bitLabCore\src\os\types.h"

class IOutputs {
public:
  virtual void setOutput(uint32_t idxOutput, uint32_t value) = 0;
  virtual void onTick() {};

protected:
  IOutputs() {
  }
};

#endif