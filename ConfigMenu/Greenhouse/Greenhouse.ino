





  // LIBRERIAS INDUSTRUINO //

#include <Indio.h>
#include <Wire.h>
#include <UC1701.h> //Screen library
#include <Switcher.h> // Temporizators for Digital Channels
#include <SHT1x.h>   // SHT10 sensor Library

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONESEC (1000UL * 1)
#define TENSEC (1000UL * 10)
#define ONEMIN (1000UL * 60 * 1)
#define FIVEMIN (1000UL * 60 * 5)
#define TEMP_READ_DELAY 800 //can only read digital temp sensor every ~750ms

//---------------------------------------------------------------------------------------------------------------------------------------------
// MENU DEFINES
//---------------------------------------------------------------------------------------------------------------------------------------------

//menu defines

//- initial cursor parameters
int coll = 0; //column counter for cursor - always kept at 0 in this demo (left side of the screen)
int channel = 0; //Counter is controlled by the up&down buttons on the membrane panel. Has double use; 1. As row controller for the cursor (screen displays 6 rows of text, counting from 0 to 5). 2. As editor for numerical values shown on screen
int lastChannel = 0; //keeps track of previous 'channel'. Is used to detect change in state.

//- initial menu level parameters
int MenuLevel = 0; //Defines the depth of the menu tree
int MenuID = 0; //Defines the unique identifier of each menu that resides on the same menu level
int channelUpLimit = 5; //Defines the upper limit of the button counter: 1. To limit cursor's downward row movement 2. To set the upper limit of value that is beeing edited.
int channelLowLimit = 0; //Defines the lower limit of the button counter: 1. To limit cursor's upward row movement 2. To set the lower limit of value that is beeing edited.

//- initial parameters for 'value editing mode'
int valueEditing = 0; //Flag to indicate if the interface is in 'value editing mode', thus disabling cursor movement.
int row = 0; //Temporary location to store the current cursor position whilst in 'value editing mode'.
int constrainEnc = 1; //Enable/disable constraining the button panel's counter to a lower and upper limit.
float valueEditingInc = 0; //Increments of each button press when using 'value editing mode'.
float TargetValue = 0; // Target value to be edited in 'value editing mode'
bool TargetValueBool = 0;
//Membrane panel button defines

int buttonUpState = 0; //status of "Up" button input
int buttonEnterState = 0; //status of "Enter" button input
int buttonDownState = 0; //status of "Down" button input

int prevBtnUp = 0; //previous state of "Up" button
int prevBtnEnt = 0; //previous state of "Enter" button
int prevBtnDown = 0; //previous state of "Down" button

int lastBtnUp = 0; //time since last "Up" pressed event
int lastBtnEnt = 0; //time since last "Enter" pressed event
int lastBtnDown = 0; //time since last "Down" pressed event

int enterPressed = 0; //status of "Enter" button after debounce filtering : 1 = pressed 0 = unpressed

int transEntInt = 250; //debounce treshold for "Enter" button
int transInt = 100; //debounce for other buttons
unsigned long lastAdminActionTime = 0; //keeps track of last button activity

int ButtonsAnalogValue = 0;        // value read from mebrane panel buttons.
int backlightIntensity = 5;        // LCD backlight intesity
int backlightIntensityDef = 5;     // Default LCD backlight intesity
unsigned long lastLCDredraw = 0;   // keeps track of last time the screen was redrawn

// Interrupt variables

volatile boolean conf = 1;       //conf!=0, return to main loop. End of configuration menus
volatile boolean refresh = 1;     //refresh the Visual() parameters display
int screen = 0;
unsigned long lastTempUpdate; //tracks clock time of last temp update
int wState = 0; // if value == 1 it means that we are watering 


//---------------------------------------------------------------------------------------------------------------------------------------------
// PIN DEFINITION
//---------------------------------------------------------------------------------------------------------------------------------------------

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A5;  // Analog input pin that the button panel is attached to
const int backlightPin = 26; // PWM output pin that the LED backlight is attached to
const int buttonEnterPin = 24;
const int buttonUpPin = 25;
const int buttonDownPin = 23;
const int D0 = 0;
const int D1 = 1;
const int D2 = 2;
const int D3 = 3;    
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;    //SHT1X CLOCK Blue   D6
const int D7 = 7;    //SHT1X DATA  Yellow D7
const int D8 = 8;
const int D9 = 9;
const int D10 = 10; //Relay control of the floor valve
const int D11 = 11;
const int D12 = 12;

//---------------------------------------------------------------------------------------------------------------------------------------------
//     DIGITAL CHANNELS
//---------------------------------------------------------------------------------------------------------------------------------------------

