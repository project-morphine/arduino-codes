#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
const float g = 9.81;
const float LFT = 0.5;
const float UFT = 2.8;
const float ST = 1.25;
const int fsrT = 100;

bool fallFlag = false;
bool stepFlag = false;
bool fsrTrig = true;
bool accelTrig = false;

unsigned long fallTimer = 0;
unsigned long alarmTimer = 0;
unsigned long stepTimer = 0;
unsigned long printTimer = 0;

const long fallInterval = 500;
const long alarmInterval = 125;
const long stepInterval = 100;
const long printInterval = 200;

const int ledPin =  LED_BUILTIN;
int ledState = LOW;

sensors_event_t event;
double ax, ay, az, anet;
int fsrval;

unsigned int stepCount = 0;

void alarmOn() {
  unsigned long timeNow = millis();
  
  if (timeNow - alarmTimer >= alarmInterval) {
    // save the last time you blinked the LED
    alarmTimer = timeNow;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

void printToSerial(double anet, int fsrval, int stepCount, bool accelTrig, bool fsrTrig) {
  unsigned long timeNow = millis();

  if (timeNow - printTimer >= printInterval) {
    printTimer = timeNow;
  
    /* Display the results (acceleration is measured in g) */
    Serial.print("Net: "); Serial.print(anet); Serial.println(" g");

    // Print pressure sensor reading
    Serial.print("FSR: "); Serial.println(fsrval);
  
    // Print number of steps
    Serial.print("Steps: "); Serial.println(stepCount);

    if (accelTrig && fsrTrig) {
      Serial.println("Fall detected!");
    }

    Serial.println(" ");
  }
}

void setup() {
  Serial.begin(9600);
  accel.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  /* Set the range and data rate to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_4_G);
  accel.setDataRate(ADXL345_DATARATE_50_HZ);
}

void loop() {
  // Obtaining acceleration values
  accel.getEvent(&event);
  ax = event.acceleration.x/g;
  ay = event.acceleration.y/g;
  az = event.acceleration.z/g;
  anet = sqrt(ax*ax + ay*ay + az*az);

  // Obtaining pressure sensor value
  fsrval = analogRead(A0);


  // Step Counting
  if (anet > ST && stepFlag == false) {
    stepCount += 1;
    stepFlag = true;
    stepTimer = millis();
    
  } else if (stepFlag == true) {
    unsigned long timeNow = millis();

    // Minimum delay for registering a new step
    if (anet <= ST && timeNow - stepTimer >= stepInterval) {
      stepFlag = false;
    }
  }

  // Fall Detection - UFT
  if (fallFlag == true) {
    unsigned long timeNow = millis();
    
    if (timeNow - fallTimer >= fallInterval) {
      fallFlag = false;
      
    } else if (anet > UFT && timeNow - fallTimer < fallInterval) {
      accelTrig = true;
    }
  }

  // Fall Detection - LFT
  if (anet < LFT && fallFlag == false) {
    fallFlag = true;
    fallTimer = millis();
  }
  
  // Pressure Sensor validation
  if (fsrTrig == true && fsrval >= fsrT) {
    fsrTrig == false;
  } else if (fsrTrig == false && fsrval < fsrT) {
    fsrTrig == true;
  }
  
  // Fall Alarm
  if (accelTrig && fsrTrig) {
    alarmOn();
  }

  printToSerial(anet, fsrval, stepCount, accelTrig, fsrTrig);
}
