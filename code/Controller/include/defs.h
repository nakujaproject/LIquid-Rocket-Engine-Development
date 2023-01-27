#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <Arduino.h>

#define soPin PA6  // SO=Serial Out
#define csPin PA7  // CS = chip select CS pin
#define sckPin PA5 // SCK = Serial Clock pi
#define ONE_WIRE_0 PB12 // One Wire Probe 1
#define ONE_WIRE_1 PB13 // One wire Probe 2

#define MAX_WALL_TEMP 400.0
#define MAX_TEMP_DIFF 5.0
#define CONTROL_TEMP_VALUE 20.0

#define ONBOARD_LED PC13
#define FLOWRATE_PIN PB3
#define PUMP_PIN PB14
#define ZERO_CROSS_PIN PB8
#define HEATER_PIN PB10
#define FAN_PIN PB4
#define GREEN_LED PB15
#define WHITE_LED PA9
#define setup_time 5000

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {PA0, PA2, PA3, PA4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {PB9, PC13, PC14, PC15}; //connect to the column pinouts of the keypad


#endif