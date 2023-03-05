#include <WiFi.h>
#include <InfluxDbClient.h>

// WiFi settings
const char* ssid = "your_SSID";
const char* password = "your_WIFI_password";

// InfluxDB settings
const char* influxdbHost = "your_InfluxDB_host";
const uint16_t influxdbPort = 8086;
const char* influxdbDatabase = "your_InfluxDB_database";
const char* influxdbUser = "your_InfluxDB_user";
const char* influxdbPassword = "your_InfluxDB_password";

// Create an instance of the InfluxDB client
InfluxDBClient influxdb(influxdbHost, influxdbPort, influxdbDatabase, influxdbUser, influxdbPassword);

void setup() {
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to InfluxDB
  if (influxdb.connect()) {
    Serial.println("Connected to InfluxDB");
  } else {
    Serial.println("Failed to connect to InfluxDB");
  }
}

void loop() {
  // Read sensor data
  float flowrate = 25.0;
  float pressure = 50.0;

  // Prepare data point
  Point dataPoint("temperature_humidity");
  dataPoint.addField("pressure", pressure);
  dataPoint.addField("flowrate", flowrate);

  // Write data point to InfluxDB
  if (influxdb.write(dataPoint)) {
    Serial.println("Data point sent to InfluxDB");
  } else {
    Serial.println("Failed to send data point to InfluxDB");
  }

  // Wait for 10 seconds
  delay(10000);
}
