#include <Indio.h>  
#include <Wire.h>
#include <Switcher.h>

Switcher lights(1);
void setup() {
  // put your setup code here, to run once:

}

void loop() {
lights.Update(1,1);
}
