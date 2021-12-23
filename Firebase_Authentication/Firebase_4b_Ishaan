#include <common.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <Utils.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

const char* WIFI_SSID = "hotspotspothot";
const char* WIFI_PASSWORD = "12345678";
const char* API_KEY = "AIzaSyBeRnBFoUmi7OBAi9SJnOyPv7yTbMeB3PE";
const char* DATABASE_URL = "https://test-62586-default-rtdb.asia-southeast1.firebasedatabase.app/";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config_x;
FirebaseJsonArray* aXArr;
FirebaseJsonArray* aYArr;
FirebaseJsonArray* aZArr;
FirebaseJsonArray* gXArr;
FirebaseJsonArray* gYArr;
FirebaseJsonArray* gZArr;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


#define ledPin 4

#define GRA 9.82
#define CONVERT_DEG(X) (X * (180 / (2 * 3.14)))
#define LFT 0.6
#define UFT 2.4
#define UFT_G 210

#define FALL_INTERVAL 500
#define ALARM_INTERVAL 125

bool lowThreshPassFlag = 0;
bool highThreshPassFlag = 0;
bool fallFlag = 0;

unsigned long fallTimer = 0;
unsigned long alarmTimer = 0;


Adafruit_MPU6050 mpu;

void alarmOn() {
  digitalWrite(ledPin,HIGH);
  delay(400);
  digitalWrite(ledPin,LOW);
  delay(400);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup Begin");
  pinMode(ledPin,OUTPUT);
  while (!mpu.begin()) {
    Serial.println("Failed to find mpu");
    delay(50);
  }
  Serial.println("mpu connected");
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.println("connecting");
  while (WiFi.status() != WL_CONNECTED){
   Serial.print(".");
   delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP().toString());

  config_x.api_key = API_KEY;
  config_x.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config_x, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config_x.signer.signupError.message.c_str());
  }

    /* Assign the callback function for the long running token generation task */
  config_x.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config_x, &auth);
  Firebase.reconnectWiFi(true);
  aXArr = new FirebaseJsonArray();
  aYArr = new FirebaseJsonArray();
  aZArr = new FirebaseJsonArray();
  gXArr = new FirebaseJsonArray();
  gYArr = new FirebaseJsonArray();
  gZArr = new FirebaseJsonArray();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a,&g,&temp);

 double ax = (double)a.acceleration.x/(double)GRA;
 double ay = (double)a.acceleration.y/(double)GRA;
 double az = (double)a.acceleration.z/(double)GRA;

 double gx = CONVERT_DEG(g.gyro.x);
 double gy = CONVERT_DEG(g.gyro.y);
 double gz = CONVERT_DEG(g.gyro.z);

 double anet = sqrt(ax*ax + ay*ay + az*az);
 double gnet = sqrt(gx*gx + gy*gy + gz*gz);

 Serial.println("Acc:");
 Serial.println(ax);
 Serial.println(ay);
 Serial.println(az);

 Serial.println("Gyro:");
 Serial.println(gx);
 Serial.println(gy);
 Serial.println(gz);

 // ====Fall Detection - Lower fall threshold detection==== //
 if (anet < LFT && lowThreshPassFlag == false) {                        // if anet is smaller than lower fall threshold and fall flag is false
                                                                //fallFlag means that LFT has been reached
   lowThreshPassFlag = true;                                            // set fall flag to true
   fallTimer = millis();                                       // assign the elasped time to fallTimer
 }


 // ====Fall Detection - Upper fall threshold detection==== //
 if (lowThreshPassFlag == true) {                                       // if fallFlag is true, aka exceeded lower fall threshold
   if (millis() - fallTimer >= FALL_INTERVAL) {                 // check the time taken with predefined fall interval
     lowThreshPassFlag = false;                                         //  reset the fall flag to false 
   } else if (anet > UFT && gnet > UFT_G && millis() - fallTimer < FALL_INTERVAL) {     // if anet larger than upper fall thresholds and time taken within fall interval
     highThreshPassFlag = true;                                                 // set the alarmTrig to true
   }
  }

 // ====Fall Alarm==== //
 if (highThreshPassFlag) {                             // if alarmTrig is True
  alarmOn();                                   // call the alarmOn function
 }
 if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
  sendDataPrevMillis = millis();
  if (Firebase.RTDB.setInt(&fbdo, "test/int", (int)highThreshPassFlag)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      highThreshPassFlag = 0;
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 } 
// aXArr->add(ax);
// aYArr->add(ay);
// aZArr->add(az);
// gXArr->add(gx);
// gYArr->add(gy);
 gZArr->add(gz);
 if (gZArr->size() >= 400) {
  if (Firebase.ready() && signupOK) {
//    Firebase.RTDB.setArray(&fbdo, "ax/array", aXArr);
//    Firebase.RTDB.setArray(&fbdo, "ay/array", aYArr);
//    Firebase.RTDB.setArray(&fbdo, "az/array", aZArr);
//    Firebase.RTDB.setArray(&fbdo, "gx/array", gXArr);
//    Firebase.RTDB.setArray(&fbdo, "gy/array", gYArr);
    if (Firebase.RTDB.setArray(&fbdo, "gz/array", gZArr)) {
      Serial.println("Passed");
    } else {
      Serial.println("Failed : " + fbdo.errorReason());
    }
  }
  aXArr->clear();
  aYArr->clear();
  aZArr->clear();
  gXArr->clear();
  gYArr->clear();
  gZArr->clear();
 }
 delay(50);
}
