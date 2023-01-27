#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "max6675.h"
#include "LiquidCrystal_I2C.h"
#include "defs.h"
#include "Keypad.h"


// Objects
MAX6675 thermocouple(sckPin, csPin, soPin); // create instance object of MAX6675
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// function prototypes
bool tests();
bool lcdTest();
bool thermocoupleTest();
void print_lcd(float temp1, float temp2, float temp3, float flowrate);
void runPump();
void turnOnHeater();
void turnOffHeater();
void turnOnFan();
void turnOffFan();
void checkflowrate();
void increaseFlowrate();
void keypadEvent(KeypadEvent key);

unsigned long lastBlink =0;

// for automation of circuit
int pinInput = 2; // Assign the pin input to a variable
// int flagValue = 0;


int triac_delay = 2500;
int constrained_triac_delay = constrain(triac_delay, 0, 5000);
short int pump_flag = 0;

float thermocoupleTemp = 0.0;
float lastReadThermocouple = 0.0;
float lastContThermocouple = 0.0;

short int temp_change_flag = 0;
float flowrate =0.0;

int initial_time;
int current_time;

int f_prev_time = 0;
int f_current_time = 0;

int prev_temp_time = 0;
int current_temp_time;

int prev_time;
int var1 = 0;
int init_const_time_flag = 0;
int init_dyn_time_flag = 0;



int fx;
int fy;
float ftime;
float f_frequency;
float ftotal;
float f_flm;
float f_fls;
float f_flh;


void setup(void)
{

  Serial.begin(115200);
 // keypad.addEventListener(keypadEvent);
  turnOffFan();
  pinMode(pinInput, INPUT); // Set the pin to input mode
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(FLOWRATE_PIN,INPUT);
  pinMode(PC13,OUTPUT);
  //digitalWrite(PC13,LOW);
  
  while(!tests())
  {
    if(millis()-lastBlink >= 500)
    {
      digitalWrite(WHITE_LED,!digitalRead(WHITE_LED));
      lastBlink =millis();
    }
  };

  delay(setup_time);
  attachInterrupt(ZERO_CROSS_PIN, runPump, CHANGE);
  prev_time = millis();
}




void loop(void)
{
  // input from aduino mega
  int pinState = digitalRead(pinInput);

  if (pinState == HIGH) {
    thermocoupleTemp = thermocouple.readCelsius();
    f_current_time = millis();
    if(f_current_time - f_prev_time >= 1000)
    {
      checkflowrate();
      f_prev_time = f_current_time;
    }
    //Serial.println(f_flm);
    Serial.println(constrained_triac_delay);
    char key = keypad.getKey();
    //Serial.println(key);
    switch(var1){
      case 0:
        pump_flag = 0;
        init_const_time_flag = 0;
        init_dyn_time_flag = 0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("I_STATE   ");
        lcd.print("LCT: ");
        lcd.print(lastContThermocouple);
        lcd.setCursor(0,1);
        lcd.print("Temp: ");
        lcd.print(thermocoupleTemp);
        lcd.print("deg");
        lcd.setCursor(0,2);
        lcd.print("Q: ");
        lcd.print(f_flm);
        lcd.print(" l/m");
        lcd.setCursor(0,3);
        lcd.print("CTEST: 1 :: DTEST: 2");
        turnOffFan();
        if(key == '1')
        {
          var1 = 1;
        }
        if(key == '2')
        {
          var1 = 2;
        }
        // if(keypad_pressed)
        // {
          //   var1 = 2;
          // }
        break;
      case 1:
        turnOnFan();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("C_STATE  ");
        lcd.print("LCT: ");
        lcd.print(lastContThermocouple);
        lcd.setCursor(0,1);
        lcd.print("Temp: ");
        lcd.print(thermocoupleTemp);
        lcd.print("deg");
        lcd.setCursor(0,2);
        lcd.print("Q: ");
        lcd.print(f_flm);
        lcd.print(" l/m");
        lcd.setCursor(0,3);
        lcd.print("CANCEL: C");
        if(init_const_time_flag == 0)
        {
          initial_time = millis();
          init_const_time_flag = 1;
        }
        triac_delay = 2500;
        constrained_triac_delay = constrain(triac_delay, 0, 5000);
        current_time = millis();
        if(current_time - initial_time <= 110000)
        {
          pump_flag = 1;
          if((current_time - initial_time >= 20000) && (current_time - initial_time <= 50000))
          {
            turnOnHeater();
            }
          else
          {
            turnOffHeater();
            }
        }
        else
        {
          pump_flag = 0; //pump off
          var1 = 0; //will go to case 0
          lastContThermocouple = thermocoupleTemp; 
        }
        if(key == 'C')
        {
          var1 = 0;
        }
        break;
        
      case 2:
        turnOnFan();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("D_STATE  ");
        lcd.print("LCT: ");
        lcd.print(lastContThermocouple);
        lcd.setCursor(0,1);
        lcd.print("Temp: ");
        lcd.print(thermocoupleTemp);
        lcd.print("deg");
        lcd.setCursor(0,2);
        lcd.print("Q: ");
        lcd.print(f_flm);
        lcd.print(" l/m");
        lcd.setCursor(0,3);
        lcd.print("CANCEL: C");
        if(init_dyn_time_flag == 0)
        {
          initial_time = millis();
          init_dyn_time_flag = 1;
        }
        current_time = millis();
        if((current_time - initial_time <= 110000))
        {
          pump_flag = 1;
          if(current_time - initial_time >= 20000)
          {
            if(current_time - initial_time <= 50000)
            {
              turnOnHeater();
            }
            else
            {
              turnOffHeater();
            }
            current_temp_time = millis();
            if(current_temp_time - prev_temp_time >= 3000)
            {
              lastReadThermocouple = thermocoupleTemp;
              prev_temp_time = current_temp_time;
            }
            if(thermocoupleTemp - lastReadThermocouple >=0.2)
            {
              increaseFlowrate();
            }
          }
        }
        else
        {
          pump_flag = 0;
          var1 = 0;
          lastContThermocouple = thermocoupleTemp;
        }
        if(key == 'C')
        {
          var1 = 0;
        }
        break;
        default:
        var1 = 0;
        break;
    }
    // if (temp_change_flag == 0)
    // {
    //   thermocoupleTemp += 1;
    //   if(thermocoupleTemp >=25)
    //   {
    //     temp_change_flag = 1;
    //   }
    // }
    // else
    // {
    //   thermocoupleTemp -= 1;
    //   if(thermocoupleTemp <=0)
    //   {
    //     temp_change_flag = 0;
    //   }
    // }
  
    // Serial.print("Thermocouple temp = ");
    // Serial.println(thermocoupleTemp);
    // Serial.print("Last Thermocouple temp = ");
    // Serial.println(lastReadThermocouple);
    // Serial.print("Triac delay is: ");
    // Serial.println(constrained_triac_delay);
    //current_time = millis();
  }
  else{
    // do nothing.
  }
}

