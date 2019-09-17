
#include "Switcher.h"
#include <Indio.h>
#include <Wire.h>


Switcher::Switcher(int pin)

{
  _pin = pin;
  st = LOW;

//pinMode(_pin,OUTPUT);
//digitalWrite(_pin,st);
//

  day = 0;
  }

void Switcher::Update(long on, long off)
  {
    // check to see if it's time to change the state of the DEVICE
    unsigned long currentMillis = millis();
    if((st == HIGH) && (currentMillis - _previousMillis >= 3600000*on) )
    {
      st = LOW;  // Turn it off
      _previousMillis = currentMillis;  // Remember the time
      Indio.digitalWrite(_pin, st);  // Update the actual DEVICE
//digitalWrite(_pin,st);
//
      day = day + 1;
    }
    else if ((st == LOW) && (currentMillis - _previousMillis >= 3600000*off) )
    {
      st = HIGH;  // turn it on
      _previousMillis = currentMillis;   // Remember the time
      Indio.digitalWrite(_pin, st);   // Update the actual DEVICE
//digitalWrite(_pin,st);
//
    }
  }


void Switcher::Start()
  {
    _previousMillis = 0;
    st = HIGH;
      Indio.digitalWrite(_pin, st);   // Start DEVICE
//digitalWrite(_pin,st);
//
  }
void Switcher::Stop()
  {
    _previousMillis = 0;
    st = LOW;
      Indio.digitalWrite(_pin, st);   // Start DEVICE
//digitalWrite(_pin,st);
//
  }

void Switcher::Timer(long on)
  {
    unsigned long OnTime;
    OnTime = on*60000;      //Minutes to milliseconds
    // check to see if it's time to change the state of the DEVICE
    unsigned long currentMillis = millis();
     
    if(st == HIGH  && (currentMillis - _previousMillis >= OnTime))
    {
      st = LOW;
      Indio.digitalWrite(_pin, st);   // Start DEVICE
//digitalWrite(_pin,st);
//
    }
 }

void Switcher::Pwm(long on, long off)
  {
    // check to see if it's time to change the state of the DEVICE
    unsigned long currentMillis = millis();
    if((st == HIGH) && (currentMillis - _previousMillis >= 1000*on) )
    {
      st = LOW;  // Turn it off
      _previousMillis = currentMillis;  // Remember the time
      Indio.digitalWrite(_pin, st);  // Update the actual DEVICE
//digitalWrite(_pin,st);
//
      day = day + 1;
    }
    else if ((st == LOW) && (currentMillis - _previousMillis >= 1000*off) )
    {
      st = HIGH;  // turn it on
      _previousMillis = currentMillis;   // Remember the time
      Indio.digitalWrite(_pin, st);   // Update the actual DEVICE
//digitalWrite(_pin,st);
//
    }
  }



void Switcher::Pwm2(long on, long off)
  {
    // check to see if it's time to change the state of the DEVICE
    unsigned long currentMillis = millis();
    if((st == HIGH) && (currentMillis - _previousMillis >= 1000*on) )
    {
      st = LOW;  // Turn it off
      _previousMillis = currentMillis;  // Remember the time
      digitalWrite(_pin, st);  // Update the actual DEVICE
//digitalWrite(_pin,st);
//
      day = day + 1;
    }
    else if ((st == LOW) && (currentMillis - _previousMillis >= 1000*off) )
    {
      st = HIGH;  // turn it on
      _previousMillis = currentMillis;   // Remember the time
      digitalWrite(_pin, st);   // Update the actual DEVICE
//digitalWrite(_pin,st);
//
    }
  }