const int CH1 = 1; //Lights
const int CH2 = 2; //Water Pump
const int CH3 = 3; //Heat Radiator
const int CH4 = 4; //Cold Radiator
const int CH5 = 5; //Fog nebulizator
const int CH6 = 6; //Deshumidification
const int CH7 = 7; //Digital IN ( WATER LEVEL SENSOR )
const int CH8 = 8; 

//---------------------------------------------------------------------------------------------------------------------------------------------
//     ANALOG CHANNELS
//---------------------------------------------------------------------------------------------------------------------------------------------

const int ai_CH1 = 1;
const int ai_CH2 = 2;
const int ai_CH3 = 3;
const int ai_CH4 = 4;
const int ao_CH1 = 1;
const int ao_CH2 = 2;

//---------------------------------------------------------------------------------------------------------------------------------------------
// CLASS DEFINITIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

static UC1701 lcd;

//SENSORS

SHT1x soil(D7, D6); //Humidity sensor class


OneWire oneWire(D4); //D4 EXPANSION PORT

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature dallas(&oneWire);

//relays

Switcher Lights(CH1);
Switcher Pump(CH2);
Switcher Heat(CH3);
Switcher Cold(CH4);
Switcher Humidify(CH5);
Switcher FanIn(CH6);
Switcher FanLed(CH7);
Switcher Valve(CH8);
Switcher ValveFloor(D10);





//---------------------------------------------------------------------------------------------------------------------------------------------
// Plantbot config default parameters
//---------------------------------------------------------------------------------------------------------------------------------------------

float   hoursDays = 24;
float   hoursLight = 14;
float   hoursDark = 10;
float   tempDay = 28;
int     tempNight = 20;
int     hMax = 70;
int     hMin = 50;
float     deltaT = 0;
float     tempTolerance = 2;
int     hTolerance = 2;
int     hSoilTolerance = 2;
int     hsMax = 80;
int     hsMin = 40;
int sensors[3] = {0, 0, 0};

unsigned long checktime3s = 0;
unsigned long checktime10s = 0;
unsigned long checktime1 = 0;
unsigned long checktime5 = 0;
unsigned long humiditytime = 0;
float pwm = 0;
float unity = 20;

//---------------------------------------------------------------------------------------------------------------------------------------------
// Plantbot sensors parameters
//---------------------------------------------------------------------------------------------------------------------------------------------

int     hour = 0;
int     day  = 0;
float   temp = 0;
float   tempBox = 0; 
int   humSoil =  0;
float   humAir  = 0;
int   wLevel = 0;

//---------------------------------------------------------------------------------------------------------------------------------------------
// End of plantbot setup
//---------------------------------------------------------------------------------------------------------------------------------------------


void setup() {


//DIGITAL PIN SETUP DECLARATION
  
Indio.digitalMode(1,OUTPUT);
Indio.digitalWrite(1,LOW);
Indio.digitalMode(2,OUTPUT);
Indio.digitalWrite(2,LOW);
Indio.digitalMode(3,OUTPUT);
Indio.digitalWrite(3,LOW);
Indio.digitalMode(4,OUTPUT);
Indio.digitalWrite(4,LOW);
Indio.digitalMode(5,OUTPUT);
Indio.digitalWrite(5,LOW);
Indio.digitalMode(6,OUTPUT);
Indio.digitalWrite(6,LOW);
Indio.digitalMode(7,OUTPUT);
Indio.digitalWrite(7,LOW);
Indio.digitalMode(8,OUTPUT);
Indio.digitalWrite(8,LOW);

  pinMode(buttonEnterPin, INPUT);
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(backlightPin, OUTPUT); //set backlight pin to output
  analogWrite(backlightPin, (map(backlightIntensity, 5, 1, 255, 0))); //convert backlight intesity from a value of 0-5 to a value of 0-255 for PWM.
  pinMode(5, INPUT);
  pinMode(2, OUTPUT);
  pinMode(10, OUTPUT);
// ANALOG PIN SETUP DECLARATION
Indio.setADCResolution(12);
Indio.analogReadMode(ai_CH1, V10_raw); // Set Analog-In CH1 to % 5V mode (0-10V -> 0-100%).
Indio.analogReadMode(ai_CH2, V10_p);

 // Set Analog-In CH4 to % mA mode (4-20mA -> 0-100%)
Indio.analogWriteMode(1,V10);
Indio.analogWriteMode(2,V10);
Indio.analogWrite(1,5,true);
Indio.analogWrite(2,5,true);
  //LCD init
  lcd.begin();  //sets the resolution of the LCD screen

  for (int y = 0; y <= 7; y++) {
    for (int x = 0; x <= 128; x++) {
      lcd.setCursor(x, y);
      lcd.print(" ");
    }
  }

  //debug
   SerialUSB.begin(9600); //enables port for debugging messages
  
  //Menu init
  Welcome(); // Welcome message
  while ( conf == 1 ) {
    ReadButtons(); //check buttons
    Navigate(); //update menus and perform actions
  }
   dallas.begin();
  
          checktime1 = millis() + TENSEC;
          checktime5 = millis() + FIVEMIN;
}