bool tests(){
  while (!lcdTest())
  { //we'll see wht will go here
  }
  while (!thermocoupleTest())
  {  // for here too
  }
  return true;
  }

bool lcdTest(){
  lcd.init();
  lcd.backlight();
  lcd.print("Maya v0.0.2");
  lcd.setCursor(7, 3);
  lcd.print("internship-2023");
  delay(1000);
  lcd.clear();
  lcd.print("Starting tests ........");
  delay(1000);
  return true;
}

bool thermocoupleTest(){
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

void print_lcd(float temp1, float temp2, float temp3, float flowrate){
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
  lcd.setCursor(0,3);
  lcd.print("Flowrate:");
  lcd.print(flowrate);
  lcd.print("L/m");
}

/**
 * @brief function to increase flowrate according to temperature
 */
void increaseFlowrate(){
  // implement increased flowrate
  triac_delay -= 500;
  constrained_triac_delay = constrain(triac_delay, 0, 5000);
}
  
/**
 * * @brief function to decrease flowrate.
 * */
void decreaseFlowrate()
{
  triac_delay += 500;
  constrained_triac_delay = constrain(triac_delay, 0, 5000);
}
  
/**
 * * @brief function to run the pump.
 * */
void runPump(){
  // TO IMPLEMENT RUN PUMP
  if(pump_flag == 0)
  {
    digitalWrite(PUMP_PIN, LOW);
  }
  else
  {
    delayMicroseconds(constrained_triac_delay);
    digitalWrite(PUMP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PUMP_PIN, LOW);
  }
}
  
void checkflowrate(){
  fx = pulseIn(FLOWRATE_PIN, HIGH);
  fy = pulseIn(FLOWRATE_PIN, LOW);
  ftime = fx + fy;
  f_frequency = 1000000/ftime;
  f_flm = f_frequency/7.5;
  f_fls = f_flm/60.0;
  f_flh = f_flm*60;
}
  
/**
 * * @brief function to turn on the heater
 * */
void turnOnHeater(){
  digitalWrite(HEATER_PIN, HIGH);
}
  
/**
 * * @brief function to turn off the heater
 * */
void turnOffHeater()
{
  digitalWrite(HEATER_PIN, LOW);
}
  
/**
 * * @brief Function to turn on the fan
 * */
void turnOnFan(){
  digitalWrite(FAN_PIN, LOW);
}
  
/**
 * * @brief Function to turn the fan off
 * * */
void turnOffFan(){
  digitalWrite(FAN_PIN, HIGH);
}