// Code skeleton for ArduSat Arduino Uno Subsystems
// Peter Mahoney SP '24
#include <stdio.h>
#include <string.h>
#define START_MARKER '<'
#define END_MARKER   '>'
#define ADDR_TCS 'A'
#define ADDR_MEGA 'D'
#define TLM 'T'
#define COMMAND 'C'
unsigned long previousMillis = 0;
const long intervalShort = 2000; // 2 second interval
const long intervalLong = 5000; // 5 second interval
bool heaterEnabled = true; 

// Define set of known commands
char commandKeys[] = {'A','B','C','D'};

void setup() {
  // initiate serial interfaces
  Serial.begin(9600);
  // initiate I2C interfaces
  // instantiate global variables

}

void loop() {
  // slow down loop
  delay(100);
  // Send telem to mega every 2 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalShort) {
    sendTlm();
    previousMillis=currentMillis;
  }
  char commandRead = readCommand();
  if (commandRead != "Z") {
    processCommand(commandRead);
  }
  // Process sensor data
  // Adjust heater states
  // Check for mega commands
    // if mega serial available:
      // read in command
      // if addressed to TCS:
         // process command
            // Turn off heaters
            // send telem, etc
}
void sendTlm() {
  // Gather telem from system and send
  float temp1 = 10.00;
  float temp2 = 25.00;
  float temp3 = 30.00;
  bool heaterOn;
  if (heaterEnabled == true) {
    heaterOn = true;
  }
  else {
    heaterOn = false;
  }
  int heaterOnInt = heaterOn ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  char temp1str[8];
  dtostrf(temp1,5,2,temp1str);
  char temp2str[8];
  dtostrf(temp2,5,2,temp2str);
  char temp3str[8];
  dtostrf(temp3,5,2,temp3str);
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvTcsPacket[50];
  // Format the data into the CSV string
  snprintf(csvTcsPacket, sizeof(csvTcsPacket), "%s,%s,%s,%d", temp1str, temp2str, temp3str, heaterOnInt);
  // Print temperature values for debugging
  Serial.print(START_MARKER);
  Serial.print(ADDR_TCS);
  Serial.print(TLM);
  Serial.print(csvTcsPacket);
  Serial.print(END_MARKER);
}

char readCommand() {
    char noCommand = "Z";
    if (Serial.available()>0) {
    char receivedChar = Serial.read();
      if (receivedChar == START_MARKER) {
        char address = Serial.read();
        if (address == ADDR_TCS) {
          char messageType = Serial.read();
          if (messageType == COMMAND){
            char command  = Serial.read();
            return command;
          }
        }
      }
    }
    return noCommand;
  }
void processCommand(char command) {
  bool commandConfirmed = false;
  for (int i = 0; i < sizeof(commandKeys); i++) {
        if (commandKeys[i] == command) {
            commandConfirmed = true;
            break; // Character found in the list
        }
    }
  if (!commandConfirmed){
    return; // Character not found in the list
  }
  if (command == 'A'){
    heaterEnabled = true;
  }
  else{
    heaterEnabled = false;
  }
  }
  
