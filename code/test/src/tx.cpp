#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

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
  bmp.begin(0x76);
  pinMode(flowMeter1Pin, INPUT);
  pinMode(flowMeter2Pin, INPUT);

  // set the pins for flow meters to high.
  digitalWrite(flowMeter1Pin, INPUT_PULLUP);
  digitalWrite(flowMeter2Pin, INPUT_PULLUP);
}

void loop() {

  flowRate2 = getFlowRateFromSensor(flowMeter1Pin);
  flowRate3 = getFlowRateFromSensor(flowMeter2Pin);
  Serial.print("flowRate2");
  Serial.print(flowRate2);
  Serial.print("flowRate3");
  Serial.print(flowRate3);

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
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
