#include <DHT.h>
#include <ros.h>
#include <std_msgs/Float64.h>
#include <std_msgs/UInt16.h>

// setting the pins for the Temp and Humidity Sensors

#define outerTempSensor 8
#define outerTempDHT DHT22
#define innerTempSensor 9
#define innerTempDHT DHT22

//creates floats for inner and outer temp and humidity

float outerTemp;
float outerHum;

float innerTemp;
float innerHum;

DHT dhtOuter(outerTempSensor, outerTempDHT);
DHT dhtInner(innerTempSensor, innerTempDHT);


// Initialzing pins and variables for flow meter
byte statusLed= 13;
byte sensorInterrupt = 0;
byte sensorPin = 2;

float calibrationFactor = 4.5;

volatile byte pulseCount;

float flowRate;
unsigned int flowMillilitres;
unsigned long oldTime;



//creating the ros publishers

ros::NodeHandle nh;

std_msgs::Float64 oTemp;
std_msgs::Float64 iTemp;
std_msgs::Float64 oHum;
std_msgs::Float64 iHum;
std_msgs::Float64 flrate;
std_msgs::UInt16 ldr;

ros::Publisher outerTemperature("outerTemp", &oTemp);
ros::Publisher innerTemperature("innerTemp",&iTemp);
ros::Publisher outerHumidity("outerHum",&oHum);
ros::Publisher innerHumidity("innerHum",&iHum);
ros::Publisher rateOfFlow("flowrate",&flrate);
ros::Publisher lightReading("lightIntensity",&ldr);




void setup() {
  
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);

  pinMode(sensorPin, INPUT);
  digitalWrite(statusLed, HIGH);
  
  //variables for flow meter 
  pulseCount = 0;
  flowRate = 0.0;
  flowMillilitres = 0;
  oldTime = 0;


  //sets the two temp and humidity sensors to start 
  dhtOuter.begin();
  dhtInner.begin();


  //initalises the ros nodes
  nh.initNode();
  nh.advertise(outerTemperature);
  nh.advertise(innerTemperature);
  nh.advertise(outerHumidity);
  nh.advertise(innerHumidity);
  nh.advertise(rateOfFlow);
  nh.advertise(lightReading);
  
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop() {
  //Flow meter logic
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 

    detachInterrupt(sensorInterrupt);
        

    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

    oldTime = millis();
    

    flowMillilitres = (flowRate / 60) * 1000;
    unsigned int frac;
    pulseCount = 0;
    
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

  //this section of code sends ros messages to roscore according to the set delay

  delay(2000);
  outerHum = dhtOuter.readHumidity();
  outerTemp = dhtOuter.readTemperature();
  innerHum = dhtInner.readHumidity();
  innerTemp= dhtInner.readTemperature();
  
  int lightValue = analogRead(A2); 

  oTemp.data=outerTemp;
  iTemp.data=innerTemp;
  oHum.data=outerHum;
  iHum.data=innerHum;
  flrate.data=flowRate;
  ldr.data=lightValue;

  outerTemperature.publish(&oTemp);
  innerTemperature.publish(&iTemp);
  outerHumidity.publish(&oHum);
  innerHumidity.publish(&iHum);
  rateOfFlow.publish(&flrate);
  lightReading.publish(&ldr);
  
  nh.spinOnce();
}

//pulse counter method
void pulseCounter()
{
  pulseCount++;
  }
