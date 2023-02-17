#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFi.h>
#include "config.h"
#include "PID.h"
#include <HTTPClient.h>
// #include <MAX6675.h>
#include <max6675.h>
#include <Stepper.h>
//#include "publish.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
// #include "./config.h"
#include "./flow_control.cpp"

#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_USE_DOUBLE 1

// Pin definitions for the MAX6675 thermocouples
#define thermocouple1_CS 12
#define thermocouple1_SCK 13
#define thermocouple1_SO 14

#define thermocouple2_CS 15
#define thermocouple2_SCK 16
#define thermocouple2_SO 17

#define thermocouple3_CS 18
#define thermocouple3_SCK 19
#define thermocouple3_SO 20

// Create the PubSubClient object for connecting to the MQTT broker
// PubSubClient client(mqttServer, mqttPort);
WiFiClient espClient;
PubSubClient client(espClient);

// Create the MAX6675 thermocouple objects
MAX6675 thermocouple1(thermocouple1_SCK, thermocouple1_CS, thermocouple1_SO);
MAX6675 thermocouple2(thermocouple2_SCK, thermocouple2_CS, thermocouple2_SO);
MAX6675 thermocouple3(thermocouple3_SCK, thermocouple3_CS, thermocouple3_SO);

const int stepsPerRevolution = 200;  // change this to match the number of steps per revolution for your stepper motor
Stepper myStepper(stepsPerRevolution, pin1, pin2, pin3, pin4);  // initialize the stepper library

// flow sensor pins
const int flowMeter1Pin = A0; // Flow meter 1 input pin
const int flowMeter2Pin = A1; // Flow meter 2 input pin
const int flowMeter3Pin = A2; // Flow meter 3 input pin
const int flowMeter4Pin = A3; // Flow meter 4 input pin
const int flowMeter5Pin = A4; // Flow meter 5 input pin
// flow meter interrupt pin
const int flowinterrupt1 = 2; // for the flow meters
const int flowinterrupt2 = 2;
const int flowinterrupt3 = 2;
const int flowinterrupt4 = 2;
const int flowinterrupt5 = 2;

// solenoid pins
const int solenoid1Pin = 12; // Solenoid 1 control pin
const int solenoid2Pin = 13; // Solenoid 2 control pin
const int solenoid3Pin = 14; // Solenoid 3 control pin
const int solenoid4Pin = 15; // Solenoid 4 control pin
const int solenoid5Pin = 16; // Solenoid 5 control pin

unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

// solenoid pins state
bool solenoid1State = false;
bool solenoid2State = false;
bool solenoid3State = false;
bool solenoid4State = false;
bool solenoid5State = false;

//pressure sensor pins.
const int pressureSensor1 = A0;
const int pressureSensor2 = A1;
const int pressureSensor3 = A2;
const int pressureSensor4 = A3;
const int pressureSensor5 = A4;

//some vrs to use
/*
int O_pulseCount = 0;
int N_pulseCount = 0;
float OxygenflowRate = 0.0;
float NitrogenflowRate = 0.0;
float flowMilliLitres = 0;
float totalMilliLitres = 0;
long OxygenpreviousMillis = 0;
long NitrogenpreviousMillis = 0;
*/
int interval = 1000;

const float calibrationFactor = 4.5;

volatile byte pulseCount1;
volatile byte pulseCount2;
volatile byte pulseCount3;
volatile byte pulseCount4;
volatile byte pulseCount5;

byte pulse1Sec = 0;
byte pulse2Sec = 0;
byte pulse3Sec = 0;
byte pulse4Sec = 0;
byte pulse5Sec = 0;

/*
volatile byte O_pulseCount;
volatile byte N_pulseCount;

byte O_pulse1Sec = 0;
byte N_pulse1Sec = 0;
*/

float flowrate1;
float flowrate2;
float flowrate3;
float flowrate4;
float flowrate5;

long OxygencurrentMillis = 0;
long OxygenpreviousMillis = 0;

long currentmillis1 = 0;
long currentmillis2 = 0;
long currentmillis3 = 0;
long currentmillis4 = 0;
long currentmillis5 = 0;

long previousmillis1 = 0;
long previousmillis2 = 0;
long previousmillis3 = 0;
long previousmillis4 = 0;
long previousmillis5 = 0;


/*
WiFiClient espClient;
PubSubClient client(espClient);
long OxygencurrentMillis = 0;
long OxygenpreviousMillis = 0;
long NitrogencurrentMillis = 0;
long NitrogenpreviousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte O_pulseCount;
volatile byte N_pulseCount;
byte O_pulse1Sec = 0;
byte N_pulse1Sec = 0;
float OxygenflowRate;
float NitrogenflowRate;
*/

unsigned int flowMilliLitres;
unsigned long totalMilliLitres;


void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}

void IRAM_ATTR pulseCounter3()
{
  pulseCount3++;
}

void IRAM_ATTR pulseCounter4()
{
  pulseCount4++;
}

