#include <WiFi.h>
#include <InfluxDbClient.h>
#include <Wire.h>

// Wi-Fi network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// InfluxDB settings
const char* server = "your_INFLUXDB_IP_ADDRESS";
const uint16_t port = 8086;
const char* database = "your_INFLUXDB_DATABASE_NAME";
const char* measurement = "your_INFLUXDB_MEASUREMENT_NAME";
const char* user = "your_INFLUXDB_USERNAME";
const char* password = "your_INFLUXDB_PASSWORD";

// Slave Arduino Mega I2C address
#define SLAVE_ADDR 9

// Solenoid pins
#define SOLENOID1_PIN 2
#define SOLENOID2_PIN 3
#define SOLENOID3_PIN 4

// Initialize InfluxDB client instance
InfluxDBClient client(server, port, database, user, password);

// Initialize Wi-Fi client instance
WiFiClient wifiClient;

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
  
  // Initialize I2C communication
  Wire.begin();
}

void loop() {
  // Check for incoming messages
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    handleMessage(message);
  }
}
