#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String jsonString = Serial.readStringUntil('\n');
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
      Serial.print("Error deserializing JSON: ");
      Serial.println(error.c_str());
    } else {
      float temperature = doc["temperature"];
      float pressure = doc["pressure"];
      bool isOn = doc["isOn"];

      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Pressure: ");
      Serial.println(pressure);
      Serial.print("Is on: ");
      Serial.println(isOn);
    }
  }
}
