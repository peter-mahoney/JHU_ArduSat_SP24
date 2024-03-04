// Code skeleton for ArduSat Arduino Uno Subsystems
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include "config.h"
#define ADDR_TCS "TCS_ADDRS"
char tcsAddress;
unsigned long previousMillis = 0;
const long intervalShort = 2000; // 2 second interval
const long intervalLong = 5000; // 5 second interval
bool heaterEnabled = true; 
bool heaterOn;
float temp1; 
float temp2;
float temp3;

void setup() {
  // initiate serial interface to main ArduSat bus 
  // also can be serial monitor for debug
  Serial.begin(9600);
  // initiate I2C interfaces if any
  // pull in TCS address
  for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        if (addrs[i].name == ADDR_TCS) {
            tcsAddress=addrs[i].address;
            break;
    }
  }
}

void loop() {
  // slow down loop
  delay(100);
  char commandRead = readCommand();
  // ! means it wasn't a command received
  if (commandRead != '!') {
    processCommand(commandRead);
  }
  // Do TCS actions
  // Process sensor data
  collectData();
  // Adjust heater states
  // Send unsolicited telem to mega every 2 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalShort) {
    sendTlm();
    previousMillis=currentMillis;
  }
}
void collectData(){
  temp1 = 10.00;
  temp2 = 25.00;
  temp3 = 30.00;
  if (heaterEnabled == true) {
    heaterOn = true;
  }
  else {
    heaterOn = false;
  }
}
void sendTlm() {
  // Gather telem from system and send
  int heaterOnInt = heaterOn ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  int heaterEnabledInt = heaterEnabled ? 1 : 0; 
  char temp1str[8];
  dtostrf(temp1,5,2,temp1str);
  char temp2str[8];
  dtostrf(temp2,5,2,temp2str);
  char temp3str[8];
  dtostrf(temp3,5,2,temp3str);
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvTcsPacket[50];
  // Format the data into the CSV string
  snprintf(csvTcsPacket, sizeof(csvTcsPacket), "%s,%s,%s,%d,%d", temp1str, temp2str, temp3str, heaterEnabledInt, heaterOnInt);
  // Print temperature values for debugging
  Serial.print(START_MARKER);
  Serial.print(tcsAddress);
  Serial.print(TLM_TYPE);
  Serial.print(csvTcsPacket);
  Serial.print(END_MARKER);
  // TODO, remove this once real TCS code implemented
  heaterEnabled = true;
}
// Reads commands in (if any) from UART buffer
// Should generalize this to other potential
// UART traffic perhaps
char readCommand() {
    char noCommand = '!';
    if (Serial.available()>0) {
      char receivedChar = Serial.read();
      if (receivedChar == START_MARKER) {
        char address = Serial.read();
        if (address == tcsAddress) {
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
  if (!commandConfirmed){
    return; // Character not found in the list
  }
  switch (commandId) {
    case 'A':
      heaterEnabled = true;
      break;
    case 'B':
      heaterEnabled = false;
      break;
    default:
      break;
  }
  // can add C and D eventually
  }
  
