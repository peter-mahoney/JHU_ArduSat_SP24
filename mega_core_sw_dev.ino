// Code skeleton for ArduSat Arduino Mega Core SW
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include "config.h"
#define ADDR_MEGA "MEGA_ADDRS"
#define ADDR_EPS "EPS_ADDRS"
#define ADDR_TTC "TTC_ADDRS"
#define ADDR_TCS "TCS_ADDRS"
#define ADDR_ADCS "ADCS_ADDRS"
// variables to hold UART Address
char megaAddress;
char epsAddress;
char ttcAddress;
char tcsAddress;
char adcsAddress;
// Serial definitions
// TODO: Replace TTC uart with Serial 1
HardwareSerial *TTC_UART = &Serial;
HardwareSerial *TCS_UART = &Serial2;
HardwareSerial *ADCS_UART = &Serial3;
// Setup subsystem watchdogs
unsigned long tcsWatchdog = 0;
unsigned long adcsWatchdog = 0;
unsigned long ttcWatchdog = 0;
// Setup timebased scheduler
unsigned long previousMillis = 0;
unsigned long previousMillisTCS = 0;
const long tenSec = 10000; // 10 second interval
const long thirtySec = 30000; // 30 second interval

void setup() {
  // initiate serial interfaces
  // Serial 0 interface to Serial Monitor over USB
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for Serial Monitor to open
  }
  // Serial 1 is TTC bus
  TTC_UART->begin(9600);
  // Serial 2 is TCS bus
  TCS_UART->begin(9600);
  // Serial 3 is ADCS bus
  ADCS_UART->begin(9600);

  // pull in address IDs
  for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        if (addrs[i].name == ADDR_MEGA) {
            megaAddress = addrs[i].address;
        }
        else if (addrs[i].name == ADDR_EPS) {
            epsAddress = addrs[i].address;
        }
        else if (addrs[i].name == ADDR_TTC) {
            ttcAddress = addrs[i].address;
        }
        else if (addrs[i].name == ADDR_TCS) {
            tcsAddress = addrs[i].address;
        }
        else if (addrs[i].name == ADDR_ADCS) {
            adcsAddress = addrs[i].address;
        }
        
  }
}

void loop() {
  // Adding small delays is required at this stage to slow down the loop
  // otherwise, the UART buffer can't be read effectively
  delay(300);
  // Send check if any commands in buffer from TTC
  processGroundCommand();
  // Send dummy commands to TCS
  testCommandGenerator(*TCS_UART);
  // Get Telemetry Over UART from subsystems
  processTelemetry(*TCS_UART);
  processTelemetry(*ADCS_UART);
  // Dummy mega service loop activity to demonstrate task scheduling
  // print to the Serial terminal every 10 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= tenSec) {
    Serial.println("Example Mega Action.");
    previousMillis = currentMillis; // Reset the timer
  }

  // EPS housekeeping?
}
void processGroundCommand() {
    char badCommand = '!';
    char commandId;
    bool validAddress = false;
    if (TTC_UART->available()>0) {
      char receivedChar = TTC_UART->read();
      if (receivedChar == START_MARKER) {
        char address = TTC_UART->read();
        for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
              if (addrs[i].address == address) {
                validAddress=true;
                break;
              }
            }
        if (validAddress != true){
          return;
        }
        char messageType = TTC_UART->read();
        if (messageType != CMD_TYPE){
           return; 
        }
        else {commandId  = TTC_UART->read();}
        if (address == megaAddress){
          processMegaCommand(commandId);
        }
        else if (address == epsAddress) {
          processEpsCommand(commandId);
        }
        else {
          HardwareSerial *nodeUart;
          if (address == tcsAddress){
            nodeUart = TCS_UART;
          }
          else if (address == adcsAddress){
            nodeUart = ADCS_UART;
          }
          nodeUart->print(START_MARKER);
          nodeUart->print(address);
          nodeUart->print(CMD_TYPE);
          nodeUart->print(commandId);
          nodeUart->print(END_MARKER);
          Serial.println("Sent command to node.");
      }
    }
  }
}

