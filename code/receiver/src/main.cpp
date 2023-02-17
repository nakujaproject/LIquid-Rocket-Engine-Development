#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* mqttServer = "your_mqtt_broker_url";
const char* mqttUsername = "your_mqtt_username";
const char* mqttPassword = "your_mqtt_password";
const char* mqttClientId = "your_mqtt_client_id";
const char* mqttTopic = "your_mqtt_topic";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

float temperature = 0;
float pressure = 0;
bool isOn = false;

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set MQTT server and port
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(callback);

  // Connect to MQTT broker
  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqttClientId, mqttUsername, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
      mqttClient.subscribe(mqttTopic);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
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

    // Prepare JSON data to send
    StaticJsonDocument<200> dataToSend;
    dataToSend["temperature"] = temperature;
    dataToSend["pressure"] = pressure;
    dataToSend["isOn"] = isOn;

    // Serialize JSON data
    String jsonString_new;
    serializeJson(dataToSend, jsonString_new);

    // Publish the JSON data to the MQTT broker
    mqttClient.publish(mqttTopic, jsonString.c_str());

    Serial.println("Data sent successfully");
  }
  // Check for incoming MQTT messages
  mqttClient.loop();
}
