#include <Wire.h>
#include <Arduino.h>
#include <ArduinoJson.h>
// #include <Adafruit_BMP280.h>

// Adafruit_BMP280 bmp;

unsigned long lastTime = 0;
int flowPin = A0;
int flowRate = 0;
int flowRate2;
int flowRate3;

const int flowMeter1Pin = A0; // Flow meter 1 input pin
const int flowMeter2Pin = A2;

int getFlowRateFromSensor(int pin) {
  int flowSensorValue = analogRead(pin);
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= 1000) { // calculate flow rate once per second
    flowRate = map(flowSensorValue, 0, 1023, 0, 60); // adjust this range based on sensor specifications
    lastTime = currentTime;
  }
  return flowRate;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  // bmp.begin(0x76);
  pinMode(flowMeter1Pin, INPUT);
  pinMode(flowMeter2Pin, INPUT);

  // set the pins for flow meters to high.
  digitalWrite(flowMeter1Pin, INPUT_PULLUP);
  digitalWrite(flowMeter2Pin, INPUT_PULLUP);
}

void loop() {


  flowRate2 = getFlowRateFromSensor(flowMeter1Pin);
  flowRate3 = getFlowRateFromSensor(flowMeter2Pin);
  Serial.print("flowRate2 \n");
  Serial.println(flowRate2);
  Serial.print("flowRate3 \n");
  Serial.println(flowRate3);

  //float temperature = bmp.readTemperature();
  // float pressure = bmp.readPressure() / 100.0F;
  bool isOn = true;

  // Create a JSON object
  StaticJsonDocument<100> doc;
  doc["temperature"] = flowRate2;
  doc["pressure"] = flowRate2;
  doc["isOn"] = isOn;

  // Serialize the JSON object to a string and send it over serial
  String jsonStr;
  serializeJson(doc, jsonStr);
  Serial.println(jsonStr);

  delay(1000);
}