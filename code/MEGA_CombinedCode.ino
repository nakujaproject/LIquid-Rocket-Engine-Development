#include <ArduinoJson.h>

const int pressurePin_GN2 = A0; // 10V Sensor
const int pressurePin_GOX = A1; // 10V Sensor
const int pressurePin_3 = A2; // 5V Sensor
const int pressurePin_4 = A3; // 5V Sensor
const int pressurePin_5 = A4; // 5V Sensor
int sv_readPin = A7; // Used to test the 3 SVs using the voltage divider circuit. Value range 30<x<75 for the curcuit

int flowPin_FUEL = 2;
int flowPin_GN2 = 3;
int flowPin_GOX = 18;
// Arduino Nano interrupt pins are only 2 & 3.
// Arduino Mega interrupt pins are 2,3,18,19,20,21 (20&21 aren't available to use for interrupts while they are used for I2C communication

int svPin_FUEL = 6;
int svPin_GN2 = 7;
int svPin_GOX = 8;

// Variables
int SerialInt; // Serial input from ESP32 to Mega
int SensorSampleRate = 100; // 100ms -> 10Hz (This is a stable rate to loop the sensors. Higher f gives less accurate Q values)
float flowrateFUEL, flowrateGN2, flowrateGOX, amountFUEL, amountGN2, amountGOX;
float pressureVal_GN2, pressureVal_GOX, pressureVal_3, pressureVal_4, pressureVal_5;
float pressureGN2, pressureGOX, pressure3, pressure4, pressure5;
int Int_SolenoidState = 0;

volatile long pulseFUEL, pulseGN2, pulseGOX;
long int signalFUEL, signalGN2, signalGOX;
long int lastpulseFUEL, lastpulseGN2, lastpulseGOX;
unsigned long currentTime = 0;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;

// Boolean Variables for Sensor Test code
bool PressTotalBool=false; bool FlowTotalBool=false; bool SVTotalBool=false; bool FinalBool=false;
bool TestBool=false; // Will be turned to true to run the sensor test functions
bool GetDataBool=false; // Will be turned to true to run the sensor test functions

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(pressurePin_GN2, INPUT);
  pinMode(pressurePin_GOX, INPUT);
  pinMode(pressurePin_3, INPUT); // INPUT_PULLUP gives some errors. It raises the 1bar from 100 to 120. Probably Sensor has internal resistance.
  pinMode(pressurePin_4, INPUT);
  pinMode(pressurePin_5, INPUT);
  pinMode(sv_readPin, INPUT);
  
  pinMode(flowPin_FUEL, INPUT);
  pinMode(flowPin_GN2, INPUT);
  pinMode(flowPin_GOX, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(svPin_FUEL, OUTPUT);
  pinMode(svPin_GN2, OUTPUT);
  pinMode(svPin_GOX, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(flowPin_FUEL), Function_increasePulseFUEL, RISING);
  attachInterrupt(digitalPinToInterrupt(flowPin_GN2), Function_increasePulseGN2, RISING);
  attachInterrupt(digitalPinToInterrupt(flowPin_GOX), Function_increasePulseGOX, RISING);

  Serial.println("Ready: ");
  Serial.println();
}


void loop() {
  if(Serial.available()>0){
    SerialInt = Serial.parseInt(SKIP_ALL, '\n');

    // Ensure MEGA is working
    if(SerialInt==0){
      // Create a JSON object
      StaticJsonDocument<30> doc;
      doc["MEGA State"] = true;       
      // Serialize the JSON object to a string and send it over serial
      String jsonStr;
      serializeJson(doc, jsonStr);Serial.println(jsonStr);Serial.println();
      
    }

    // Run Sensor Test Code
    else if(SerialInt==123){
      TestBool=true;
      SensorsTest();
      
      StaticJsonDocument<15> doc;
      doc["FinalBool"] = FinalBool;
      String jsonStr;
      serializeJson(doc, jsonStr);Serial.println(jsonStr);Serial.println();
      TestBool=false;
      
    }
    
    // Get sensor data
    else if(SerialInt==456){
      GetDataBool=true;
    }

    // Stop getting sensor data
    else if(SerialInt==789){
      digitalWrite(LED_BUILTIN, LOW);
      GetDataBool=false;
    }
    
    // Control SVs
    else if(SerialInt>=0 && SerialInt<=7){
      Function_solenoids(SerialInt);
    }
    else{
      Serial.println("Invalid command.");
    }    
  }

  if(GetDataBool==true){
    SensorData();
      
    // Create a JSON object
    StaticJsonDocument<150> doc;
    doc["SVState"] = Int_SolenoidState;
    doc["Q FUEL"] = flowrateFUEL;
    doc["Q GN2"] = flowrateGN2;
    doc["Q GOX"] = flowrateGOX;
    doc["P GN2"] = pressureGN2;
    doc["P GOX"] = pressureGOX;
    doc["P 3"] = pressure3;
    doc["P 4"] = pressure4;
    doc["P 5"] = pressure5;
    // Serialize the JSON object to a string and send it over serial
    String jsonStr;
    serializeJson(doc, jsonStr);
    Serial.println(jsonStr);
    Serial.println();
  }
}




