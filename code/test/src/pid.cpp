//code for controlling flow meters using pid control using solenoid vlves

float PIDControl(float inputValue, float setpoint, float Kp, float Ki, float Kd) {
  float error, integral = 0, lastError = 0, outputValue;
  
  error = setpoint - inputValue;
  integral += error;
  float derivative = error - lastError;
  outputValue = Kp * error + Ki * integral + Kd * derivative;
  
  lastError = error;

  return outputValue, error;
}