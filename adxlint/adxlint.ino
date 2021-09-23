#include <SparkFun_ADXL345.h>

#include <Wire.h>


/* Assign a unique ID to this sensor at the same time */
ADXL345 adxl = ADXL345();
const float g = 9.81;
const float LFT = 0.5;
const float UFT = 2.8;
const float ST = 1.25;

bool fallFlag = false;
bool stepFlag = false;
bool alarmTrig = false;

unsigned long fallTimer = 0;
unsigned long alarmTimer = 0;
unsigned long stepTimer = 0;
unsigned long fsrTimer = 0;
unsigned long printTimer = 0;

const long fallInterval = 500;
const long alarmInterval = 125;
const long stepInterval = 100;
const long fsrInterval = 40;
const long printInterval = 200;

const int ledPin =  LED_BUILTIN;
int ledState = LOW;

unsigned int stepCount = 0;
int fsrprev = 0;

void alarmOn() {
//  if (millis() - alarmTimer >= alarmInterval) {
//    // save the last time you blinked the LED
//    alarmTimer = millis();
//
//    // if the LED is off turn it on and vice-versa:
//    if (ledState == LOW) {
//      ledState = HIGH;
//    } else {
//      ledState = LOW;
//    }
//
//    // set the LED with the ledState of the variable:
//    digitalWrite(ledPin, ledState);
// }
  digitalWrite(ledPin,HIGH);
  delay(200);
  digitalWrite(ledPin,LOW);
  
}

void printToSerial(double anet, int stepCount, bool alarmTrig) {
  if (millis() - printTimer >= printInterval) {
    printTimer = millis();
  
    /* Display the results (acceleration is measured in g) */
    Serial.print("Net: "); Serial.print(anet); Serial.println(" g");
  
    // Print number of steps
    Serial.print("Steps: "); Serial.println(stepCount);

    if (alarmTrig) {
      Serial.println("Fall detected!");
    }

    Serial.println(" ");
  }
}

void setup() {
  Serial.begin(9600);
  adxl.powerOn();
  adxl.setRangeSetting(4);
  adxl.setRate(50);
  adxl.setActivityXYZ(1, 1, 1);
  adxl.setActivityThreshold(75);
  adxl.setInactivityXYZ(1, 0, 0);
  adxl.setInactivityThreshold(75);
  adxl.setTimeInactivity(10);
  adxl.setTapDetectionOnXYZ(1, 1, 1);
  adxl.setTapThreshold(50);
  adxl.setTapDuration(15);
  adxl.setDoubleTapLatency(80);
  adxl.setDoubleTapWindow(200);
  adxl.setFreeFallThreshold(5);
  adxl.setFreeFallDuration(15);
  adxl.InactivityINT(1);
  adxl.ActivityINT(1);
  adxl.FreeFallINT(1);
  adxl.doubleTapINT(1);
  adxl.singleTapINT(1);
  //Setup builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  int x,y,z;   
  adxl.readAccel(&x, &y, &z); 
  ADXL_ISR();
  //if (adxl.error_code == ADXL345_READ_ERROR) alarmOn();
//  Serial.print(x);
//  Serial.print(", ");
//  Serial.print(y);
//  Serial.print(", ");
//  Serial.println(z); 
}

void ADXL_ISR() {
  
  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  byte interrupts = adxl.getInterruptSource();
  //if(adxl.status == ADXL345_READ_ERROR) ADXL_ISR();
  
  // Free Fall Detection
  if(adxl.triggered(interrupts, ADXL345_FREE_FALL)){
    //Serial.println("*** FREE FALL ***");
    //add code here to do when free fall is sensed
    alarmOn();
  } else if(adxl.triggered(interrupts, ADXL345_ACTIVITY)){
    //Serial.println("*** ACTIVITY ***"); 
     //add code here to do when activity is sensed
  } else {
    Serial.println("Nothing");
  }
  
}