void IRAM_ATTR pulseCounter5()
{
  pulseCount5++;
}

// function to est wifi connection
void setup_wifi() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    client.setServer(mqttServer, mqttPort);
    // client.setCallback(callback);
}

// function to reconnection in event of connection loss
void reconnect() {
    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUsername, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.println("Connection failed, retrying in 5 seconds");
      delay(5000);
    }
  }
}

bool check_connection(){
    if (!client.connected()){
        Serial.println("server disconnected...");
        return false;
    }
    return true;
}


void setup() {
    Serial.begin(115200);
    delay(1000);
    setup_wifi();
    client.setServer(mqttServer, 1883);
    delay(1000);

    // Initialize stepper motors and solenoid valves
    pinMode(stepper1Pin, OUTPUT);
    pinMode(stepper2Pin, OUTPUT);

    pinMode(flowMeter1Pin, INPUT);
    pinMode(flowMeter2Pin, INPUT);
    pinMode(flowMeter3Pin, INPUT);
    pinMode(flowMeter4Pin, INPUT);
    pinMode(flowMeter5Pin, INPUT);

    pinMode(solenoid1Pin, OUTPUT);
    pinMode(solenoid2Pin, OUTPUT);
    pinMode(solenoid3Pin, OUTPUT);
    pinMode(solenoid4Pin, OUTPUT);
    pinMode(solenoid5Pin, OUTPUT);

    pinMode(pressureSensor1, INPUT);
    pinMode(pressureSensor2, INPUT);
    pinMode(pressureSensor3, INPUT);
    pinMode(pressureSensor4, INPUT);
    pinMode(pressureSensor5, INPUT);

    // set the pins for flow meters to high.
    digitalWrite(flowMeter1Pin, INPUT_PULLUP);
    digitalWrite(flowMeter2Pin, INPUT_PULLUP);
    digitalWrite(flowMeter3Pin, INPUT_PULLUP);
    digitalWrite(flowMeter4Pin, INPUT_PULLUP);
    digitalWrite(flowMeter5Pin, INPUT_PULLUP);
    /*
    pinMode(OxygenpressureInput, INPUT);
    pinMode(OxygensolenoidPin, OUTPUT);
    digitalWrite(OxygensolenoidPin,LOW);
    pinMode(OXYGENFLOWSENSOR, INPUT_PULLUP);
    pinMode(NitrogenpressureInput, INPUT);
    pinMode(NitrogensolenoidPin, OUTPUT);
    digitalWrite(NitrogensolenoidPin,LOW);
    pinMode(NITROGENFLOWSENSOR, INPUT_PULLUP);
    
    
    
    O_pulseCount = 0;
    N_pulseCount = 0;
    OxygenflowRate = 0.0;
    NitrogenflowRate = 0.0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    OxygenpreviousMillis = 0;
    NitrogenpreviousMillis = 0;
    */
    
    // attachInterrupt(digitalPinToInterrupt(OXYGENFLOWSENSOR), pulseCounter1, FALLING);
    attachInterrupt(digitalPinToInterrupt(flowinterrupt1), pulseCounter1, FALLING);
    attachInterrupt(digitalPinToInterrupt(flowinterrupt2), pulseCounter2, FALLING);
    attachInterrupt(digitalPinToInterrupt(flowinterrupt3), pulseCounter3, FALLING);
    attachInterrupt(digitalPinToInterrupt(flowinterrupt4), pulseCounter4, FALLING);
    attachInterrupt(digitalPinToInterrupt(flowinterrupt5), pulseCounter5, FALLING);
}

