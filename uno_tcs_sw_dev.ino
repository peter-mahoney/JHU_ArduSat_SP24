// ArduSat Arduino Uno Thermal Subsystem SW
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include "config.h"
#include <Adafruit_INA219.h>
#include <AHT20.h>

#define ADDR_TCS "TCS_ADDRS"
char myAddress;
// initialize timing
unsigned long previousMillis = 0;
// global TLM variables
bool heatersEnabled;
bool heater1On;
bool heater2On;
float temp_panel_1; 
float temp_panel_2;
float temp_board;
float railVoltage;
float heaterCurrent;
float heaterPower;
// UNO digital pins
int heater1SwitchPin = 6;
int heater2SwitchPin = 7;
// UNO analog pins
int thermistor_pin_1 = 0;
int thermistor_pin_2 = 1;
// Subsystem constants
const float panelNomLowC = 25.0;
const float panelNomHighC = 28.0;
const float panelSurvivalLowC = 20.0;
const float panelSurvivalHighC = 22.0;
const float railVoltageHigh = 7.0; //volts
const float railVoltageLow = 4.0; //volts
const float heaterCurrentHigh = 2000; //mA
// Other global variables
float setpoint_low;
float setpoint_high;
// Peripherals
Adafruit_INA219 ina219;
AHT20 aht20;

void setup() {
  // initiate serial interface to mega UART
  // also can be serial monitor for debug
  Serial.begin(9600);
  pinMode(heater1SwitchPin, OUTPUT); // Set heater pin as output
  pinMode(heater2SwitchPin, OUTPUT); // Set heater pin as output
  // pull in TCS address
  for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        if (addrs[i].name == ADDR_TCS) {
            myAddress=addrs[i].address;
            break;
    }
  }
  // initiate I2C interfaces
  // Current voltage monitor
  if (! ina219.begin()) {
    Serial.print(START_MARKER);
    Serial.print(myAddress);
    Serial.print(LOG_TYPE);
    Serial.print("Failed to find INA219 chip");
    Serial.print(END_MARKER);
  }
  // Current voltage monitor
  if (! aht20.begin()) {
    Serial.print(START_MARKER);
    Serial.print(myAddress);
    Serial.print(LOG_TYPE);
    Serial.print("Failed to find AHT20 chip");
    Serial.print(END_MARKER);
  }
  // nominal heater setpoints
  setpoint_low=panelNomLowC;
  setpoint_high=panelNomHighC;
  // start with heaters enabled
  heatersEnabled = true;
  // testing only
  temp_panel_2 = 24.0;
  heater1On=false;
  heater2On=false;
  }

