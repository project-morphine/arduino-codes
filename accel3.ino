#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
const float g = 9.81;
const float LFT = 0.8;
const float UFT = 1.2;

bool trig = false;
unsigned long previousMillis = 0;
const long interval = 125;

const int ledPin =  LED_BUILTIN;
int ledState = LOW;

void alarmOn() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

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

void setup() {
  Serial.begin(9600);
  accel.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  /* Set the range and data rate to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_8_G);
  accel.setDataRate(ADXL345_DATARATE_25_HZ);
}

void loop() {
  /* Get a new sensor event */ 
  sensors_event_t event;
  double ax, ay, az, anet;
  
  accel.getEvent(&event);
  ax = event.acceleration.x/g;
  ay = event.acceleration.y/g;
  az = event.acceleration.z/g;
  anet = sqrt(ax*ax + ay*ay + az*az);
  
  /* Display the results (acceleration is measured in g) */
  Serial.print("Net: "); Serial.print(anet); Serial.println(" g");

  if (anet > UFT || anet < LFT) {
    trig = true;
  }

  if (trig) {
    alarmOn();
  }
}
