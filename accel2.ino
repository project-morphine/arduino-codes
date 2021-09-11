#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
const float g = 9.81;

void setup() 
{
  Serial.begin(9600);
  accel.begin();

  /* Set the range and data rate to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_8_G);
  accel.setDataRate(ADXL345_DATARATE_25_HZ);
}

void loop() 
{
  /* Get a new sensor event */ 
  sensors_event_t event;
  double ax, ay, az, anet;
  
  accel.getEvent(&event);
  ax = event.acceleration.x/g;
  ay = event.acceleration.y/g;
  az = event.acceleration.z/g;
  anet = sqrt(ax*ax + ay*ay + az*az);
  
  /* Display the results (acceleration is measured in g) */
  Serial.print("Net: ");
  Serial.print(anet);
  Serial.println(" g");
}
