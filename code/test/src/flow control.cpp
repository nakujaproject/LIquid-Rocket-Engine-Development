#include <iostream>
#include <Arduino.h>
#include <Stepper.h>

unsigned long lastTime = 0;
int flowPin = A0;
int flowRate = 0;

double Kp, Ki, Kd;
double error, lastError, integral = 0, derivative;
double setPoint;

//const float Kp = /*Proportional gain value*/;
//const float Ki = /*Integral gain value*/;
//const float Kd = /*Derivative gain value*/;

float prevError = 0;
float integral = 0;

const int stepsPerRevolution = 200;
Stepper solenoidStepper(stepsPerRevolution, 8, 9, 10, 11);
const float maxFlowRate = /* Maximum flow rate value that can be taken by the flow sensor */;

// Function to set the propellant output based on the flow rate
void setPropellantOutput(float flowRate) {
  // Propellant/Oxidizer flow rate ratio
  float propOxRatio = 1.5;
  // Calculate the desired propellant output
  float desiredPropOutput = flowRate / propOxRatio;
  // Use the desired propellant output to control the actuator
  controlActuator(desiredPropOutput, 24); //enter desired pin number here
}

// Function to set the oxidizer output based on the flow rate
void setOxidizerOutput(float flowRate) {
  // Propellant/Oxidizer flow rate ratio
  float propOxRatio = 1.5;
  // get desired propellnt output from user or function
  float desiredPropOutput = input();
  // Calculate the desired oxidizer output
  float desiredOxOutput = flowRate - desiredPropOutput * propOxRatio;
  // Use the desired oxidizer output to control the actuator
  controlActuator(desiredOxOutput, 23);  //enter desired pin number here
}

double pidController(double input) {
  error = setPoint - input;
  integral += error;
  derivative = error - lastError;
  lastError = error;
  return Kp * error + Ki * integral + Kd * derivative;
}

float input() {
  float userInput;
  std::cout << "Please enter a float value: ";
  std::cin >> userInput;
  std::cout << "You entered: " << userInput << std::endl;
  return userInput;
}

int getFlowRateFromSensor(int pin) {
  int flowSensorValue = analogRead(pin);
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= 1000) { // calculate flow rate once per second
    flowRate = map(flowSensorValue, 0, 1023, 0, 100); // adjust this range based on sensor specifications
    lastTime = currentTime;
  }
  return flowRate;
}

void controlActuator(float desiredFlowRate, int pin) {
  // Read the current flow rate value from the flow sensor
  float currentFlowRate = getFlowRateFromSensor(pin);
  float currentStepperPosition = map(currentFlowRate, 0, maxFlowRate, 0, stepsPerRevolution);
  
  // Map the desired oxidizer output value to the number of steps to take.
  float maxFlowRate = /* max possible value that can be taken by the flow sensor */;
  float mappedDesiredStep = map(desiredFlowRate, 0, maxFlowRate, 0, stepsPerRevolution);
  int stepsToTake = round(mappedDesiredStep - currentStepperPosition);
  
  // Calculate the error
  float error = desiredFlowRate - currentFlowRate;
  
  // Update integral
  integral += error;
  
  // Calculate the derivative
  float derivative = error - prevError;
  
  // Calculate the PID output
  float output = Kp * error + Ki * integral + Kd * derivative;
  
  // Update previous error
  prevError = error;
  
  // Control the actuator
  stepsToTake = pidController(output);
  
  // Print the number of steps to take
  Serial.print("Number of steps to take: ");
  Serial.println(stepsToTake);
  
  // Rotate the stepper motor to the desired position
  solenoidStepper.step(stepsToTake);
}