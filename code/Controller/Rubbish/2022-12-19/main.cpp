#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "max6675.h"
#include "LiquidCrystal_I2C.h"
#include "defs.h"

// Objects
MAX6675 thermocouple(sckPin, csPin, soPin); // create instance object of MAX6675
OneWire oneWire(ONE_WIRE_0);
OneWire oneWire1(ONE_WIRE_1);
DallasTemperature sensor0(&oneWire);
DallasTemperature sensor1(&oneWire1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// function prototypes
bool tests();
bool lcdTest();
bool thermocoupleTest();
bool DS18B20Test();
void print_lcd(float temp1, float temp2, float temp3);
void controlFlowrate(float tempDifference, float wallTemperature);
void runPump();
void checkTemperatureDifference(float a, float b, float c, float d);
void turnOnHeater();
void turnOffHeater();
void turnOnFan();
void turnOffFan();
void fullspeed();

// Global variables
unsigned long lastRead = 0;
unsigned long lastBlink =0;
unsigned long fanTimer = 0;

float lastReadThermocouple = 0.0;
int triac_delay = 2500;
int constrained_triac_delay = constrain(triac_delay, 0, 5000);
short int pump_flag = 0;

float thermocoupleTemp = 0;
float DS18B20Temp0 = 20;
float DS18B20Temp1 = 20;
short int temp_change_flag = 0;

int current_time;
int prev_time;

void setup(void)
{

  Serial.begin(115200);
  sensor0.begin();
  sensor1.begin();
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  while(!tests())
  {
    if(millis()-lastBlink >= 500){
      digitalWrite(WHITE_LED,!digitalRead(WHITE_LED));
      lastBlink =millis();
    }
  };
  delay(setup_time);
  digitalWrite(FAN_PIN, LOW);
  
  attachInterrupt(ZERO_CROSS_PIN, runPump, CHANGE);
  
  pump_flag = 1;
  prev_time = millis();
}




void loop(void)
{

  sensor0.requestTemperatures();
  // sensor1.requestTemperatures(); // Send the command to get temperature readings
                                 // Serial.println("DONE");
  /********************************************************************/
  if (temp_change_flag == 0)
  {
    thermocoupleTemp += 1;
    if(thermocoupleTemp >=50)
    {
      temp_change_flag = 1;
    }
  }
  else
  {
    thermocoupleTemp -= 1;
    if(thermocoupleTemp <=0)
    {
      temp_change_flag = 0;
    }
  }
  
  Serial.print("Thermocouple temp = ");
  Serial.println(thermocoupleTemp);
  Serial.print("Last Thermocouple temp = ");
  Serial.println(lastReadThermocouple);
  Serial.print("Triac delay is: ");
  Serial.println(constrained_triac_delay);
  // runPump();
  current_time = millis();
  checkTemperatureDifference(DS18B20Temp0, DS18B20Temp1, lastReadThermocouple, thermocoupleTemp);
  if(current_time - prev_time >= 2000)
  {
    lastReadThermocouple = thermocoupleTemp;
    prev_time = current_time;
  }
  
  if (thermocoupleTemp < CONTROL_TEMP_VALUE)
  {
    turnOnHeater();
  }
  else if (thermocoupleTemp > CONTROL_TEMP_VALUE + 10)
  {
    turnOffHeater();
  }
  if (millis() - lastRead >= 1000)
  {
    print_lcd(thermocoupleTemp, DS18B20Temp0, DS18B20Temp1);
    lastRead = millis();
  }
  if(millis()-lastBlink >= 100){
      digitalWrite(WHITE_LED,!digitalRead(WHITE_LED));
      lastBlink =millis();
    }
  
}

bool tests()
{
  while (!lcdTest())
  {
  }
  // while (!thermocoupleTest())
  // {
  // }
  while (!DS18B20Test())
  {
  }
  return true;
}

bool lcdTest()
{
  lcd.init();
  lcd.backlight();
  lcd.print("Maya v0.0.1");
  lcd.setCursor(7, 3);
  lcd.print("FYP 18-22");
  delay(1000);
  lcd.clear();
  lcd.print("Starting tests ........");
  delay(1000);
  return true;
}
bool thermocoupleTest()
{
  lcd.clear();
  lcd.print("Running thermocouple test!!!!");
  int count = 0;
  while (count < 10)
  {
    delay(100);
    if (thermocouple.readCelsius() > 0 && thermocouple.readCelsius() < 50)
    {
      count++;
    }
    else
    {
      continue;
    }
  }
  lcd.clear();
  lcd.println("Thermocouple test complete !!!!!!");
  delay(1000);
  return true;
}

bool DS18B20Test()
{
  lcd.clear();
  lcd.println("Running DS18B20 test !!!!!!!!!!");
  delay(1000);
  int count = 0;
  while (count < 10)
  {
    delay(100);
    if (sensor0.getTempCByIndex(0) > 0 && sensor0.getTempCByIndex(0) < 50 && sensor1.getTempCByIndex(0) > 0 && sensor1.getTempCByIndex(0))
    {
      count++;
    }
    else
    {
      continue;
    }
  }
  lcd.clear();
  lcd.println("DS18B20 test complete.....!!!!!");
  delay(1000);
  return true;
}

void print_lcd(float temp1, float temp2, float temp3)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thermocouple: ");
  lcd.print(temp1);
  lcd.setCursor(0, 1);
  lcd.print("Probe 1: ");
  lcd.print(temp2);
  lcd.setCursor(0, 2);
  lcd.print("Probe 2: ");
  lcd.print(temp3);
}

/**
 * @brief function to increase flowrate according to temperature
 */
void increaseFlowrate()
{
  // implement increased flowrate
triac_delay -= 500;
constrained_triac_delay = constrain(triac_delay, 0, 5000);
 
}

/**
 * @brief function to decrease flowrate.
 */
void decreaseFlowrate()
{
  triac_delay += 500;
  constrained_triac_delay = constrain(triac_delay, 0, 5000);
  
}

/**
 * @brief function to run the pump.
 */
void runPump()
{
  // TO IMPLEMENT RUN PUMP
  if(pump_flag == 0)
  {
    constrained_triac_delay = constrained_triac_delay;
  }
  else
  {
    delayMicroseconds(constrained_triac_delay);
    digitalWrite(PUMP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PUMP_PIN, LOW);
  }

}

void checkTemperatureDifference(float a, float b, float c, float d)
{
  if (d - c >= 5)
  {
    Serial.println("Mike");
    increaseFlowrate();
  }
  else if (c - d >= 5)
  {
    Serial.println("Mike");
    decreaseFlowrate();
  }
}
/**
 * @brief function to turn on the heater
 */
void turnOnHeater()
{
  digitalWrite(HEATER_PIN, HIGH);
}

/**
 * @brief function to turn off the heater
 */
void turnOffHeater()
{
  digitalWrite(HEATER_PIN, LOW);
}

/**
 * @brief Function to turn on the fan
 */
void turnOnFan()
{
  digitalWrite(FAN_PIN, HIGH);
}

/**
 * @brief Function to turn the fan off
 * */
void turnOffFan()
{
  digitalWrite(FAN_PIN, LOW);
}

// void full_speed()
// {
//   triac_delay = 0;
//   pump_flag = 0;
//   for(int full_speed = 0; full_speed <= 6; full_speed++)
//   {
//     delay(1000);
//     digitalWrite(PUMP_PIN, HIGH);
//     Serial.print("Full speed temp. data is : ");
//     Serial.println("last value");
//   }
//   pump_flag = 1;
// }