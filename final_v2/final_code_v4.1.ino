// useful links to refer to
// integration of esp32 with mpu6050 https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// research paper on anet, gnet combination algorithm https://www.hindawi.com/journals/js/2015/452078/

#include <Wire.h>            // include the Wire h library so we can communicate with I2C devices
#include <Adafruit_MPU6050.h> // include the mpu6050 library
#include <Adafruit_Sensor.h>  
//#include <avr/wdt.h>         // include the watchdog timer library to enable auto reboot 
#define soundPin 25           // define the pins for buzzer and leds (follow the gpio pinout for esp32)
#define greenPin 26
#define redPin 27

Adafruit_MPU6050 mpu;         // Create an Adafruit_MPU6050 object called mpu to handle the sensor
double ax, ay, az, anet;      // initialize the accelerations in xyz directions as floats 
double gx, gy, gz;            // initialize the orientation in xyz directions 

const float g = 9.81;        // initialize gravitatinal acceleration
const float LFT = 0.3;       // lower fall threshold
const float UFT = 2.4;       // upper fall threshold
const float UFT_G = 210;     // upper fall gyroscope threshold
const float ST = 1.25;       // step count threshold

bool fallFlag = false;       // initialize flags and alarm trigger
bool stepFlag = false;
bool alarmTrig = false;

unsigned long fallTimer = 0;    // initialize timers
unsigned long alarmTimer = 0;
unsigned long stepTimer = 0;
unsigned long printTimer = 0;

const long fallInterval = 500;    // initialize intervals
const long alarmInterval = 125;
const long stepInterval = 100;
const long printInterval = 200;

const int ledPin =  LED_BUILTIN;  //define built in led
int ledState = LOW;               // set the built in led state to low

unsigned int stepCount = 0;       // initialize the step count as 0
const double pi = 3.14159265358979323846 // define pi value
float skewness_net_acc = 0;
float mean_y_acc = 0;
float max_y_acc = 0;

// Function to calculate mean of data.
float mean(float arr[], int n){
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum = sum + arr[i];       
    return sum / n;
}
 
// Function to calculate standard deviation of data.
float standardDeviation(float arr[],int n){
    float sum = 0;
    // find standard deviation
    // deviation of data.
    for (int i = 0; i < n; i++)
        sum = (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n));
               
    return sqrt(sum / n);
}
 
// Function to calculate skewness.
float skewness(float arr[], int n){  
    // Find skewness using above formula
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum = (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n)) *
              (arr[i] - mean(arr, n));             
    return sum / (n * standardDeviation(arr, n) *
                 standardDeviation(arr, n) *
                 standardDeviation(arr, n) *
                 standardDeviation(arr, n));
}

// create circular queue class for storing acceleration values

class Queue{
    // Initialize front and rear
    int rear, front;
 
    // Circular Queue
    int size;
    int *arr;
 
    Queue(int s){
       front = rear = -1;
       size = s;
       arr = new int[s];
    }
 
    void enQueue(int value);
    int deQueue();
    void displayQueue();
}

/* Function to create Circular queue */
void Queue::enQueue(int value)
{
    if ((front == 0 && rear == size-1) ||
            (rear == (front-1)%(size-1)))
    {
        printf("\nQueue is Full");
        return;
    }
 
    else if (front == -1) /* Insert First Element */
    {
        front = rear = 0;
        arr[rear] = value;
    }
 
    else if (rear == size-1 && front != 0)
    {
        rear = 0;
        arr[rear] = value;
    }
 
    else
    {
        rear++;
        arr[rear] = value;
    }
}
 
// Function to delete element from Circular Queue
int Queue::deQueue()
{
    if (front == -1)
    {
        printf("\nQueue is Empty");
        return INT_MIN;
    }
 
    int data = arr[front];
    arr[front] = -1;
    if (front == rear)
    {
        front = -1;
        rear = -1;
    }
    else if (front == size-1)
        front = 0;
    else
        front++;
 
    return data;
}

void alarmOn() {
  digitalWrite(ledPin,HIGH);     // set the built in led pin to high
  digitalWrite(soundPin,HIGH);   // set the buzzer pin to high
  digitalWrite(redPin,HIGH);     // set the red led pin to high
  delay(200);
  digitalWrite(ledPin,LOW);      // set the followings to low
  digitalWrite(soundPin,LOW);
  digitalWrite(redPin,LOW);
}

void printToSerial(double ax, double ay, double az, double anet, int stepCount, bool alarmTrig, double gx, double gy, double gz, double gnet) {
  // print to serial after every print interval
  if (millis() - printTimer >= printInterval) { 
    printTimer = millis();

    // Display the respective xyz acceleration values in g
    Serial.print("AccX: "); Serial.print(ax);Serial.print(" g");
    Serial.print("AccY: "); Serial.print(ay);Serial.print(" g");
    Serial.print("AccZ: "); Serial.print(az);Serial.print(" g");

    // Display the net acceleration (acceleration is measured in g)
    Serial.print("Net Acc: "); Serial.print(anet); Serial.println(" g"); 

    // Display the respective xyz rotational acceleration values in degree/s
    Serial.print("GyroX: "); Serial.print(gx);Serial.print(" degree/s");
    Serial.print("GyroY: "); Serial.print(gy);Serial.print(" degree/s");
    Serial.print("GyroZ: "); Serial.print(gz);Serial.print(" degree/s");

    // Display the net gyroscopic acceleration (acceleration is measured in degree/s)
    Serial.print("Net Gyro: "); Serial.print(gnet); Serial.println(" degree/s"); 
  
    // Print number of steps
    Serial.print("Steps: "); Serial.println(stepCount); 

    if (alarmTrig) {
      // print "Fall detected!" if alarmTrig function is True
      Serial.println("Fall detected!");
    }
    // else print a new blank line
    Serial.println(" ");
  }
}

