// ArduSat Arduino MEGA C&DH and EPS CORE SW
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include <Adafruit_INA219.h>
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
HardwareSerial *TTC_UART = &Serial1;
HardwareSerial *TCS_UART = &Serial2;
HardwareSerial *ADCS_UART = &Serial3;
// Setup subsystem watchdog timers;
unsigned long tcsWatchdog;
unsigned long adcsWatchdog;
unsigned long ttcWatchdog;
// Watchdog pinOuts
const int ttcResetPin = 31;
const int tcsResetPin = 33;
const int adcsResetPin = 35;
// Setup timebased scheduler
unsigned long tlmTimer;
const long tenSec = 10000; // 10 second interval
const long thirtySec = 30000; // 30 second interval
const long tenMin = 600000; // 10 min interval
// EPS telem
bool batteryEnabled;
bool railEnabled;
// EPS switches
const int batterySwitchPin = 50;
const int railSwitchPin = 52;
// Rail voltage sensor
Adafruit_INA219 ina219;

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
  // pull in message addresses
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
  // initiate I2C interfaces
  // Current voltage monitor
  if (! ina219.begin()) {
    Serial.print(START_MARKER);
    Serial.print(megaAddress);
    Serial.print(LOG_TYPE);
    Serial.print('Failed to find INA219 chip');
    Serial.print(END_MARKER);
    while (1) { delay(10); }
  }
  // initialize timers/watchdogs
  unsigned long currentMillis = millis();
  tcsWatchdog=currentMillis;
  adcsWatchdog=currentMillis;
  ttcWatchdog=currentMillis;
  tlmTimer = currentMillis;
  // initialize board reset pins high
  // Set the reset pin as an output
  pinMode(tcsResetPin, OUTPUT);
  pinMode(ttcResetPin, OUTPUT);
  pinMode(adcsResetPin, OUTPUT);
  // Initially, keep the Uno running normally
  digitalWrite(tcsResetPin, HIGH);
  digitalWrite(ttcResetPin, HIGH);
  digitalWrite(adcsResetPin, HIGH);
  // Initialize EPS switches
  pinMode(batterySwitchPin, OUTPUT);
  pinMode(railSwitchPin, OUTPUT);
  // EPS enabled at startup
  batteryEnabled = true;
  railEnabled = true;
}

void loop() {
  // Adding small delays is required at this stage to slow down the loop
  // otherwise, the UART buffer can't be read effectively
  delay(300);
  // Send check if any commands in buffer from TTC
  processGroundCommand();
  // EPS service loop
  epsLoop();
  // Get Telemetry or Log over UART from subsystems
  processMessage(*TCS_UART);
  processMessage(*ADCS_UART);
  // Dummy mega service loop activity to demonstrate task scheduling
  unsigned long currentMillis = millis();
  // Check watchdogs on each node
  if (currentMillis - tcsWatchdog >= thirtySec)
  {
    resetBoard(tcsResetPin);
  }
  if (currentMillis - adcsWatchdog >= thirtySec)
  {
    resetBoard(adcsResetPin);
  }
  if (currentMillis - ttcWatchdog >= tenMin)
  {
    resetBoard(ttcResetPin);
  }
  
}
void processGroundCommand() {
    char badCommand = '!';
    char commandId;
    bool validAddress = false;
    if (TTC_UART->available()>0) {
      char receivedChar = TTC_UART->read();
      if (receivedChar == START_MARKER) {
        unsigned long currentMillis = millis();
        ttcWatchdog = currentMillis; // Reset the TTC watchdog
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
          // TODO remove debug
          Serial.println("Sent command to node.");
      }
    }
  }
}

