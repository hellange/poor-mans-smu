#include "ZeroCalibration.h"

extern ADCClass SMU[];


int timeBeforeAutoNull = millis() + 5000;
bool nullCalibrationDone0 = false;
bool nullCalibrationDone1 = false;



void ZeroCalibrationlass::startNullCalibration(OPERATION_TYPE operationType_) {
  operationType = operationType_;
  Serial.println("Force auto null...");
  timeBeforeAutoNull = millis(); // Is this needed anymore????
  nullCalibrationDone0 = false;
  nullCalibrationDone1 = false;
  nullCalibrationDone2 = false;
  autoNullStarted = true;

  // set null to 0
  if (operationType == SOURCE_VOLTAGE) {
    V_CALIBRATION.setNullValueVol(0.0, MILLIAMP10);
    V_CALIBRATION.setNullValueVol(0.0, AMP1);
  } else {
    V_CALIBRATION.setNullValueCur(0.0, MILLIAMP10);
    V_CALIBRATION.setNullValueCur(0.0, AMP1);
  }
  //TODO: Do the same when calibration on constant current mode as well !


}


void ZeroCalibrationlass::handleAutoNull() {

//   Serial.print("x ");
//   Serial.print(nullCalibrationDone0);
//   Serial.print(" ");
//   Serial.println(timeBeforeAutoNull < millis());
  if (autoNullStarted && !nullCalibrationDone0 && timeBeforeAutoNull < (int)millis()) {
    Serial.println("Performing auto null...");
    C_CALIBRATION.useCalibratedValues = false;
    V_CALIBRATION.useCalibratedValues = false;
    current_range = MILLIAMP10;
    //Serial.println("Null calibration initiated...");
    SMU[0].setCurrentRange(current_range, operationType);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0, true);
    } else {
      SMU[0].fltSetCommitCurrentSource(0);
    }
    //V_FILTERS.init();
    V_FILTERS.setFilterSize(10);
    C_FILTERS.setFilterSize(10);

    SMU[0].setSamplingRate(20);
    nullCalibrationDone0 = true;
   
  }
  int msWaitPrCal = 5000;
  if (autoNullStarted && !nullCalibrationDone1 && timeBeforeAutoNull + msWaitPrCal < (int)millis()) {
    //float v = V_STATS.rawValue; 
    if (operationType == SOURCE_VOLTAGE) {
      float v = V_FILTERS.mean;   
      V_CALIBRATION.setNullValueVol(v, current_range);
      Serial.print("Removed voltage offset from 10mA range:");  
      Serial.println(v,3);  
    } else {
      float v = C_FILTERS.mean;   
      V_CALIBRATION.setNullValueCur(v, current_range);
      Serial.print("Removed current offset from 10mA range:");  
      Serial.println(v,3);  
    }

    nullCalibrationDone1 = true;
  } 
 
  if (autoNullStarted && !nullCalibrationDone2 && /*timeAtStartup + */timeBeforeAutoNull + msWaitPrCal + 100 < (int)millis()) {
    current_range = AMP1;
    //Serial.println("Null calibration initiated...");
    SMU[0].setCurrentRange(current_range, operationType);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0, true);
    } else {
      SMU[0].fltSetCommitCurrentSource(0);
    }
  }
  if (autoNullStarted && !nullCalibrationDone2 && /*timeAtStartup +*/ timeBeforeAutoNull + msWaitPrCal*2 < (int)millis()) {
   if (operationType == SOURCE_VOLTAGE) {
      float v = V_FILTERS.mean;   
      V_CALIBRATION.setNullValueVol(v, current_range);
      Serial.print("Removed voltage offset from 10mA range:");  
      Serial.println(v,3);  
    } else {
      float v = C_FILTERS.mean;   
      V_CALIBRATION.setNullValueCur(v, current_range);
      Serial.print("Removed current offset from 10mA range:");  
      Serial.println(v,3);  
    }
    
    
    nullCalibrationDone2 = true;
    //V_FILTERS.init(1234);
    V_FILTERS.setFilterSize(5);
    //C_FILTERS.init(2345);
    C_FILTERS.setFilterSize(5);
    V_CALIBRATION.useCalibratedValues = true;
        C_CALIBRATION.useCalibratedValues = true;

    autoNullStarted = false;

  } 

}