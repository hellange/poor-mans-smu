#include <Arduino.h>
class Ada4254Class {

public:
  void ada4254(bool on); 
  void ada4254_2(); 
  void ada4254_3(bool on);
  void ada4254_4();
  void ada4254_5_gainx1d25();
  void ada4254_5_gain();
  void relay(bool on); // Does not belong here;
};

extern Ada4254Class ADA4254;