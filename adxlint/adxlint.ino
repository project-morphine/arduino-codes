#include <SparkFun_ADXL345.h>
#include <avr/wdt.h>

#include <Wire.h>
#define ledPin 13
#define soundPin 8

/* Assign a unique ID to this sensor at the same time */
ADXL345 adxl = ADXL345();
unsigned volatile long timeout = 0;

void showSetupCompleted() {
   digitalWrite(ledPin,HIGH);
  digitalWrite(soundPin,HIGH);
  delay(200); //200
  digitalWrite(ledPin,LOW); 
  digitalWrite(soundPin,LOW);
  delay(1000);
}

void alarmOn() {
  digitalWrite(ledPin,HIGH);
  digitalWrite(soundPin,HIGH);
  delay(200); //200
  digitalWrite(ledPin,LOW); 
  digitalWrite(soundPin,LOW);
  delay(200);
}

void setup() {
  Serial.begin(9600);
  setupADXL();
  //Setup builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(soundPin, OUTPUT);
  showSetupCompleted();
}

void setupADXL() {
  adxl.powerOn();
  adxl.setRangeSetting(4);
  //adxl.setRate(10);
  adxl.setActivityXYZ(1, 1, 1);
  adxl.setActivityThreshold(75);
  adxl.setInactivityXYZ(1, 0, 0);
  adxl.setInactivityThreshold(75);
  adxl.setTimeInactivity(6);
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
  wdt_enable(WDTO_2S);
}

void loop() {
  int x,y,z;   
  wdt_reset();
  ADXL_ISR();
//  Serial.print(x);
//  Serial.print(", ");
//  Serial.print(y);
//  Serial.print(", ");
//  Serial.println(z); 
delay(100);
}

void ADXL_ISR() {
  
  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  byte interrupts = adxl.getInterruptSource();
  
  // Free Fall Detection
  if(adxl.triggered(interrupts, ADXL345_FREE_FALL)){
    //Serial.println("*** FREE FALL ***");
    //add code here to do when free fall is sensed
    alarmOn();
  }
}
