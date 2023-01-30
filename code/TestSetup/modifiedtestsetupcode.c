#include <Wire.h>
#include <LabVIEW_API.h>

#define OXYGENFLOWSENSOR  27
#define NITROGENFLOWSENSOR  35

const int OxygenpressureInput = 32;
const int NitrogenpressureInput = 33;
int OxygensolenoidPin = 17;
int NitrogensolenoidPin = 16;

float OxygenpressureValue = 0;
float NitrogenpressureValue = 0;
bool OxygensolenoidState= false;
bool NitrogensolenoidState= false;

long OxygencurrentMillis = 0;
long OxygenpreviousMillis = 0;
long NitrogencurrentMillis = 0;
long NitrogenpreviousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte O_pulseCount;
volatile byte N_pulseCount;
byte O_pulse1Sec = 0;
byte N_pulse1Sec = 0;
float OxygenflowRate;
float NitrogenflowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter1()
{
  O_pulseCount++;
  
}
void IRAM_ATTR pulseCounter2()
{
   N_pulseCount++;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(OxygenpressureInput, INPUT);
    pinMode(OxygensolenoidPin, OUTPUT);
    digitalWrite(OxygensolenoidPin,LOW);
    pinMode(OXYGENFLOWSENSOR, INPUT_PULLUP);
    pinMode(NitrogenpressureInput, INPUT);
    pinMode(NitrogensolenoidPin, OUTPUT);
    digitalWrite(NitrogensolenoidPin,LOW);
    pinMode(NITROGENFLOWSENSOR, INPUT_PULLUP);
    
    O_pulseCount = 0;
    N_pulseCount = 0;
    OxygenflowRate = 0.0;
    NitrogenflowRate = 0.0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    OxygenpreviousMillis = 0;
    NitrogenpreviousMillis = 0;
    
    attachInterrupt(digitalPinToInterrupt(OXYGENFLOWSENSOR), pulseCounter1, FALLING);
    attachInterrupt(digitalPinToInterrupt(NITROGENFLOWSENSOR), pulseCounter2, FALLING);
}

void loop() {
   OxygenpressureValue = analogRead(OxygenpressureInput);
   NitrogenpressureValue = analogRead(NitrogenpressureInput);
   OxygenoutputValue = map(OxygenpressureValue, 0, 4095, 0, 10);
  NitrogenoutputValue = map(NitrogenpressureValue, 0, 4095, 0, 10);
  Serial.print("Pressure output value = ");
  Serial.print("O2");
  Serial.println(OxygenoutputValue);
  Serial.print("N2");
  Serial.println(NitrogenoutputValue);
  //Serial.print(" bar"); 
  if (OxygensolenoidState == false && OxygenoutputValue >= 3){
    digitalWrite(OxygensolenoidPin,HIGH);
    OxygensolenoidState = true;
  }
  if(OxygensolenoidState == true && OxygenoutputValue <= 2){
    digitalWrite(OxygensolenoidPin,LOW);
    OxygensolenoidState = false;
    }
    
    if (NitrogensolenoidState == false && NitrogenoutputValue >= 3){
    digitalWrite(NitrogensolenoidPin,HIGH);
    NitrogensolenoidState = true;
  }
  if(NitrogensolenoidState == true && NitrogenoutputValue <= 2){
    digitalWrite(NitrogensolenoidPin,LOW);
    NitrogensolenoidState = false;
    }

     OxygencurrentMillis = millis();
     NitrogencurrentMillis = millis();
  
    if (OxygencurrentMillis - OxygenpreviousMillis > interval) {
    O_pulse1Sec = O_pulseCount;
    O_pulseCount = 0; 
    OxygenflowRate = ((1000.0/ (millis() - OxygenpreviousMillis)) * O_pulse1Sec ) / calibrationFactor;
    
    OxygenpreviousMillis = millis();
    delay(1000);
    Serial.print("Flow rate: ");
    Serial.print(int(OxygenflowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    delay(5000);}
    
   if (NitrogencurrentMillis - NitrogenpreviousMillis > interval) {
    N_pulse1Sec = N_pulseCount;
    N_pulseCount = 0; 
    
    NitrogenflowRate = ((1000.0/ (millis() - NitrogenpreviousMillis)) * N_pulse1Sec) / calibrationFactor;
    NitrogenpreviousMillis = millis();
    delay(1000);
    Serial.print("Flow rate: ");
    Serial.print(int(NitrogenflowRate));
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    
    //In the loop function, after processing the data, use the LabVIEW API to send it to LabVIEW
    LabVIEW_API_SendData("OxygenpressureValue", OxygenpressureValue);
    LabVIEW_API_SendData("NitrogenpressureValue", NitrogenpressureValue);

   }

}
