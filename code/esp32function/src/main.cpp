#include <WiFi.h>
#include <InfluxDbClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

// Wi-Fi network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// InfluxDB settings
const char* server = "your_INFLUXDB_IP_ADDRESS";
const uint16_t port = 8086;
const char* database = "your_INFLUXDB_DATABASE_NAME";
const char* measurement = "your_INFLUXDB_MEASUREMENT_NAME";
const char* user = "your_INFLUXDB_USERNAME";
const char* influxpassword = "your_INFLUXDB_PASSWORD";

// Slave Arduino Mega I2C address
#define SLAVE_ADDR 9

// Solenoid pins. Change pin numbers to yours
#define SOLENOID1_PIN 2
#define SOLENOID2_PIN 3
#define SOLENOID3_PIN 4

// Initialize InfluxDB client instance
InfluxDBClient influxdb(server, port, database, user, influxpassword);

// Initialize Wi-Fi client instance
//WiFiClient wifiClient;

// Sensor data storing
float flowrate;
float pressure;
float temperature;
bool isOn;

// Function to turn solenoid on/off via I2C
void turnSolenoid(int solenoidNum, bool state) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(solenoidNum);
  Wire.write(state);
  Wire.endTransmission();
}

// Function to handle incoming messages
void handleMessage(String message) {
  if (message == "solenoid1_on") {
    turnSolenoid(SOLENOID1_PIN, HIGH);
  } else if (message == "solenoid1_off") {
    turnSolenoid(SOLENOID1_PIN, LOW);
  } else if (message == "solenoid2_on") {
    turnSolenoid(SOLENOID2_PIN, HIGH);
  } else if (message == "solenoid2_off") {
    turnSolenoid(SOLENOID2_PIN, LOW);
  } else if (message == "solenoid3_on") {
    turnSolenoid(SOLENOID3_PIN, HIGH);
  } else if (message == "solenoid3_off") {
    turnSolenoid(SOLENOID3_PIN, LOW);
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi!");

  // Connect to InfluxDB
  if (influxdb.validateConnection()) {
    Serial.println("Connected to InfluxDB");
  } else {
    Serial.println("Failed to connect to InfluxDB");
  }
  
  // Initialize I2C communication
  Wire.begin();
}

void loop() {
  // Check for incoming messages
  if (Serial.available() > 0 && Serial.available()) {
    String jsonString = Serial.readStringUntil('\n');
    String message = Serial.readStringUntil('\n');
    handleMessage(message);
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
      Serial.print("Error deserializing JSON: ");
      Serial.println(error.c_str());
    } else {
      temperature = doc["temperature"];
      pressure = doc["pressure"];
      isOn = doc["isOn"];

      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Pressure: ");
      Serial.println(pressure);
      Serial.print("Is on: ");
      Serial.println(isOn);
    }
  }

  // Prepare data point
  Point dataPoint("engine_v6");
  dataPoint.addField("pressure", pressure);
  dataPoint.addField("flowrate", flowrate);
  dataPoint.addField("isOn", isOn);

  // Write data point to InfluxDB
  if (influxdb.writePoint(dataPoint)) {
    Serial.println("Data point sent to InfluxDB");
  } else {
    Serial.println("Failed to send data point to InfluxDB");
  }

  // Wait for 10 seconds
  delay(10000);

}