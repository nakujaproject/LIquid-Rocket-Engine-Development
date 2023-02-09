#include <math.h>

float result, dampingRatio, naturalFrequency;
float desiredFlow1, desiredFlow2;


void getDesiredFlow(float res, float dr, float nf) {

  result = res;
  dampingRatio = dr;
  naturalFrequency = nf;

  desiredFlow1 = result / (2 * dampingRatio * naturalFrequency) *
    (1 - sqrt(1 - pow(2 * dampingRatio, 2)) * sin(naturalFrequency * t));
  desiredFlow2 = result / (2 * dampingRatio * naturalFrequency) *
    (1 + sqrt(1 - pow(2 * dampingRatio, 2)) * sin(naturalFrequency * t));
}