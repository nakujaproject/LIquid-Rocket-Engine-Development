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

// Global variables
unsigned long lastRead = 0;
unsigned long lastBlink =0;
unsigned long fanTimer = 0;
float thermocoupleTemp = 0.0;
float DS18B20Temp0 = 0.0;
float DS18B20Temp1 = 0.0;
float lastReadThermocouple = 0.0;
short int triac_delay = 0;
short int constrained_triac_delay = constrain(triac_delay, 0, 6);

void setup(void)
{

  Serial.begin(115200);
  sensor0.begin();
  sensor1.begin();
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  attachInterrupt(ZERO_CROSS_PIN, runPump, CHANGE);
  while(!tests())
  {
    if(millis()-lastBlink >= 500){
      digitalWrite(WHITE_LED,!digitalRead(WHITE_LED));
      lastBlink =millis();
    }
    

  };
 
}
void loop(void)
{

  sensor0.requestTemperatures();
  sensor1.requestTemperatures(); // Send the command to get temperature readings
                                 // Serial.println("DONE");
  /********************************************************************/
  float thermocoupleTemp = thermocouple.readCelsius();
  float DS18B20Temp0 = sensor0.getTempCByIndex(0);
  float DS18B20Temp1 = sensor1.getTempCByIndex(0);

  Serial.print("C = ");
  Serial.println(thermocoupleTemp);
  Serial.print("Temp 1 is: ");
  Serial.println(DS18B20Temp0);
  Serial.print("Temp 2 is: ");
  Serial.println(DS18B20Temp1);
  Serial.print("Triac delay is: ");
  Serial.println(triac_delay);
  runPump();
  checkTemperatureDifference(DS18B20Temp0, DS18B20Temp1, lastReadThermocouple, thermocoupleTemp);
  lastReadThermocouple = thermocoupleTemp;
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
  if(millis()-fanTimer >= 3000){
      digitalWrite(FAN_PIN,!digitalRead(FAN_PIN));
      fanTimer =millis();
    }
  
}

bool tests()
{
  while (!lcdTest())
  {
  }
  while (!thermocoupleTest())
  {
  }
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
  triac_delay -= 1;
}

/**
 * @brief function to decrease flowrate.
 */
void decreaseFlowrate()
{
  triac_delay += 1;
}

/**
 * @brief function to run the pump.
 */
void runPump()
{
  // TO IMPLEMENT RUN PUMP
  digitalWrite(GREEN_LED, HIGH);
  delay(constrained_triac_delay);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(PUMP_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(PUMP_PIN, LOW);
}

void checkTemperatureDifference(float a, float b, float c, float d)
{
  if ((a - b > 10) || (d - a > 10))
  {
    increaseFlowrate();
  }
  else if ((a - b < 0) && (d - a < 0))
  {
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