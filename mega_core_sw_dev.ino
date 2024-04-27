// ArduSat Arduino MEGA C&DH and EPS CORE SW
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#include <Adafruit_INA219.h>
#include "config.h"
#define ADDR_CDH "CDH_ADDRS"
#define ADDR_EPS "EPS_ADDRS"
#define ADDR_TTC "TTC_ADDRS"
#define ADDR_TCS "TCS_ADDRS"
#define ADDR_ADCS "ADCS_ADDRS"
// Ground test mode?
bool groundTestMode = true; 
// variables to hold UART Address
char cdhAddress;
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
bool wheelSwitchOn; 
float busVoltage; //V
float busCurrent; // mA
float busPower; // mW
// float battSoc; //%
// EPS constants
const float busVoltageHigh = 13.0; //volts
const float busVoltageLow = 4.0; //volts
const float busCurrentHigh = 6000; //mA
const int wheelSwitchPin = 50;
// Rail voltage sensor
Adafruit_INA219 ina219;

void setup() {
  // initiate serial interfaces
  // Serial 0 interface to Serial Monitor over USB for testing
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
        if (addrs[i].name == ADDR_CDH) {
            cdhAddress = addrs[i].address;
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
    TTC_UART->print(START_MARKER);
    TTC_UART->print(epsAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("Failed to find INA219 chip.");
    TTC_UART->print(END_MARKER);
    delay(tenSec);
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
  pinMode(wheelSwitchPin, OUTPUT);
  // Wheel disabled at startup
  wheelSwitchOn = false;
  // Print startup log
  TTC_UART->print(START_MARKER);
  TTC_UART->print(cdhAddress);
  TTC_UART->print(LOG_TYPE);
  TTC_UART->print("Core SW starting up!");
  TTC_UART->print(END_MARKER);
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
  // Check watchdogs on each node
  checkWatchdogs();
}
void checkWatchdogs(){
  // Pet watchdogs (testing only)
  if (groundTestMode){
    petWatchdogs(true,true,true);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - tcsWatchdog >= thirtySec)
  {
    resetBoard(tcsResetPin);
    tcsWatchdog = currentMillis;
    TTC_UART->print(START_MARKER);
    TTC_UART->print(cdhAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("TCS watchdog expired, board reset.");
    TTC_UART->print(END_MARKER);
  }
  if (currentMillis - adcsWatchdog >= thirtySec)
  {
    resetBoard(adcsResetPin);
    adcsWatchdog = currentMillis;
    TTC_UART->print(START_MARKER);
    TTC_UART->print(cdhAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("ADCS watchdog expired, board reset.");
    TTC_UART->print(END_MARKER);
  }
  if (currentMillis - ttcWatchdog >= tenMin)
  {
    resetBoard(ttcResetPin);
    ttcWatchdog = currentMillis;
    TTC_UART->print(START_MARKER);
    TTC_UART->print(cdhAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("TTC watchdog expired, board reset.");
    TTC_UART->print(END_MARKER);
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
        if (address == cdhAddress){
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
            commandName = cdhCmds[i].name;
            break; // Character found in the list
        }
    }
  if (!commandConfirmed){
    TTC_UART->print(START_MARKER);
    TTC_UART->print(cdhAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("Command not found by CDH.");
    TTC_UART->print(END_MARKER);
    return; // Character not found in the list
  }
  unsigned long currentMillis = millis();
  switch (commandId) {
    case 'A':
      // reset TCS
      resetBoard(tcsResetPin);
      tcsWatchdog = currentMillis;
      break;
    case 'B':
      // reset ADCS
      resetBoard(adcsResetPin);
      adcsWatchdog = currentMillis;
      break;
    case 'C':
      // reset TTC
      resetBoard(ttcResetPin);
      ttcWatchdog = currentMillis;
    case 'D':
      // reset TTC
      resetBoard(ttcResetPin);
      resetBoard(adcsResetPin);
      resetBoard(tcsResetPin);
      ttcWatchdog = currentMillis;
      adcsWatchdog = currentMillis;
      tcsWatchdog = currentMillis;
    default:
      return;
  }
  TTC_UART->print(START_MARKER);
  TTC_UART->print(cdhAddress);
  TTC_UART->print(LOG_TYPE);
  TTC_UART->print("CDH processed command: ");
  TTC_UART->print(commandName);
  TTC_UART->print(END_MARKER);
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
      // Confirm sender is not MEGA
      if (address != cdhAddress) {
        char infoType = nodeUart.read();
        // Confirm message is TLM or LOG type
        if (infoType == TLM_TYPE || infoType == LOG_TYPE){
            const char* subsystem;
            // Get subsystem name
            for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
              if (addrs[i].address == address) {
                subsystem = addrs[i].name;
                // Pet watchdog for subsystem received
                unsigned long currentMillis = millis();
                if (subsystem == ADDR_TCS){
                  tcsWatchdog = currentMillis; // Pet the tcs watchdog
                }
                else if (subsystem == ADDR_ADCS){
                  adcsWatchdog = currentMillis; // Pet the adcs watchdog
                }
                else if (subsystem == ADDR_TTC){
                  ttcWatchdog = currentMillis; // Pet the ttc watchdog
                }
                break;
              }
            }
          // Route message across TTC UART for downlink
          TTC_UART->print(START_MARKER);
          TTC_UART->print(address); //sender address
          TTC_UART->print(infoType);
          if (groundTestMode && infoType == 'T'){
            telemPrettyPrinter(nodeUart, subsystem);
          }
          else {
            while (nodeUart.available() > 0) {
              char character  = nodeUart.read();
              // Print character over TTC UART
              TTC_UART->print(character);
            }
          }
        }
      }
     }
  }
 }
void telemPrettyPrinter(Stream &nodeUart, char *subsystem){
  // Loop through message and print to TTC/Serial for testing
          int id = 0;
          int i=0;
          char message[100];
          message[0] = '\0';
          bool newPoint = true;
          while (nodeUart.available() > 0) {
            char character  = nodeUart.read();
            // TODO remove all below this, integration only
            if (character != ',' && character != END_MARKER){
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
  //Gather telem from EPS system
  busVoltage = ina219.getBusVoltage_V();
  busCurrent = ina219.getCurrent_mA();
  busPower = ina219.getPower_mW();
  // battSoc = 100*(battVoltage/3.7);
}
void sendEpsTelem() {
  // Send telem to TTC from EPS system
  int wheelSwitchInt = wheelSwitchOn ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  char busVStr[8];
  dtostrf(busVoltage,6,2,busVStr);
  char busCurStr[8];
  dtostrf(busCurrent,6,2,busCurStr);
  char busPowStr[8];
  dtostrf(busPower,6,2,busPowStr);
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvEpsPacket[40];
  // Format the data into the CSV string
  snprintf(csvEpsPacket, sizeof(csvEpsPacket), "%s,%s,%s,%d" busVStr, busCurStr, busPowStr, wheelSwitchInt);
  // Send data across UART to MEGA
  TTC_UART->print(START_MARKER);
  TTC_UART->print(epsAddress);
  TTC_UART->print(TLM_TYPE);
  TTC_UART->print(csvEpsPacket);
  TTC_UART->print(END_MARKER);
}
void processEpsAutonomy(){
  // Monitor telemetry and respond to out of limits
  // Bus voltage out of range, isolate battery
  if ((busVoltage>busVoltageHigh || busVoltage<busVoltageLow)&& wheelSwitchOn){
    batteryEnabled=false;
    TTC_UART->print(START_MARKER);
    TTC_UART->print(epsAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("Bus voltage out of range, isolating wheel.");
    TTC_UART->print(END_MARKER);
    setEpsSwitch(wheelSwitchPin, wheelSwitchOn);
  }
  // Current draw too high, shutdown rail
  if (busCurrent>busCurrentHigh && wheelSwitchOn){
    railEnabled=false;
    TTC_UART->print(START_MARKER);
    TTC_UART->print(epsAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("Bus current out of range, disabling wheel and heaters.");
    TTC_UART->print(END_MARKER);
    setEpsSwitch(wheelSwitchPin, wheelSwitchOn);
  }
}
void processEpsCommand(char commandId) {
  bool commandConfirmed = false;
  const char* commandName;
  for (int i = 0; i < sizeof(epsCmds)/ sizeof(epsCmds[0]); i++) {
        if (epsCmds[i].id == commandId) {
            commandConfirmed = true;
            commandName = epsCmds[i].name;
            break; // Character found in the list
        }
    }
  // Character not found in the list
  if (!commandConfirmed){
    TTC_UART->print(START_MARKER);
    TTC_UART->print(epsAddress);
    TTC_UART->print(LOG_TYPE);
    TTC_UART->print("Command not found by EPS.");
    TTC_UART->print(END_MARKER);
    return; 
  }
  // Execute command and send log back to MEGA
  switch (commandId) {
    case 'A':
      wheelSwitchOn = true;
      setEpsSwitch(wheelSwitchPin, wheelSwitchOn);
      break;
    case 'B':
      wheelSwitchOn = false;
      setEpsSwitch(wheelSwitchPin, wheelSwitchOn);
      break;
    default:
      return;
  }
  TTC_UART->print(START_MARKER);
  TTC_UART->print(epsAddress);
  TTC_UART->print(LOG_TYPE);
  TTC_UART->print("EPS processed command: ");
  TTC_UART->print(commandName);
  TTC_UART->print(END_MARKER);
  }
void setEpsSwitch(int switchPin, bool on){
  if (on==true){
    digitalWrite(switchPin,HIGH);
  }
  else {
    digitalWrite(switchPin,LOW);
  }
}
void petWatchdogs(bool ttc, bool tcs, bool adcs){
  unsigned long currentMillis = millis();
  if (ttc==true){
    ttcWatchdog = currentMillis;
  }
  if (tcs==true){
    tcsWatchdog = currentMillis;
  }
  if (adcs==true){
    adcsWatchdog = currentMillis;
  }
}
