#include <Switcher.h>
float hoursoflight=0.00086667;   // 1 minute = 0.0166667 hours
int daylenght=24;

int day=0;


const long lightinterval = 3600000*hoursoflight;
const long darkinterval =  3600000*daylenght-lightinterval;


Switcher lights(8);
void setup() {
  Serial.begin(9600);
 // put your setup code here, to run once:
 Serial.print("empezamos---");
lights.Start();
delay(5000);
 Serial.print("-------encendemos---");

}

void loop() {
lights.Timer(0.5);
}