void processMegaCommand(char commandId) {
  bool commandConfirmed = false;
  const char* commandName;
  for (int i = 0; i < sizeof(megaCmds)/ sizeof(megaCmds[0]); i++) {
        if (megaCmds[i].id == commandId) {
            commandConfirmed = true;
            break; // Character found in the list
        }
    }
  if (!commandConfirmed){
    return; // Character not found in the list
  }
  switch (commandId) {
    case 'A':
      // reset board
      break;
    case 'B':
      // reset board
      break;
    default:
      break;
  }
}
void processEpsCommand(char commandId) {
  bool commandConfirmed = false;
  for (int i = 0; i < sizeof(epsCmds)/ sizeof(epsCmds[0]); i++) {
        if (megaCmds[i].id == commandId) {
            commandConfirmed = true;
            break; // Character found in the list
        }
    }
  if (!commandConfirmed){
    return; // Character not found in the list
  }
  // TODO switch statement for EPS commandIds
  switch (commandId) {
    case 'A':
      // do something
      break;
    case 'B':
      // do something
      break;
    default:
      break;
  }
}
  // Get telemetry from subsystems
  // Process telemetry and check rules?
  // Package telem and send to TTC over UART
 void processTelemetry(Stream &nodeUart) {
  if (nodeUart.available()>0) {
    char receivedChar = nodeUart.read();
    if (receivedChar == START_MARKER) {
      char address = nodeUart.read();
      if (address != megaAddress) {
        char infoType = nodeUart.read();
        if (infoType == 'T'){
            const char* subsystem;
            for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
              if (addrs[i].address == address) {
                subsystem = addrs[i].name;
                break;
              }
            }
          // TODO remove all Serial prints when integrated with TTC
          Serial.print("Receiving telemetry from: ");
          Serial.println(subsystem);
          // TODO: add these prints to TTC back in
//          TTC_UART->print(START_MARKER);
//          TTC_UART->print(ttcAddress);
//          TTC_UART->print(TLM_TYPE);
          int id = 0;
          int i=0;
          char message[5];
          message[0] = '\0';
          bool newPoint = true;
          while (nodeUart.available() > 0) {
            char character  = nodeUart.read();
            if (character != ',' && character != END_MARKER){
              if (newPoint==true){
                i=0;
                message[0]='\0';
                newPoint=false;
              }
              sprintf(message,"%s%c",message,character);
              i++;
//              TTC_UART->print(character);
            }
            else {
              newPoint = true;
              if (subsystem == "TCS_ADDRS"){
                Serial.print(tcsPoints[id].name);
              }
              else if (subsystem = "ADCS_ADDRS"){
                Serial.print(adcsPoints[id].name);
              }
              else {Serial.print("UNKNOWN NODE");}
              Serial.print(": ");
              Serial.print(message);
              Serial.println();
//              TTC_UART->print(character);
              id++;
              if (character == END_MARKER){
                Serial.print("End of packet.\n");
                break;
              }
            }
          }
        }
      }
     }
  }
 }
// Testing for node integration
// currently hardcoded for TCS node
void testCommandGenerator(Stream &nodeUart){
  // Send dummy command to disable heater every 5 sec
  bool sendCommand = false;
  char commandId;
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillisTCS;
  if (elapsedMillis > thirtySec) {
    // CommandID should be 'B' for disabling heaters
    commandId = tcsCmds[1].id;
    sendCommand = true;
    Serial.println("MEGA sending disable heater command to TCS node.");
    previousMillisTCS = currentMillis; // Reset the timer
  }
  // Build and send command using protocol
  // Pick arbitrary command, will refine this
  // hardcoded to TCS address
  if (sendCommand){
    nodeUart.print(START_MARKER);
    nodeUart.print(addrs[0].address);
    nodeUart.print(CMD_TYPE);
    nodeUart.print(commandId);
    nodeUart.print(END_MARKER);
  }
}
void getEpsTelem() {
  // TODO, gather telem from EPS system
}
void sendEpsCommands(){
  // TODO, send commands to EPS system
}
