#include <TimerOne.h>

const int triacPin = 12; // Triac output pin
const int zeroCrossPin = 2; // Zero-cross detection pin
int firingAngle = 8; // Adjustable firing angle
int desiredFlow = 50; // Desired flow rate in percent
int actualFlow = 0; // Actual flow rate measured by sensor
int error = 0; // Error between desired and actual flow rate

void setup() {
  pinMode(triacPin, OUTPUT);
  pinMode(zeroCrossPin, INPUT);
  Timer1.initialize(100000); // Set Timer1 interval to 100ms
  Timer1.attachInterrupt(fireTriac); // Attach interrupt to fireTriac function
}

void loop() {
  actualFlow = readFlowSensor(); // Read actual flow rate from sensor
  error = desiredFlow - actualFlow; // Calculate error between desired and actual flow rate
  firingAngle = pidController(error); // Update firing angle using pidController function
}

void fireTriac() {
  static boolean zeroCrossFlag = LOW;
  if (digitalRead(zeroCrossPin) == HIGH) { // Check for zero-crossing
    zeroCrossFlag = LOW;
  } else if (zeroCrossFlag == LOW) { // At the next zero-crossing
    digitalWrite(triacPin, LOW); // Turn off Triac
    zeroCrossFlag = HIGH;
    Timer1.setPwmDuty(firingAngle); // Set firing angle
    digitalWrite(triacPin, HIGH); // Turn on Triac
  }
}

// Example PID Controller function
int pidController(int error) {
  // Proportional, Integral and Derivative terms calculation
  int pTerm = /* proportional gain * error */;
  int iTerm = /* integral gain * integral of error */;
  int dTerm = /* derivative gain * derivative of error */;
  
  // Return firing angle
  return /* sum of P, I and D terms */;
}

// Example function to read flow sensor
int readFlowSensor() {
  // Code to read flow sensor and return flow rate in percent
  return 0;
}