#include "ZeroCalibration.h"
#include "Debug.h"

extern ADCClass SMU[];


int timeBeforeAutoNull = millis() + 5000;
int samplingRateBeforeZeroCal;
int filterSizeBeforeZeroCalV;
int filterSizeBeforeZeroCalC;

int ZeroCalibrationlass::getProgress() {
  int seconds = (millis() - zeroCalstartTime) / 1000;
  int percent = 100 * seconds/21; // given 21 seconds total
  return percent;
}
void ZeroCalibrationlass::startNullCalibration(OPERATION_TYPE operationType_) {
  operationType = operationType_;
  //timeBeforeAutoNull = millis(); // Is this needed anymore????
  nullCalibrationDone0 = false;
  nullCalibrationDone1 = false;
  nullCalibrationDone2 = false;
  nullCalibrationDone3 = false;
  nullCalibrationReady = false;
  samplingRateBeforeZeroCal = SMU[0].getSamplingRate();
  filterSizeBeforeZeroCalV = V_FILTERS.filterSize;
  filterSizeBeforeZeroCalC = C_FILTERS.filterSize;


  autoNullStarted = true;

  // set null to 0
  if (operationType == SOURCE_VOLTAGE) {
    DEBUG.println("Resetting null adjust for voltage source");
    V_CALIBRATION.setNullValueVol(0.0, MILLIAMP10);
    V_CALIBRATION.setNullValueVol(0.0, AMP1);
  } else {
    DEBUG.println("Resetting null adjust for current source");
    C_CALIBRATION.setNullValueCur(0.0, MILLIAMP10);
    C_CALIBRATION.setNullValueCur(0.0, AMP1);
  }
  zeroCalstartTime=millis();
  
}

void ZeroCalibrationlass::handleAutoNull() {

//   DEBUG.print("x ");
//   DEBUG.print(nullCalibrationDone0);
//   DEBUG.print(" ");
//   DEBUG.println(timeBeforeAutoNull < millis());
  if (autoNullStarted && !nullCalibrationDone0) {
    DEBUG.println("Performing auto null...");
    C_CALIBRATION.useCalibratedValues = false;
    V_CALIBRATION.useCalibratedValues = false;
    SMU[0].setSamplingRate(10);
    current_range = MILLIAMP10;
       SMU[0].setCurrentRange(current_range, operationType);
      if (operationType == SOURCE_VOLTAGE) {
        DEBUG.println("Source 0V at output (current range 10mA)");
        SMU[0].fltSetCommitVoltageSource(0, true);
       } else {
        DEBUG.println("Source 0 current at output (current range 10mA)");
        SMU[0].fltSetCommitCurrentSource(0);
       }
    
    //V_FILTERS.init();
    V_FILTERS.setFilterSize(20);
    C_FILTERS.setFilterSize(20);

    nullCalibrationDone0 = true;
   
  }
  
  if (autoNullStarted && !nullCalibrationDone1 && zeroCalstartTime + 10000 < millis() ) {
 
    //float v = V_STATS.rawValue; 
    if (operationType == SOURCE_VOLTAGE) {
      float v = V_FILTERS.mean;   
      V_CALIBRATION.setNullValueVol(v, current_range);
      DEBUG.print("Found offset when sourcing voltage (10mA current range):");  
      DEBUG.println(v,3);  
    } else {
      float v = C_FILTERS.mean; 
      //float v = SMU[0].measureCurrent(current_range);
      C_CALIBRATION.setNullValueCur(v, current_range);
      DEBUG.print("Found offset when sourcing current (10mA current range):");  
      DEBUG.println(v,3);  
    }

    nullCalibrationDone1 = true;
  } 
 
  if (autoNullStarted && !nullCalibrationDone2 && zeroCalstartTime + 11000 < millis() ) {
     current_range = AMP1;
     SMU[0].setCurrentRange(current_range, operationType);

    if (operationType == SOURCE_VOLTAGE) {
      DEBUG.println("Source 0V at output (current range 1A)");
      SMU[0].fltSetCommitVoltageSource(0, true);
 
    } else {
      DEBUG.println("Source 0 current at output (current range 1A)");
      SMU[0].fltSetCommitCurrentSource(current_range);
 
    }
    
    nullCalibrationDone2 = true;
  }
  
  if (autoNullStarted && !nullCalibrationDone3 && zeroCalstartTime + 21000 < millis()) {
   if (operationType == SOURCE_VOLTAGE) {
      float v = V_FILTERS.mean;   
      V_CALIBRATION.setNullValueVol(v, current_range);
      DEBUG.print("Found offset when sourcing voltage (1A current range):");  
      DEBUG.println(v,3);  
    } else {
      float v = C_FILTERS.mean; 
      //float v = SMU[0].measureCurrent(current_range);
      C_CALIBRATION.setNullValueCur(v, current_range);
      DEBUG.print("Found offset when sourcing current (1A current range):");  
      DEBUG.println(v,3);  
    }
    
    
    nullCalibrationDone3 = true;
    //V_FILTERS.init(1234);
    V_FILTERS.setFilterSize(20);
    //C_FILTERS.init(2345);
    C_FILTERS.setFilterSize(20);
    SMU[0].setSamplingRate(samplingRateBeforeZeroCal);
    V_CALIBRATION.useCalibratedValues = true;
    C_CALIBRATION.useCalibratedValues = true;

    V_FILTERS.setFilterSize(filterSizeBeforeZeroCalV);
    C_FILTERS.setFilterSize(filterSizeBeforeZeroCalC);

    autoNullStarted = false;
    nullCalibrationReady = true;
  } 


}