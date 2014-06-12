/*
 MPL3115A2 Barometric Pressure Sensor Library Example Code
 By: Nathan Seidle
 SparkFun Electronics
 Date: September 24th, 2013
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Uses the MPL3115A2 library to display the current altitude and temperature
 
 Hardware Connections (Breakoutboard to Arduino):
 -VCC = 3.3V
 -SDA = A4 (use inline 10k resistor if your board is 5V)
 -SCL = A5 (use inline 10k resistor if your board is 5V)
 -INT pins can be left unconnected for this demo
 
 During testing, GPS with 9 satellites reported 5393ft, sensor reported 5360ft (delta of 33ft). Very close!
 During testing, GPS with 8 satellites reported 1031ft, sensor reported 1021ft (delta of 10ft).
*/

#include <Wire.h>
#include "MPL3115A2.h"
#include "SevSeg.h"
#include <Bounce2.h>

//#define SERIAL_DEBUG //Used for activating Serial Debugging

#define TITLE_MODE 1
#define VALUE_MODE 0

#define ALT 0
#define TOP 1
#define BOT 2
#define DIFF 3
#define STBY 4


//Create an instance of the object
MPL3115A2 myPressure;
SevSeg myDisplay;

unsigned long timer;
unsigned long titletimer;
const int buttonPin = 3;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

Bounce debouncer = Bounce(); 

char tempString[10]; //Used for sprintf

float maxaltitude = -99999;
float minaltitude = 99999;
boolean lastbutton = HIGH;
boolean currentbutton = HIGH;
int mode;
boolean metamode = TITLE_MODE;
char *s;
  
const int titletimerBeat = 1000; //fiddle away!
const int timerBeat = 500; //shouldn't be changed much


void setup()
{
  pinMode(buttonPin, INPUT_PULLUP);  
  debouncer.attach(buttonPin);
  debouncer.interval(5);
  Wire.begin();        // Join i2c bus
  Serial.begin(9600);  // Start serial for output

  myPressure.begin(); // Get sensor online
  
  int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode

    //This pinout is for a bubble dispaly
       //Declare what pins are connected to the GND pins (cathodes)
       int digit1 = A1; //Pin 1
       int digit2 = 6; //Pin 10
       int digit3 = 12; //Pin 4
       int digit4 = 10; //Pin 6
       
       //Declare what pins are connected to the segments (anodes)
       int segA = 4; //Pin 12
       int segB = 5; //Pin 11
       int segC = 13; //Pin 3
       int segD = 8; //Pin 8
       int segE = A0; //Pin 2
       int segF = 7; //Pin 9
       int segG = 9; //Pin 7
       int segDP= 11; //Pin 5
   
  int numberOfDigits = 4; //Do you have a 1, 2 or 4 digit display?

  myDisplay.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  
  myDisplay.SetBrightness(100); //Set the display to 100% brightness level

  timer = millis();
  
  titletimer = timer+titletimerBeat;

  //Configure the sensor
  myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  //myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa

  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 
}

void loop() {
    
  float altitude;
  float pressure;
  
  debouncer.update();
  int buttonState = debouncer.read();

//  buttonState = digitalRead(buttonPin);
  
  lastbutton = currentbutton;
  currentbutton = buttonState;
  
  if (lastbutton == HIGH && currentbutton == LOW) {
    if (mode >= STBY) mode = 0;
    else
      mode++;
    metamode = TITLE_MODE;
    titletimer = millis()+titletimerBeat;
  }
  
  if (millis() > titletimer) {
    titletimer = millis()+titletimerBeat;
    metamode = VALUE_MODE;
  }

  if (millis() > timer) {
    timer = millis()+timerBeat;
       
	//things to do every 500 milliseconds
    altitude = myPressure.readAltitudeFt();
    #ifdef SERIAL_DEBUG
    Serial.print(" Altitude(ft):");
    Serial.print(altitude, 2);
    if (buttonState == HIGH) Serial.print("BUTTON PRESSED!");
    #endif
    
  if (altitude > maxaltitude) {
    maxaltitude = altitude;
  }
  
  if (altitude < minaltitude) {
    minaltitude = altitude;
  }

    if (metamode == VALUE_MODE) {
      switch (mode) {
      case ALT:
        sprintf(tempString, "%4d", (int)altitude); //Convert altitude into a string that is right adjusted
        break;
      case TOP:
        sprintf(tempString, "%4d", (int)maxaltitude);
        break;
      case BOT:
        sprintf(tempString, "%4d", (int)minaltitude);
        break;
      case DIFF:
        sprintf(tempString, "%4d", (int)(maxaltitude - minaltitude));
        break;
      case STBY:
        sprintf(tempString, "    ");
        break;
        }
      s = tempString;
    }
    else if (metamode == TITLE_MODE) {
      switch (mode) {
      case ALT:
        s = "Alt ";
        break;
      case TOP:
        s = "HiGH";
        break;
      case BOT:
        s = "Lo  ";
        break;
      case DIFF:
        s = "diFF";
        break;
      case STBY:
        s = "Stby";
      }
    }
    #ifdef SERIAL_DEBUG
    Serial.print(" OUR STR:");
    Serial.print(s);
    
    Serial.println();
    #endif
  }
  
    if(mode != STBY || metamode == TITLE_MODE) myDisplay.DisplayString(s, 0); //MUST STAY OUTSIDE OF "TIMER" IF STATEMENT!!!
}