void loop() {

  if (!client.connected()){
        Serial.println("Server disconnected...");
        delay(200);
        reconnect();
    }

  // Read the pressure readings from the pressure sensors
  float pressure1 = analogRead(pressureSensor1);
  float pressure2 = analogRead(pressureSensor2);
  float pressure3 = analogRead(pressureSensor3);
  float pressure4 = analogRead(pressureSensor4);
  float pressure5 = analogRead(pressureSensor5);

  // Read the temperature readings from the thermocouples
  float temperature1 = thermocouple1.readCelsius();
  float temperature2 = thermocouple2.readCelsius();
  float temperature3 = thermocouple3.readCelsius();
  
  float pressvl1 = map(pressure1, 0, 4095, 0, 10);
  float pressvl2 = map(pressure2, 0, 4095, 0, 10);
  float pressvl3 = map(pressure3, 0, 4095, 0, 10);
  float pressvl4 = map(pressure4, 0, 4095, 0, 10);
  float pressvl5 = map(pressure5, 0, 4095, 0, 10);

  /*
  Serial.print("Pressure output value = ");
  Serial.print("O2");
  Serial.println(OxygenoutputValue);
  Serial.print("N2");
  Serial.println(NitrogenoutputValue);
  */

  
  //Serial.print(" bar"); 
  if (solenoid1State == false && pressvl1  >= 3){
    digitalWrite(solenoid1Pin,HIGH);
    solenoid1State = true;
  }
  if (solenoid2State == false && pressvl2  >= 3){
    digitalWrite(solenoid2Pin,HIGH);
    solenoid2State = true;
  }
  if (solenoid3State == false && pressvl3  >= 3){
    digitalWrite(solenoid3Pin,HIGH);
    solenoid3State = true;
  }
  if (solenoid4State == false && pressvl4  >= 3){
    digitalWrite(solenoid4Pin,HIGH);
    solenoid4State = true;
  }
  if (solenoid5State == false && pressvl5  >= 3){
    digitalWrite(solenoid5Pin,HIGH);
    solenoid5State = true;
  }

  /*
  OxygencurrentMillis = millis();
  NitrogencurrentMillis = millis();
  */
  currentmillis1 = millis();
  currentmillis2 = millis();
  currentmillis3 = millis();
  currentmillis4 = millis();
  currentmillis5 = millis();
  
  /*
  if (OxygencurrentMillis - OxygenpreviousMillis > interval) {
    O_pulse1Sec = O_pulseCount;
    O_pulseCount = 0; 
    OxygenflowRate = ((1000.0/ (millis() - OxygenpreviousMillis)) * O_pulse1Sec ) / calibrationFactor;
  */

 if (currentmillis1 - previousmillis1 > interval) {
  pulse1Sec = pulseCount1;
  pulseCount1 = 0; 
  flowrate1 = ((1000.0/ (millis() - previousmillis1)) * pulse1Sec ) / calibrationFactor;

  previousmillis1 = millis();
  delay(1000);
  Serial.print("Flow rate: ");
  Serial.print(int(flowrate1));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");       // Print tab space
  delay(5000);

 }
 if (currentmillis2 - previousmillis2 > interval) {
  pulse2Sec = pulseCount2;
  pulseCount2 = 0; 
  flowrate2 = ((1000.0/ (millis() - previousmillis2)) * pulse2Sec ) / calibrationFactor;

  previousmillis2 = millis();
  delay(1000);
  Serial.print("Flow rate: ");
  Serial.print(int(flowrate2));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");       // Print tab space
  delay(5000);

 }
 if (currentmillis3 - previousmillis3 > interval) {
  pulse1Sec = pulseCount3;
  pulseCount3 = 0; 
  flowrate3 = ((1000.0/ (millis() - previousmillis3)) * pulse3Sec ) / calibrationFactor;

  previousmillis3 = millis();
  delay(1000);
  Serial.print("Flow rate: ");
  Serial.print(int(flowrate3));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");       // Print tab space
  delay(5000);

 }
 if (currentmillis4 - previousmillis4 > interval) {
  pulse1Sec = pulseCount4;
  pulseCount4 = 0; 
  flowrate4 = ((1000.0/ (millis() - previousmillis4)) * pulse4Sec ) / calibrationFactor;

  previousmillis4 = millis();
  delay(1000);
  Serial.print("Flow rate: ");
  Serial.print(int(flowrate4));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");     // Print tab space
  delay(5000);

 }
 if (currentmillis5 - previousmillis5 > interval) {
  pulse5Sec = pulseCount5;
  pulseCount5 = 0; 
  flowrate5 = ((1000.0/ (millis() - previousmillis5)) * pulse5Sec ) / calibrationFactor;

  previousmillis5 = millis();
  delay(1000);
  Serial.print("Flow rate: ");
  Serial.print(int(flowrate5));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");       // Print tab space
  delay(5000);
 }


    /*
    OxygenpreviousMillis = millis();
    delay(1000);
    Serial.print("Flow rate: ");
    Serial.print(int(OxygenflowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    delay(5000);
    
    if (NitrogencurrentMillis - NitrogenpreviousMillis > interval) {
      N_pulse1Sec = N_pulseCount;
      N_pulseCount = 0; 
    
    NitrogenflowRate = ((1000.0/ (millis() - NitrogenpreviousMillis)) * N_pulse1Sec) / calibrationFactor;
    NitrogenpreviousMillis = millis();
    delay(1000);
    Serial.print("Flow rate: ");
    Serial.print(int(NitrogenflowRate));
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    */
    
  
   delay(5000);
    
    // Create the JSON object to store the sensor data
    StaticJsonDocument<200> doc;
    doc["pressureSensor1"] = pressure1;
    doc["pressureSensor2"] = pressure2;
    doc["pressureSensor3"] = pressure3;
    doc["pressureSensor4"] = pressure4;
    doc["pressureSensor5"] = pressure5;
    doc["pressureSensor5"] = pressure5;
    doc["temperature1"] = temperature1;
    doc["temperature2"] = temperature2;
    doc["temperature3"] = temperature3;
    doc["pressvl1"] = pressvl1;
    doc["pressvl2"] = pressvl2;
    doc["pressvl3"] = pressvl3;
    doc["pressvl4"] = pressvl4;
    doc["pressvl5"] = pressvl5;
    
    // Serialize the JSON object
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Publish the data to the MQTT broker
    client.publish("pressureSensors", jsonString.c_str());
}