void loop() {


//---------------------------------------------------------------------------------------------------------------------------------------------
// Every 3 seconds
//---------------------------------------------------------------------------------------------------------------------------------------------

  if((long)(millis() - checktime3s) >= 0) {


// Get sensor data //


temp = soil.readTemperatureC();        // SHTXX SENSOR Humidity and Temp air
humAir = soil.readHumidity();          // SHTXX SENSOR Humidity and Temp air
dallas.requestTemperatures();          // Send the command to get temperatures out of the cooling box
tempBox = dallas.getTempCByIndex(0);   // Send the command to get temperatures out of the cooling box

humSoil = Indio.analogRead(1); //Read Analog-In CH1 (output depending on selected mode)
humSoil = map(humSoil,1811,240,0,100);
wLevel = Indio.analogRead(2); //Read Analog-In CH1 (output depending on selected mode)
Visual();
deltaT = tempDay - temp;

   ////////////////////////////////// START TEMP CONTROL /////////////////////////////////////

    if ( sensors[0] == 1 && tempDay< 28) {  // ACTIVAMOS HEAT
      
      if (deltaT >= 2*tempTolerance)  
      {
        //PWM A TOPE, NO COMMUTA
        pwm=1;
        Heat.Start();
        FanIn.Start();
      }
      else if (deltaT >= tempTolerance && deltaT < 2*tempTolerance)
      {
        pwm = 0.7;
        Heat.Pwm(unity*pwm, unity*(1 - pwm) );
        FanIn.Start();

      }
      else if (deltaT < tempTolerance && deltaT >= 0.3 )
      {
        //PWM a 50%
        pwm = 0.5;
        Heat.Pwm(unity*pwm, unity*(1-pwm) );
        FanIn.Start();
      }
      else {
        Heat.Stop();
        FanIn.Stop();

      }
      
    }
    
   else if ( sensors[0] == 1 && tempDay >= 28) {  // ACTIVAMOS HEAT sin PWM, a tope
      
        Heat.Start();
        FanIn.Start();
     
      
      if ( temp >= 40) {
        Heat.Stop();

      } 
    }
    
   else if ( sensors[0] == 2 ) {  //ACTIVAMOS COLD
    Cold.Start();
    FanIn.Start();
     
    }
   else {
    Cold.Stop();
    Heat.Stop();
    if (sensors[2] != 1){
    FanIn.Stop();
   }
   }

   ////////////////////////////////// END TEMP CONTROL /////////////////////////////////////

   ////////////////////////////////// START HUMIDITY CONTROL /////////////////////////////////////
   

  if ( sensors[2] == 1 ) {
    FanIn.Start();
  }
  else if ( sensors[2] == 2 ) {
    FanIn.Stop();
    if ( wLevel == 49 ){
     Humidify.Pwm(5,10);//Usamós la función Pwm de la clase Switcher para alternar 5s ON y 10 OFF para controlar la subida de humedad
    }
  }
  else {
    Humidify.Stop();
    if ( sensors[0] == 0 ) {
    FanIn.Stop();
  }
  }

if ( wLevel == 49 ) {
  Valve.Stop();
}
   ////////////////////////////////// END HUMIDITY CONTROL /////////////////////////////////////


/*

    if (humAir - hMax > hTolerance) {  // Humedad Aire superior a la humedad máxima + tolerancia = deshumidificar
      sensors[2] = 1; //Hay que Deshumidificar
    }
    else {
      sensors[2] = 0;
    }

    if (hsMin > humSoil + hSoilTolerance ) {
      sensors[1] = 2;  // Debemos Regar
    }
    else if (hMin - humAir > hTolerance) 
      sensors[1] = 1; // Hay que nebulizar
    else {
      sensors[1] = 0;
    }

    if ( deltaT >= 0 ) {
      if ( tempDay > temp + tempTolerance < 0 ) {
        sensors[0] = 1; //activamos HEAT
      }
      else if ( deltaT < 0 ) {
        sensors[0] = 2; //activamos COLD
        if ( sensors [1] == 1 ) {
          sensors [1] = 0; No nebulizamos
        }
      }
      else {
        sensors[0] = 0;
      }
    }
*/
 ////////////////////////////////// START SOIL HUMIDITY CONTROL ////////////////////////////////////
if ( sensors[1] == 1 ) {
  Fill();   // we call Fill()function to verify the water lever of the tank and refill if necessary.
  if ( wLevel == 49 or wState == 1) {
  ValveFloor.Pwm2(1,20); // Commutates the state of pin D10 that controls floor watering, to make the changes slower.
  wState = 1;
  }
}

else  {
  digitalWrite(10, LOW); // Stops floor watering when it's enough humid
  wState = 0;
  
}
 
 ////////////////////////////////// END SOIL HUMIDITY CONTROL /////////////////////////////////////
    checktime3s = millis()+3*ONESEC;
    
 }


//---------------------------------------------------------------------------------------------------------------------------------------------
// Every 10 secs;            Here we define the 'sensors' Vector
//     sensors[0] Regulate thermal control;                             sensors[0]=0 no actuation;    sensors[0]=1 Heat;          sensors[0]=2  Cold;
//     sensors[1] Regulate the direct water to thre soil;               sensors[1]=0 no actuation;    sensors[1]=1 Water;  
//     sensors[2] Regulate the humidity;                                sensors[2]=0 no actuation;    sensors[2]=1 Deshumidify ;  sensors[2]=2 Humidify;
//---------------------------------------------------------------------------------------------------------------------------------------------

  if((long)(millis() - checktime1) >= 0) {

////////////////////////////////////////// Air Humidity ///////////////////////////////////////////////////
    
     if ( humAir - hMax >= hTolerance ) {  // Humedad Aire superior a la humedad máxima + tolerancia = deshumidificar
      sensors[2] = 1; //Hay que Deshumidificar
    }
    
    else if (( hMin - humAir >= hTolerance ) && sensors[0] == 0 ) { //Humedad mínima superior a la Humedad Aire + Tolerancia = nebulizar
      sensors[2] = 2; // Hay que nebulizar
    }
    else {
      sensors[2] = 0;
    }
////////////////////////////////////////// Air Humidity ///////////////////////////////////////////////////


////////////////////////////////////////// Soil Humidity //////////////////////////////////////////////////

    if (hsMin > humSoil - hSoilTolerance ) { // Cuando la humedad del suelo esté cerca del valor mínimo, tanto como indique la tolerancia, debemos regar
      if ( sensors[2] != 2 ){
      sensors[1] = 1;  // Debemos Regar
    }
     else if ( sensors[2] == 2){
      sensors[1] = 0;
    }
    }
 /*
    else if (hMin - humAir > hTolerance) //Humedad mínima superior a la Humedad Aire + Tolerancia = nebulizar
      sensors[1] = 1;
*/      
    else  {
      sensors[1] = 0;
    }

////////////////////////////////////////// Soil Humidity //////////////////////////////////////////////////


////////////////////////////////////////// Temperature  ///////////////////////////////////////////////////

    if ( temp + tempTolerance  <= tempDay  ) { // Activamos HEAT cuando temp esté 'tantos grados como la tolerancia' por debajo de la consigna
        sensors[0] = 1; //activamos HEAT
      }
   
    else if ( tempDay + tempTolerance <= temp  ) {// Activamos COLD cuando temp esté 'tantos grados como la tolerancia' por encima de la consigna
         sensors[0] = 2; //activamos COLD
        if ( sensors [2] == 2 ) {
          sensors [2] = 0;  // No nebulizamos   
        }
      }
   else if ( abs(temp-tempDay) <= 0.3 ) {
        sensors[0] = 0; // Cuando las temperaturas coinciden, es hora de Resetear sensors[0] a estado parado
      }
    

    /*
    if ( abs(tempNight - temp) > tempTolerance && lights.st() == LOW) {
      if ( tempNight > temp - tempTolerance ) {
        sensors[0] = 1; //activamos HEAT
      }
      else if ( tempNight < temp + tempTolerance ) {
        sensors[0] = 2; //activamos COLD
      }
      else {
        sensors[0] = 0;
      }
    }
*/
////////////////////////////////////////// Temperature  ///////////////////////////////////////////////////



////////////////////////////////////////// Water Level   //////////////////////////////////////////////////

    if ( sensors[2] == 2 && wLevel == 0 ) {

      Fill(); 
    }
    checktime1 += TENSEC;
    
 }

///////////////////////////////////////// Water Level   //////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------------------------------
// Every 5 minute
//---------------------------------------------------------------------------------------------------------------------------------------------

  if((long)(millis() - checktime5) >= 0) { // Stuff we check every 5 minutes
    
    
   
////////////////////////////////////////// Lights  ///////////////////////////////////////////////////

  Lights.Update(hoursLight,hoursDark);
  FanLed.Update(hoursLight,hoursDark);
    
////////////////////////////////////////// Lights  ///////////////////////////////////////////////////

   
    checktime5 += FIVEMIN;
    
    }

//---------------------------------------------------------------------------------------------------------------------------------------------
// Interruptors rutines
//---------------------------------------------------------------------------------------------------------------------------------------------


  if (conf == 1 ) {   // Show config menu if ENTER is pressed
    MainMenu();
    while ( conf == 1 ) {
      ReadButtons(); //check buttons
      Navigate(); //update menus and perform actions
    }
  }
  if (refresh == 1) {   // Refresh screen values if DOWN is pressed

  switch (screen) {
    case 0:
      Visual();
      screen = 1;
      break;
    case 1:
      Visual1();
      screen = 0;
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
  refresh = 0;
 }
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS DECLARATIONS
//---------------------------------------------------------------------------------------------------------------------------------------------



void Visual() { //this function draws the real parameters of sensors and actuators



  lcd.clear();
  
  // First Row
  
  lcd.setCursor(2, 0);
  lcd.print("Temperatura");
  lcd.setCursor(96, 0);
  lcd.print(temp, 1);
  lcd.setCursor(120, 0);
  lcd.print("C");
  
  // Second Row
  
  lcd.setCursor(2, 1);
  lcd.print("Humedad T");
  lcd.setCursor(85, 1);
  lcd.print(humSoil, 1);
  lcd.setCursor(120, 1);
  

  
  lcd.setCursor(2, 2);
  lcd.print("Humedad amb");
  lcd.setCursor(96, 2);
  lcd.print(humAir, 1);
  lcd.setCursor(120, 2);
  lcd.print("%");

  
  lcd.setCursor(2, 3);
  lcd.print("Dia");
  lcd.setCursor(25, 3);
  lcd.print(day);

  
  lcd.setCursor(2, 4);
  lcd.print("TemB");
  lcd.setCursor(96, 4);
  lcd.print(tempBox);

  
  lcd.setCursor(2, 5);
  lcd.print("Level");
  lcd.setCursor(80, 5);
  lcd.print(wLevel);

  lcd.setCursor(2, 6);
  lcd.print("Sensors");
  lcd.setCursor(70,6);
  lcd.print("(");  
  lcd.setCursor(80,6);
  lcd.print(sensors[0]);
  lcd.setCursor(90,6);
  lcd.print(sensors[1]);
  lcd.setCursor(100,6);
  lcd.print(sensors[2]);
  lcd.setCursor(110,6);
  lcd.print(")");  
  
//


}

void Visual1() { //this function draws the real parameters of sensors and actuators
  lcd.clear();
  
  lcd.setCursor(2, 0);
  lcd.print("Luces");
  lcd.setCursor(96, 0);
  //lcd.print(Lights.st, 1);

  lcd.setCursor(2, 1);
  lcd.print("Bomba");
  lcd.setCursor(96, 1);
  //lcd.print(Pump.st, 1);

  lcd.setCursor(2, 2);
  lcd.print("Calor");
  lcd.setCursor(96, 2);
//  lcd.print(Heat.st, 1);

  lcd.setCursor(2, 3);
  lcd.print("Frio");
  lcd.setCursor(96, 3);
  //lcd.print(Cold.st,1);
  
  lcd.setCursor(2, 4);
  lcd.print("Deshum");
  lcd.setCursor(96, 4);
  //lcd.print(Deshum.st,1);

  lcd.setCursor(2, 5);
  lcd.print("Nebulizador");
  lcd.setCursor(96, 5);
  //lcd.print(Deshum.st,1);
  
}


void Welcome() { //Mensaje de bienvenida

  lcd.clear(); //clear the screen
  //actual user content on the screen
  lcd.setCursor(5, 1); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Welcome to"); //print text on screen
  lcd.setCursor(5, 2); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Plantduino!"); //print text on screen
  delay(2000);
  lcd.clear(); //clear the screen
  //actual user content on the screen
  lcd.setCursor(5, 1); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Vamos a configurar"); //print text on screen
  lcd.setCursor(5, 2); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("esta movida"); //print text on screen
  delay(2000);
  MainMenu();
}

void Vamos() { //Mensaje de bienvenida

  lcd.clear(); //clear the screen
  //actual user content on the screen
  lcd.setCursor(5, 1); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Plantbot riega"); //print text on screen
  lcd.setCursor(5, 4); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Plantbot cuida"); //print text on screen

}



// Menu principal

void MainMenu() { //this function draws the first menu - splash screen
  //menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 2; //upper row limit
  channelLowLimit = 0; //lower row limit
  MenuLevel = 0; //menu tree depth -> first level
  MenuID = 0; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Configuracion");
  lcd.setCursor(6, 1);
  lcd.print("Start!");
  lcd.setCursor(6, 2);
  lcd.print("Test Manual");
}
// Visualización de parametros



// Menu configuracion

void MenuConfig() {
  channel = 0;
  channelUpLimit = 5;
  channelLowLimit = 0;
  MenuID = 1;
  MenuLevel = 1;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Luz");
  lcd.setCursor(6, 1);
  lcd.print("Temperatura");
  lcd.setCursor(6, 2);
  lcd.print("Humedad ambiental");
  lcd.setCursor(6, 3);
  lcd.print("Humedad sustrato");
  lcd.setCursor(6, 4);
  lcd.print("LCD backlight");
  lcd.setCursor(6, 5);
  lcd.print("Salir!");
}

void MenuLights() {
  channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 16;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Duracion dia");
  lcd.setCursor(89, 0);
  lcd.print(hoursDays, 1);
  //  lcd.setCursor(114, 0);
  //  lcd.print("H");
  lcd.setCursor(6, 1);
  lcd.print("Luz:");
  lcd.setCursor(89, 1);
  lcd.print(hoursLight, 1);
  //  lcd.setCursor(114, 1);
  //  lcd.print("h");
  lcd.setCursor(6, 2);
  lcd.print("Atras!");
  lcd.setCursor(6, 4);
  lcd.print("Oscuridad:");
  hoursDark = hoursDays - hoursLight;
  lcd.setCursor(89, 4);
  lcd.print(hoursDark, 1);


}

void MenuTemp() {
  channel = 0;
  channelUpLimit = 3;
  channelLowLimit = 0;
  MenuID = 17;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Diurna");
  lcd.setCursor(89, 0);
  lcd.print(tempDay, 1);
  lcd.setCursor(114, 0);
  lcd.print("C");
  lcd.setCursor(6, 1);
  lcd.print("Nocturna");
  lcd.setCursor(89, 1);
  lcd.print(tempNight, 1);
  lcd.setCursor(114, 1);
  lcd.print("C");

  lcd.setCursor(6,2);
  lcd.print("tempTolerance");
  lcd.setCursor(89,2);
  lcd.print(tempTolerance,1);
  lcd.setCursor(114,2);
  lcd.print("C");
  lcd.setCursor(6, 3);
  lcd.print("Atras!");
}

void MenuHumidity() {
  channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 15;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Humedad Max");
  lcd.setCursor(89, 0);
  lcd.print(hMax, 1);
  lcd.setCursor(114, 0);
  lcd.print("%");
  lcd.setCursor(6, 1);
  lcd.print("Humedad Min");
  lcd.setCursor(89, 1);
  lcd.print(hMin, 1);
  lcd.setCursor(114, 1);
  lcd.print("%");
  lcd.setCursor(6, 2);
  lcd.print("Atras!");
}

void MenuSoil() {
  channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 31;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Humedad Max");
  lcd.setCursor(89, 0);
  lcd.print(hsMax, 1);
  lcd.setCursor(114, 0);
  lcd.print("%");
  lcd.setCursor(6, 1);
  lcd.print("Humedad Min");
  lcd.setCursor(89, 1);
  lcd.print(hsMin, 1);
  lcd.setCursor(114, 1);
  lcd.print("%");
  lcd.setCursor(6, 2);
  lcd.print("Atras!");
}


void MenuSetup() { //submenu of Main menu - setup screen for Industruino
  channel = 0;
  channelUpLimit = 1;
  channelLowLimit = 0;
  MenuID = 9;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("BackLight     ");
  lcd.setCursor(65, 0);
  lcd.print(backlightIntensity, 1);
  lcd.setCursor(6, 1);
  lcd.print("Back");
}

void MenuTest() {
  channel = 0;
  channelUpLimit = 6;
  channelLowLimit = 0;
  MenuID = 19;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();

  lcd.setCursor(6, 0);
  lcd.print("Luces");
  lcd.setCursor(96, 0);
  lcd.print(Indio.digitalRead(1));

  lcd.setCursor(6, 1);
  lcd.print("Bomba");
  lcd.setCursor(96, 1);
  lcd.print(Indio.digitalRead(2));

  lcd.setCursor(6, 2);
  lcd.print("Calor");
  lcd.setCursor(96, 2);
  lcd.print(Indio.digitalRead(3));

  lcd.setCursor(6, 3);
  lcd.print("Frio");
  lcd.setCursor(96, 3);
  lcd.print(Indio.digitalRead(4));

  lcd.setCursor(6, 4);
  lcd.print("Nebulitz.");
  lcd.setCursor(96, 4);
  lcd.print(Indio.digitalRead(5));

  lcd.setCursor(6, 5);
  lcd.print("Fan In");
  lcd.setCursor(96, 5);
  lcd.print(Indio.digitalRead(6));
  
  lcd.setCursor(6,6);
  lcd.print("Next");
  
}
void MenuTest1() {
// Next //

  channel = 0;
  channelUpLimit = 4;
  channelLowLimit = 0;
  MenuID = 21;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  
  lcd.setCursor(6, 0);
  lcd.print("Fan Led");
  lcd.setCursor(96, 0);
  lcd.print(Indio.digitalRead(7));
  
  lcd.setCursor(6, 1);
  lcd.print("Valve");
  lcd.setCursor(96, 1);
  lcd.print(Indio.digitalRead(8));
  
  lcd.setCursor(6, 2);
  lcd.print("Valve Floor");
  lcd.setCursor(96, 2);
  lcd.print(digitalRead(10));

  lcd.setCursor(6, 3);
  lcd.print("Atras!");

  lcd.setCursor(6, 4);
  lcd.print("EXIT");
}




//---------------------------------------------------------------------------------------------------------------------------------------------
// Lectura de botones
//---------------------------------------------------------------------------------------------------------------------------------------------

void ReadButtons() {

  buttonEnterState = digitalRead(buttonEnterPin);
  buttonUpState = digitalRead(buttonUpPin);
  buttonDownState = digitalRead(buttonDownPin);

  if (buttonEnterState == HIGH && prevBtnEnt == LOW)
  {
    if ((millis() - lastBtnEnt) > transEntInt)
    {
      enterPressed = 1;
    }
    lastBtnEnt = millis();
    lastAdminActionTime = millis();
    //SerialUSB.println(EnterPressed);
  }
  prevBtnEnt = buttonEnterState;


  if (buttonUpState == HIGH && prevBtnUp == LOW)
  {
    if ((millis() - lastBtnUp) > transInt)
    {
      channel--;
    }
    lastBtnUp = millis();
    lastAdminActionTime = millis();
    //SerialUSB.println("UpPressed");
  }
  prevBtnUp = buttonUpState;


  if (buttonDownState == HIGH && prevBtnDown == LOW)
  {
    if ((millis() - lastBtnDown) > transInt)
    {
      channel++;
    }
    lastBtnDown = millis();
    lastAdminActionTime = millis();
    //SerialUSB.println("DownPressed");
  }
  prevBtnDown = buttonDownState;

  if (constrainEnc == 1) {
    channel = constrain(channel, channelLowLimit, channelUpLimit);
  }

}

//---------------------------------------------------------------------------------------------------------------------------------------------
// Edit values
//---------------------------------------------------------------------------------------------------------------------------------------------

float EditFloatValue(int n, int m, float r) //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditingInc = r; //increment for each button press
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1) { //stays in 'value editing mode' until enter is pressed
    ReadButtons(); //check the buttons for any change
    lcd.setCursor(0, row);
    lcd.print("*");
    if (channel != lastChannel) { //when up or down button is pressed
      if (channel < lastChannel && TargetValue <= m) { //if 'Up' button is pressed, and is within constraint range.
        TargetValue += valueEditingInc; //increment target variable with pre-defined increment value
      }
      if (channel > lastChannel && TargetValue > n) { //if 'Down' button is pressed, and is within constraint range.
        TargetValue -= valueEditingInc ; //decrement target variable with pre-defined increment value
      }

      //print updated value
      lcd.setCursor(89, row);
      SerialUSB.println(TargetValue);
      lcd.print(TargetValue, 1);
      lastChannel = channel;
    }
    //delay(50);
  }
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 1; //enable constrainment of button counter's range so to stay within the menu's range
  channelUpLimit = 3; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValue; //return the edited value to the main 'Navigate' UI control function for further processing
}
bool EditBooleanValue() //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1) { //stays in 'value editing mode' until enter is pressed
    ReadButtons(); //check the buttons for any change
    lcd.setCursor(0, row);
    lcd.print("*");
    if (channel != lastChannel) { //when up or down button is pressed
       if(TargetValueBool == true) {
         TargetValueBool = false;
       }
       else {
         TargetValueBool = true;
       }
      //print updated value
      lcd.setCursor(96, row);
      lcd.print(TargetValueBool);
      lastChannel = channel;
    }
    //delay(50);
  }
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 1; //enable constrainment of button counter's range so to stay within the menu's range
//  channelUpLimit = 3; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValueBool; //return the edited value to the main 'Navigate' UI control function for further processing
}
//---------------------------------------------------------------------------------------------------------------------------------------------
// Navegador de los menús
//---------------------------------------------------------------------------------------------------------------------------------------------


