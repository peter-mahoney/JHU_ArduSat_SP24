// Code skeleton for ArduSat Arduino Uno Subsystems
// ADCS example
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include "config.h"
#define ADDR_ADCS "ADCS_ADDRS"
char myAddress;
unsigned long previousMillis = 0;
const long intervalShort = 2000; // 2 second interval
const long intervalLong = 5000; // 5 second interval
bool rwOn = true;
bool rwSpinning = false;
float temp1; 
float temp2;
float temp3;

void setup() {
  // initiate serial interface to to mega UART
  // also can be serial monitor for debug
  Serial.begin(9600);
  // initiate I2C interfaces if any
  // pull in ADCS address
  for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        if (addrs[i].name == ADDR_ADCS) {
            myAddress=addrs[i].address;
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
// nothing!
}
void sendTlm() {
  // Gather telem from system and send
  int rwOnInt = rwOn ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvTlmPacket[50];
  // Format the data into the CSV string
  snprintf(csvTlmPacket, sizeof(csvTlmPacket), "%d", rwOnInt);
  // Print temperature values for debugging
  Serial.print(START_MARKER);
  Serial.print(myAddress);
  Serial.print(TLM_TYPE);
  Serial.print(csvTlmPacket);
  Serial.print(END_MARKER);
  // TODO, remove this once real ADCS code implemented
  rwOn = true;
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
  for (int i = 0; i < sizeof(adcsCmds)/ sizeof(adcsCmds[0]); i++) {
        if (tcsCmds[i].id == commandId) {
            commandConfirmed = true;
            commandName = adcsCmds[i].name;
            break; // Character found in the list
        }
    }
  if (!commandConfirmed){
    return; // Character not found in the list
  }
  switch (commandId) {
    case 'A':
      rwOn = true;
      break;
    case 'B':
      rwOn = false;
      break;
    default:
      break;
  }
  // can add C and D eventually
  }
  
