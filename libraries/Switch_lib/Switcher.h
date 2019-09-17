/*
  Switch.h - Library for Switch temporarly relays
  Released into the public domain.
*/
#include "Arduino.h"

#include <Wire.h>
#ifndef SWITCHER_H
#define SWITCHER_H


class Switcher   //Turn on/off a device and set delay time on and off.
{  
  public:
    Switcher(int pin);
    void Update(long on, long off);
    void Pwm(long on, long off);
    void Pwm2(long on, long off);
    void Start();
    void Timer(long on);
    void Stop();
    int day;
    int st;
    
  private:
    int _pin;      // the number of the Power pin

    unsigned long _previousMillis; // will store last time DEVICE was updated

};

#endif
