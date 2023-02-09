// Function to publish the pressure sensor values to MQTT
void publishPressureSensorData() {
    // Create the JSON object to store the sensor data
    StaticJsonDocument<200> doc;
    doc["pressureSensor1"] = pressureSensor1;
    doc["pressureSensor2"] = pressureSensor2;
    doc["pressureSensor3"] = pressureSensor3;
    doc["pressureSensor4"] = pressureSensor4;
    doc["pressureSensor5"] = pressureSensor5;
    
    // Serialize the JSON object
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Publish the data to the MQTT broker
    client.publish("pressureSensors", jsonString.c_str());
}