void processMegaCommand(char commandId) {
  bool commandConfirmed = false;
  const char* commandName;
  for (int i = 0; i < sizeof(cdhCmds)/ sizeof(cdhCmds[0]); i++) {
        if (cdhCmds[i].id == commandId) {
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

// Get telemetry or logs from subsystems
// Process telemetry and check rules?
// Package telem and send to TTC over UART
void processMessage(Stream &nodeUart) {
  // Read in from UART buffer if anything there
  if (nodeUart.available()>0) {
    char receivedChar = nodeUart.read();
    // If start marker found, keep reading, else return
    if (receivedChar == START_MARKER) {
      char address = nodeUart.read();
      // Confirm address is not MEGA
      if (address != megaAddress) {
        char infoType = nodeUart.read();
        // Confirm message is TLM or LOG type
        if (infoType == TLM_TYPE || infoType == LOG_TYPE){
            const char* subsystem;
            // Get subsystem name
            for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
              if (addrs[i].address == address) {
                subsystem = addrs[i].name;
                // Reset watchdog for subsystem received
                unsigned long currentMillis = millis();
                if (subsystem == 'TCS_ADDRS'){
                  tcsWatchdog = currentMillis; // Reset the tcs watchdog
                }
                else if (subsystem == 'ADCS_ADDRS'){
                  adcsWatchdog = currentMillis; // Reset the adcs watchdog
                }
                else if (subsystem == 'TTC_ADDRS'){
                  ttcWatchdog = currentMillis; // Reset the ttc watchdog
                }
                break;
              }
            }
          // Test code, TODO remove all Serial prints when integrated with TTC
          if (infoType == TLM_TYPE){
            Serial.print("Receiving telemetry from: ");
          }
          else if (infoType == LOG_TYPE) {
            Serial.print("Receiving log message from: ");
          }
          Serial.println(subsystem);
          // Route message across TTC UART for downlink
          TTC_UART->print(START_MARKER);
          TTC_UART->print(ttcAddress);
          TTC_UART->print(infoType);
          // Loop through message and print to TTC/Serial for testing
          int id = 0;
          int i=0;
          char message[5];
          message[0] = '\0';
          bool newPoint = true;
          while (nodeUart.available() > 0) {
            char character  = nodeUart.read();
            // Print character over TTC UART
            TTC_UART->print(character);
            // TODO remove all below this, integration only
            if (character != ',' && character != END_MARKER && infoType == TLM_TYPE){
              if (newPoint==true){
                i=0;
                message[0]='\0';
                newPoint=false;
              }
              sprintf(message,"%s%c",message,character);
              i++;
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
              id++;
              if (character == END_MARKER){
                Serial.print("End of message.\n");
                break;
              }
            }
          }
        }
      }
     }
  }
 }
void resetBoard(int resetPin){
  digitalWrite(resetPin, LOW);
  delay(10);
  digitalWrite(resetPin, HIGH);
}
void epsLoop() {
  // TODO, gather telem from EPS system
  processEpsTelem();
  processEpsAutonomy();
  unsigned long currentMillis = millis();
  if (currentMillis - tlmTimer >= TLM_PERIOD) {
    sendEpsTelem();
    tlmTimer = currentMillis; // Reset the timer
  }
}
void processEpsTelem() {
  // TODO, gather telem from EPS system
}
void sendEpsTelem() {
  // TODO, gather telem from EPS system
}
void processEpsAutonomy(){
  // TODO, send commands to EPS system
}
void processEpsCommand(char commandId) {
  bool commandConfirmed = false;
  const char* commandName;
  for (int i = 0; i < sizeof(epsCmds)/ sizeof(epsCmds[0]); i++) {
        if (epsCmds[i].id == commandId) {
            commandConfirmed = true;
            commandName = tcsCmds[i].name;
            break; // Character found in the list
        }
    }
  // Character not found in the list
  if (!commandConfirmed){
    Serial.print(START_MARKER);
    Serial.print(epsAddress);
    Serial.print(LOG_TYPE);
    Serial.print('Command not found by EPS.');
    Serial.print(END_MARKER);
    return; 
  }
  // Execute command and send log back to MEGA
  switch (commandId) {
    case 'A':
      batteryEnabled = true;
      setEpsSwitch(batterySwitchPin, batteryEnabled);
      break;
    case 'B':
      batteryEnabled = false;
      setEpsSwitch(batterySwitchPin, batteryEnabled);
      break;
    case 'C':
      railEnabled = true;
      setEpsSwitch(railSwitchPin, railEnabled);
      break;
    case 'D':
      railEnabled = false;
      setEpsSwitch(railSwitchPin, railEnabled);
      break;
    default:
      return;
  }
  Serial.print(START_MARKER);
  Serial.print(epsAddress);
  Serial.print(LOG_TYPE);
  Serial.print('EPS processed command: ');
  Serial.print(commandName);
  Serial.print(END_MARKER);
  }
void setEpsSwitch(int switchPin, bool on){
  if (on==true){
    digitalWrite(switchPin,HIGH);
  }
  else {
    digitalWrite(switchPin,LOW);
  }
}
