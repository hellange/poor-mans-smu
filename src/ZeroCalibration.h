#ifndef ZEROCALIBRATION_H
#define ZEROCALIBRATION_H
#include "operations.h"
#include "SMU_HAL.h"
//#include <SPI.h>
//#include "GD2.h"
//#include "volt_display.h"
//#include "Mainmenu.h"
#include "Calibration.h"


class ZeroCalibrationlass {

private:
  SMU_HAL *SMU1;
  OPERATION_TYPE operationType;
  CURRENT_RANGE current_range; // was "global" in poormanssmu... Does this work after moving to separate class ?
  unsigned long zeroCalstartTime;

  bool nullCalibrationDone0 = false;
  bool nullCalibrationDone1 = false;
  bool nullCalibrationDone2 = false;
  bool nullCalibrationDone3 = false;

public:
  void init(SMU_HAL &SMU);
  void startNullCalibration(OPERATION_TYPE operationType);
  void handleAutoNull();
  bool autoNullStarted = false;
  bool nullCalibrationReady = false;

  int getProgress();
};



extern ZeroCalibrationlass ZEROCALIBRATION;
#endif