void Navigate()
{

  if (valueEditing != 1) {
    if (MenuLevel == 0) //check if current activated menu is the 'splash screen' (first level)
    {
      {
        if (channel == 0 && enterPressed == 1) MenuConfig();
        if (channel == 1 && enterPressed == 1) {
          Vamos();
          attachInterrupt(24, enter, FALLING);
          attachInterrupt(23, down, FALLING);
          conf = 0;
          refresh = 1;
          delay(500);
        }
        if (channel == 2 && enterPressed == 1) {
          MenuTest();
        }       
      }
    }
    if (MenuLevel == 1) { //check if current activated menu is the 'Main menu' (first level)

      if (channel == 0 && enterPressed == 1) MenuLights();
      if (channel == 1 && enterPressed == 1) MenuTemp();
      if (channel == 2 && enterPressed == 1) MenuHumidity();
      if (channel == 3 && enterPressed == 1) MenuSoil();
      if (channel == 4 && enterPressed == 1) MenuSetup();
      if (channel == 5 && enterPressed == 1) MainMenu();
    }
    if (MenuLevel == 2) {
      if (MenuID == 9) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = backlightIntensity; //copy variable to be edited to 'Target value'
          backlightIntensity = EditFloatValue(0, 4, 1);
          analogWrite(backlightPin, (map(backlightIntensity, 5, 0, 255, 0)));
        }
        if (channel == 1 && enterPressed == 1) MenuConfig();
      }
      if (MenuID == 17) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = tempDay; //copy variable to be edited to 'Target value'
          tempDay = EditFloatValue(5, 49, 1);
        }
        if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = tempNight; //copy variable to be edited to 'Target value'
          tempNight = EditFloatValue(5, 34, 1);
        }
        if (channel == 2 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = tempTolerance; //copy variable to be edited to 'Target value'
          tempTolerance = EditFloatValue(0.5, 5, 0.5);
        }
        if (channel == 3 && enterPressed == 1) {
          MenuConfig();
        }
      }
        if (MenuID == 19) {
          if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
          if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
          if (channel == 2 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
          if (channel == 3 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
          if (channel == 4 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
          if (channel == 5 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+1,TargetValueBool);
          }
            if (channel == 6 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            MenuTest1();
          }
        }
      if (MenuID == 21) {

          if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+7,TargetValueBool);
          }
        
          if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            Indio.digitalWrite(channel+7,TargetValueBool);
          }
          if (channel == 2 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
          {
            TargetValueBool = EditBooleanValue();
            digitalWrite(10,TargetValueBool);
          }        
          if (channel == 3 && enterPressed == 1)  MenuTest();
          
          if (channel == 4 && enterPressed == 1)  {
            Stop();
            MainMenu();
          }           
      }
      if (MenuID == 16) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hoursDays; //copy variable to be edited to 'Target value'
          hoursDays = EditFloatValue(1, 50, 0.5);
        }
        if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hoursLight; //copy variable to be edited to 'Target value'
          hoursLight = EditFloatValue(1, hoursDays - 1, 1);
        }
        if (channel == 2 && enterPressed == 1) {
          MenuConfig();
        }
      }
      if (MenuID == 15) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hMax; //copy variable to be edited to 'Target value'
          hMax = EditFloatValue(15, 94, 1);
        }
        if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hMin; //copy variable to be edited to 'Target value'
          hMin = EditFloatValue(10, 84, 1);
        }
        if (channel == 2 && enterPressed == 1) {
          MenuConfig();
        }
      }
      if (MenuID == 31) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hMax; //copy variable to be edited to 'Target value'
          hsMax = EditFloatValue(15, 94, 1);
        }
        if (channel == 1 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = hMin; //copy variable to be edited to 'Target value'
          hsMin = EditFloatValue(10, 84, 1);
        }
        if (channel == 2 && enterPressed == 1) {
          MenuConfig();
        }
      }
    }
    if (MenuLevel == 3)
    {


    }
    if (channel != lastChannel && valueEditing != 1) { //updates the cursor position if button counter changed and 'value editing mode' is not running
      ScrollCursor();
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------
// UI core functions
//---------------------------------------------------------------------------------------------------------------------------------------------



void ScrollCursor() //makes the cursor move
{
  lastChannel = channel; //keep track button counter changes
  for (int i = 0; i <= 6; i++) { //clear the whole column when redrawing a new cursor
    lcd.setCursor(coll, i);
    lcd.print(" ");
  }
  lcd.setCursor(coll, channel); //set new cursor position
  lcd.print(">"); //draw cursor

}
void enter() {
  conf = 1;
  detachInterrupt(24);
  detachInterrupt(23);
}
void down() {
  refresh = 1;
}
/*void up() {
  screen = 2;
}*/
/*
bool updateTemperature() {
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
    temp = dallas.getTempFByIndex(0); //get temp reading
    lastTempUpdate = millis();
    dallas.requestTemperatures(); //request reading for next time
    return true;
  }
  return false;
}//void updateTemperature
*/


//---------------------------------------------------------------------------------------------------------------------------------------------
// Usefull functions
//---------------------------------------------------------------------------------------------------------------------------------------------

void Fill(){  // this function refills the water tank inside the chamber to humidify or water the plants
  
  if (wLevel < 49){
  Valve.Start();
   }
   
  else {
  Valve.Stop(); 
  }
}

void Stop() {
Indio.digitalWrite(1,LOW);
Indio.digitalWrite(2,LOW);
Indio.digitalWrite(3,LOW);
Indio.digitalWrite(4,LOW);
Indio.digitalWrite(5,LOW);
Indio.digitalWrite(6,LOW);
Indio.digitalWrite(7,LOW);
Indio.digitalWrite(8,LOW);
digitalWrite(10,LOW);
}




