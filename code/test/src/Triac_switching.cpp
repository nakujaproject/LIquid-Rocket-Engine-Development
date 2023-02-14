#include <Arduino.h>

const int triacPin = 13;
const int sensorPin = A0;
const float desiredFlowRate = 50.0;
const float kp = 0.1;
const float cycles = 120;

void setup() {
  pinMode(triacPin, OUTPUT);
  pinMode(sensorPin, INPUT);
}

void loop() {
  // Read the current flow rate value from the flow sensor
  float currentFlowRate = analogRead(sensorPin);
  // Calculate the error between the desired flow rate and the current flow rate
  float error = desiredFlowRate - currentFlowRate;
  // Use the proportional control algorithm to adjust the Triac's firing angle
  float firingAngle = kp * error;
  // Trigger the Triac at the calculated firing angle
  digitalWrite(triacPin, HIGH);
  delayMicroseconds(firingAngle);
  digitalWrite(triacPin, LOW);
  delay(cycles - firingAngle);
}
