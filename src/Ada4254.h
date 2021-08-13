#include <Arduino.h>
class Ada4254Class {

public:
  void ada4254_id(); 
  void ada4254_2(bool on); 
  void ada4254_3(bool on);
  void ada4254_4();
  void ada4254_5_gainx1d25();
  void ada4254_5_gain();
  void ada4254_setgain(int gain);

    void ada4254_reset();
    void ada4254_clear_analog_error();
        void ada4254_check();


  void relay(bool on); // Does not belong here;
};

extern Ada4254Class ADA4254;