void setup() {
  Serial.begin(9600);                               // set the baud rate to 9600
  if (!mpu.begin()){                                // initialize the mpu6050 
    Serial.println("Fail to find MPU6050!");
    while (1){
      delay(10);
    }
  }
  Serial.println ("MPU6050 found!")
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);      // set the accelerometer range to +-2g

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);           // set the gyroscope range to +-500 degree

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);         // set the filter bandwidth to 5 Hz
  
  pinMode(LED_BUILTIN, OUTPUT);       // setup the built in led
  pinMode(soundPin, OUTPUT);          // setups for buzzer and led pins
  pinMode(greenPin, OUTPUT);
  pinMode(redPin,OUTPUT);
  wdt_enable(WDTO_2S);                // enable watchdog timer at a delay of 2 seconds
  Queue ay_history(12);
  Queue anet_history(12);
}

void loop() {
  wdt_reset();                        // reset the watchdog timer

  sensors_event_t a, g;               // get new sensor events with the current readings
  mpu.getEvent(&a, &g);

  ax = a.acceleration.x/g;              // assign the respective values into variables
  ay = a.acceleration.y/g;              // unit: g (gravitational acceleration)
  az = a.acceleration.z/g;

  gx = g.gyro.x*180/2*pi;               // unit: degrees/second
  gy = g.gyro.y*180/2*pi;
  gz = g.gyro.z*180/2*pi;
  
  anet = sqrt(ax*ax + ay*ay + az*az);       // getting the anet value
  gnet = sqrt(gx*gx + gy*gy + gz*gz);       // getting the gnet value

  if (sizeof(ay_history)!= 12 && sizeof(anet_history)!= 12){
    ay_history.enQueue(ay);
    anet_history.enQueue(anet);
  }
  else{
    ay_history.enQueue(ay);
    ay_history.deQueue();
    anet_history.enQueue(anet);
    anet_history.deQueue();

    skewness_net_acc = skewness(anet_history,sizeof(anet_history));
    mean_y_acc = mean(ay_history,sizeof(ay_history));
    max_y_acc = max(ay_history);
  }

  // =======Decision Tree=========//
  if (skewness_net_acc <= 5.010115385055542){
    if (max_y_acc > 1.301025390625){
      if (mean_y_acc > -0.707072913646698){
        alarmTrig = true;
      }
      else {
      alarmTrig = false;
      }
    }
    else {
      alarmTrig = false;
    }
  }
  else {
    if(mean_y_acc > -0.7913139462471008){
      alarmTrig = true;
    }
    else {
      alarmTrig = false;
    }
  }
  
  // ====step counting==== //
  if (anet > ST && stepFlag == false) {      // if anet is larger than step threshold and step flag equal false
    stepCount += 1;                          // increment step count by 1
    stepFlag = true;                         // set the step flag to true
    stepTimer = millis();                    // assign the current elasped time to the step timer
    
  } else if (stepFlag == true) {
    // Minimum delay for registering a new step
    if (anet <= ST && millis() - stepTimer >= stepInterval) {   // if anet is smaller than step threshold and the time taken is larger than step interval
      stepFlag = false;                                         // set step flag to false
    }
  }
  
  // ====Fall Detection - Lower fall threshold detection==== //
  if (anet < LFT && fallFlag == false) {                        // if anet is smaller than lower fall threshold and fall flag is false
                                                                //fallFlag means that LFT has been reached
    fallFlag = true;                                            // set fall flag to true
    fallTimer = millis();                                       // assign the elasped time to fallTimer
  }

  // ====Fall Detection - Upper fall threshold detection==== //
  if (fallFlag == true) {                                       // if fallFlag is true, aka exceeded lower fall threshold
    if (millis() - fallTimer >= fallInterval) {                 // check the time taken with predefined fall interval
      fallFlag = false;                                         //  reset the fall flag to false 
    } else if (anet > UFT && gnet > UFT_G && millis() - fallTimer < fallInterval) {     // if anet larger than upper fall thresholds and time taken within fall interval
      alarmTrig = true;                                                 // set the alarmTrig to true
    }
   }
  
    // ====Fall Alarm==== //
   if (alarmTrig) {                             // if alarmTrig is True
   alarmOn();                                   // call the alarmOn function
   }
   printToSerial(ax, ay, az, anet, stepCount, alarmTrig, gx, gy, gz, gnet);         // print the accelerations, anet, step count, alarm trigger status and gyro values to serial monitor
}