void loop() {
  // slow down loop
  delay(100);
  char commandRead = readCommand();
  // ! means it wasn't a command received
  if (commandRead != '!') {
    processCommand(commandRead);
  }
  // Process sensor data
  collectData();
  // Check autonomy rules
  processAutonomy();
  // Actuate heaters
  actuateHeater(temp_panel_1,1);
  actuateHeater(temp_panel_2,2);
  // Send unsolicited telem to MEGA on schedule
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= TLM_PERIOD) {
    sendTlm();
    previousMillis=currentMillis;
  }
}
void collectData(){
  temp_panel_1 = read10kThermistor(thermistor_pin_1);
  temp_panel_2 = read10kThermistor(thermistor_pin_2);
  temp_board = readI2CTemp();
  railVoltage = ina219.getBusVoltage_V();
  heaterCurrent = ina219.getCurrent_mA();
  heaterPower = ina219.getPower_mW();
}
float read10kThermistor(float analogPin){
  int tempReading = analogRead(analogPin);
  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK ); //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  return tempC;
}
float readI2CTemp(){
  float tempC = aht20.getTemperature();
  if (isnan(tempC)) {
    return 0.0;
  }
  return tempC;
}
void actuateHeater(float tempC, int heaterId){
  if (tempC < setpoint_low && heatersEnabled == true) {
      // If the temperature is above setpoint and heaters enabled, turn on
    if (heaterId == 1) {
      digitalWrite(heater1SwitchPin, HIGH);
      heater1On = true;
    }
    else if (heaterId ==2){
      digitalWrite(heater2SwitchPin, HIGH);
      heater2On = true;
    }
  }
  // If the temperature is above setpoint or heaters disabled, turn off
  else if (tempC > setpoint_high || heatersEnabled == false) {
    if (heaterId == 1) {
      digitalWrite(heater1SwitchPin, LOW);
      heater1On = false;
    }
    else if (heaterId ==2){
      digitalWrite(heater2SwitchPin, LOW);
      heater2On = false;
    }
  }
}
void sendTlm() {
  // Gather telem from system and send
  int heater1OnInt = heater1On ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  int heater2OnInt = heater2On ? 1 : 0;
  int heatersEnabledInt = heatersEnabled ? 1 : 0; 
  char tempPanel1Str[8];
  dtostrf(temp_panel_1,6,2,tempPanel1Str);
  char tempPanel2Str[8];
  dtostrf(temp_panel_2,6,2,tempPanel2Str);
  char tempBoardStr[8];
  dtostrf(temp_board,6,2,tempBoardStr);
  char busVStr[8];
  dtostrf(railVoltage,6,2,busVStr);
  char heaterCurStr[8];
  dtostrf(heaterCurrent,6,2,heaterCurStr);
  char heaterPowStr[8];
  dtostrf(heaterPower,6,2,heaterPowStr);
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvTcsPacket[60];
  // Format the data into the CSV string
  snprintf(csvTcsPacket, sizeof(csvTcsPacket), "%s,%s,%s,%s,%s,%s,%d,%d,%d", tempPanel1Str, tempPanel2Str, tempBoardStr, busVStr, heaterCurStr,heaterPowStr, heatersEnabledInt, heater1OnInt, heater2OnInt);
  // Send data across UART to MEGA
  Serial.print(START_MARKER);
  Serial.print(myAddress);
  Serial.print(TLM_TYPE);
  Serial.print(csvTcsPacket);
  Serial.print(END_MARKER);
}
// Reads commands in (if any) from UART buffer
char readCommand() {
    char noCommand = '!';
    if (Serial.available()>0) {
      char receivedChar = Serial.read();
      if (receivedChar == START_MARKER) {
        char address = Serial.read();
        if (address == myAddress) {
          char messageType = Serial.read();
          if (messageType == CMD_TYPE){
            char commandId  = Serial.read();
            return commandId;
          }
        }
      }
    }
    return noCommand;
  }
 // Takes in a command character and validates
 // then executes
void processCommand(char commandId) {
  bool commandConfirmed = false;
  const char* commandName;
  for (int i = 0; i < sizeof(tcsCmds)/ sizeof(tcsCmds[0]); i++) {
        if (tcsCmds[i].id == commandId) {
            commandConfirmed = true;
            commandName = tcsCmds[i].name;
            break; // Character found in the list
        }
    }
  // Character not found in the list
  if (!commandConfirmed){
    Serial.print(START_MARKER);
    Serial.print(myAddress);
    Serial.print(LOG_TYPE);
    Serial.print("Command not found by TCS.");
    Serial.print(END_MARKER);
    return; 
  }
  // Execute command and send log back to MEGA
  switch (commandId) {
    case 'A':
      heatersEnabled = true;
      break;
    case 'B':
      heatersEnabled = false;
      break;
    case 'C':
      setpoint_low = panelNomLowC;
      setpoint_high = panelNomHighC;
      break;
    case 'D':
      setpoint_low = panelSurvivalLowC;
      setpoint_high = panelSurvivalHighC;
      break;
    default:
      return;
  }
  Serial.print(START_MARKER);
  Serial.print(myAddress);
  Serial.print(LOG_TYPE);
  Serial.print("TCS processed command: ");
  Serial.print(commandName);
  Serial.print(END_MARKER);
  }

void processAutonomy(){
    // power conditions off nominal, disable both heaters
    if (heaterCurrent>heaterCurrentHigh || (railVoltage>railVoltageHigh) || (railVoltage<railVoltageLow)){
      heatersEnabled = false;
      Serial.print(START_MARKER);
      Serial.print(myAddress);
      Serial.print(LOG_TYPE);
      Serial.print("Heaters disabled by autonomy.");
      Serial.print(END_MARKER);

    }
  }
  
