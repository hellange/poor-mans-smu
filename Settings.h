#ifndef _Settings_h
#define _Settings_h

class SettingsClass {

private:
 
public:


  float MAX_CURRENT_10mA_RANGE = 8.0; // current values set because the ADC limit is 6 volt now...
  float MAX_CURRENT_1A_RANGE = 1300.0;
  void init();
 
};

extern SettingsClass SETTINGS;

#endif _Settings_h