//---------------------------------   FUNCTIONS   -------------------------------------------------------------------------------------------


// *****************************************************************************************
//                                         1 A. SENSORS TEST
// 1. Compilation of the 3 sensor test functions. This function is called at the loop.
int SensorsTest(){
  int x=0;
  FlowTotalBool=false; PressTotalBool=false; SVTotalBool=false; FinalBool=false;
  
  while(x<3 && FinalBool!=true){ // x<5 && FinalBool!=true
    if((millis() - lastTime1) >= SensorSampleRate){
      
      if(FlowTotalBool!=true){Function_flowmeters(pulseFUEL, pulseGN2, pulseGOX);}
      if(PressTotalBool!=true){Function_pressures();}
      if(SVTotalBool!=true){Test_SolenoidValves();}
      
      x++;
      lastTime1 = millis();
    }   
    
    if(FlowTotalBool==true && PressTotalBool==true && SVTotalBool==true){ //  FlowTotalBool==true && PressTotalBool==true && SVTotalBool==true
      FinalBool=true;
      TestBool=false;
    }
    if(x>2 && FinalBool!=true){
      Serial.println("Sensor Test aborted after 3 Tests. Faulty sensor(s)");
    }
  }
}


// ****************************************************************************************************
//                                          1 B. SENSORS DATA
//
int SensorData(){
  if((millis()-lastTime2) >= SensorSampleRate){
    digitalWrite(LED_BUILTIN, HIGH);
    
    Function_flowmeters(pulseFUEL, pulseGN2, pulseGOX);
    Function_pressures();

    lastTime2 = millis();
  }else{
    digitalWrite(LED_BUILTIN, LOW);
  }
}



// *****************************************************************************************************
//                                      2 A. FLOW SENSORS
// 1. Pulse increase upon interrupts
void Function_increasePulseFUEL(){
  pulseFUEL++;}
void Function_increasePulseGN2(){
  pulseGN2++;}
void Function_increasePulseGOX(){
  pulseGOX++;}


// 2. Flowrate and Amount (Incorporates both sensor testing and data taking)
int Function_flowmeters(int pulFUEL, int pulGN2, int pulGOX){
  FlowTotalBool = false;
  // 
  float Calibration_factor = SensorSampleRate*(0.00593334);
  // 356 pulses / (1L/min), thus 356 pulses/60 sec, thus 5.9334 pulses/sec, 0.59334 pulses for 100ms, 0.29667 for 50ms, etc.
  
  signalFUEL = pulseFUEL - lastpulseFUEL; signalGN2 = pulseGN2 - lastpulseGN2; signalGOX = pulseGOX - lastpulseGOX;
  
  flowrateFUEL = signalFUEL/Calibration_factor; flowrateGN2 = signalGN2/Calibration_factor; flowrateGOX = signalGOX/Calibration_factor;

  // IF condition to test sensors
  if(TestBool==true){
    if(flowrateFUEL==0 && flowrateGN2==0 && flowrateGOX==0){
      FlowTotalBool = true;Serial.println("Flow sensors working: ");
    }else{
      FlowTotalBool = false;
      if(flowrateFUEL!=0){Serial.print("Flow sensor 1 error! ");Serial.println(flowrateFUEL);}
      if(flowrateGN2!=0){Serial.print("Flow sensor 2 error! ");Serial.println(flowrateGN2);}
      if(flowrateGOX!=0){Serial.print("Flow sensor 3 error! ");Serial.println(flowrateGOX);}
    }
    lastpulseFUEL=pulseFUEL; lastpulseGN2=pulseGN2; lastpulseGOX=pulseGOX;
    return;
  } 
  lastpulseFUEL = pulseFUEL; lastpulseGN2 = pulseGN2; lastpulseGOX = pulseGOX;
  amountFUEL = pulFUEL/356.00; amountGN2 = pulGN2/356.00; amountGOX = pulGOX/356.00;
}


