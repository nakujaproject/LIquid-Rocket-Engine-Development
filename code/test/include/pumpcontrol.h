#include <math.h>

float result, dampingRatio, naturalFrequency;
float desiredPumpFlow1, desiredPumpFlow2;


void getDesiredPumpFlow(float res, float dr, float nf) {

  result = res;
  dampingRatio = dr;
  naturalFrequency = nf;

  desiredPumpFlow1 = result / (2 * dampingRatio * naturalFrequency) *
    (1 - sqrt(1 - pow(2 * dampingRatio, 2)) * sin(naturalFrequency * t));
  desiredPumpFlow2 = result / (2 * dampingRatio * naturalFrequency) *
    (1 + sqrt(1 - pow(2 * dampingRatio, 2)) * sin(naturalFrequency * t));
}