//*******************************************************************************************
//                                    2 B. PRESSURE SENSORS
// 3. Pressure Function (Incorporates both sensor testing and data taking)
int Function_pressures(){
  //  THE PRESSURE VALUE RANGES WILL BE DETERMINED BY THE INITIAL CONDITIONS
  PressTotalBool = false;

  // 10V Pressure Sensors may not need to be tested.
  // This code is only for the 5V sensors.
  pressureVal_3 = analogRead(pressurePin_3);
  pressureVal_4 = analogRead(pressurePin_4);
  pressureVal_5 = analogRead(pressurePin_5);

  if(TestBool==true){
    if(pressureVal_3<150 && pressureVal_4<150 && pressureVal_5<150){
      PressTotalBool = true;
      Serial.println("Pressure Sensors working: ");
    }else if(pressureVal_3>150 || pressureVal_4>150 || pressureVal_5>150){
      if(pressureVal_3>150){Serial.println("Pressure Sensor 3 error.");}
      if(pressureVal_4>150){Serial.println("Pressure Sensor 4 error.");}
      if(pressureVal_5>150){Serial.println("Pressure Sensor 5 error.");}
    }else{
      Serial.println("Pressure Code Error!!!");
    }
    return;
  }

  pressureVal_GN2 = analogRead(pressurePin_GN2);
  pressureVal_GOX = analogRead(pressurePin_GOX);

  // 10V Pressure Sensors (Only 2 such sensors)
  // The conversion came from the 2 quad eqns below:
  // 1bar = m(0)+c , 10bars = m(1023)+c.     Thus, c = 1 & m = 0.0087977
  // (Voltage divider divides 10V into 5V which Arduino translates to 1023. Thus 1023 means 10bars)
  pressureGN2 = ((pressureVal_GN2)*0.0087977) + 1.013;
  pressureGOX = ((pressureVal_GOX)*0.0087977) + 1.013;
  
  // 5V Sensors (Upto 4 such sensors asofnow)
  // The conversion came from the 2 quad eqns below:
  // 1bar = m(0.5V)+c , 12bars = m(4.5V)+c. (From Data sheet of Pixel Electric Sensors).
  // Thus, c = -0.375 & m = 0.013441
  pressure3 = ((pressureVal_3)*0.013441) - 0.345; // To give 1.01bars
  pressure4 = ((pressureVal_4)*0.013441) - 0.345;
  pressure5 = ((pressureVal_5)*0.013441) - 0.345;
    
}

// *****************************************************************************************
//                                        2 C. SOLENOID VALVES TESTING

// Test SolenoidValves (Only for testing the SVs)
int Test_SolenoidValves(){
  int sv_val=0;
  int x=0; int y=0; int z=0;
  bool SVFUEL = false; bool SVGN2 = false; bool SVGOX = false;

  // SV FUEL
  while(SVFUEL==false && SVTotalBool==false){
    digitalWrite(svPin_FUEL, HIGH);
    sv_val = analogRead(sv_readPin);
    if(sv_val>30 && sv_val<70){
      x++;
      if(x>2){
        SVFUEL=true;
        digitalWrite(svPin_FUEL, LOW);
      }
    }
  }
  // SV GN2
  while(SVGN2==false && SVFUEL==true && SVTotalBool==false){
    digitalWrite(svPin_GN2, HIGH);
    sv_val = analogRead(sv_readPin);
    if(sv_val>30 && sv_val<70){
      y++;
      if(y>2){
        SVGN2=true;
        digitalWrite(svPin_GN2, LOW);
      }
    }
  }
  // SV GOX
  while(SVGOX==false && SVGN2==true && SVTotalBool==false){
    digitalWrite(svPin_GOX, HIGH);
    sv_val = analogRead(sv_readPin);
    if(sv_val>30 && sv_val<70){
      z++;
      if(z>2){
        SVGOX=true;
        digitalWrite(svPin_GOX, LOW);
      }
    }
  }
  if(SVFUEL==true && SVGN2==true && SVGOX==true){
    SVTotalBool=true;
    Serial.println("SVs working!");
  }
}

// *****************************************************************************************
//                                       2 D. SOLENOID VALVES ACTUATING

// Solenoid Valves (Only for actuating Solenoids)
int Function_solenoids(int digit){

// ****************************************************
// Consider using boolean values as inputs to turn ON or OFF.

  switch (digit) {
    case 1:
      digitalWrite(svPin_GOX, HIGH);
      Int_SolenoidState = 1;
      break;
     case 2:
      digitalWrite(svPin_GN2, HIGH);
      Int_SolenoidState = 2;
      break;
     case 3:
      digitalWrite(svPin_FUEL, HIGH);
      Int_SolenoidState = 3;
      break;
     case 4:
      digitalWrite(svPin_GOX, LOW);
      Int_SolenoidState = 4;
      break;
     case 5:
      digitalWrite(svPin_GN2, LOW);
      Int_SolenoidState = 5;
      break;
     case 6:
      digitalWrite(svPin_FUEL, LOW);
      Int_SolenoidState = 6;
      break;
     case 7:
      digitalWrite(svPin_GOX, LOW);
      digitalWrite(svPin_FUEL, LOW);
      digitalWrite(svPin_GN2, LOW);
      Int_SolenoidState = 7;
      break; 
     default:
      Serial.println("Beyond scope");
      break;
  }  
}
