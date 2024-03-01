// Code skeleton for ArduSat Arduino Mega Core SW
// Peter Mahoney SP '24

// Define protocol characters
#define START_MARKER '<'
#define END_MARKER   '>'
#define ADDR_TCS 'A'
#define ADDR_MEGA 'D'
#define TLM 'T'
#define COMMAND 'C'

// Define set of known commands
char tcsCommandKeys[] = {'A','B','C','D'};

// Setup timebased scheduler
unsigned long previousMillis = 0;
unsigned long previousMillisTCS = 0;
const long tenSec = 10000; // 10 second interval
const long fiveSec = 5000; // 5 second interval

void setup() {
  // initiate serial interfaces
  // Serial 0 interface to Serial Monitor over USB
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for Serial Monitor to open
  }
  // Serial 1 is ArduSat UART bus
  Serial1.begin(9600);
  // Serial 2 interface could be for TTC if desired

}

void loop() {
  // Adding small delays is required at this stage to slow down the loop
  // otherwise, the UART buffer can't be read effectively
  delay(100);
  // Send dummy TCS commands
  sendTcsCommands();
  // Get Telemetry Over UART from subsystems
  // Simply print to Serial monitor for now
  if (Serial1.available()>0) {
    char receivedChar = Serial1.read();
    if (receivedChar == START_MARKER) {
      char address = Serial1.read();
      if (address == ADDR_TCS) {
        char infoType = Serial1.read();
        if (infoType == 'T'){
          Serial.print("Received TCS telemetry:\n");
          while (Serial1.available() > 0) {
            char message  = Serial1.read();
            if (message == ','){
              Serial.println();
            }
            else if (message == END_MARKER) {
              Serial.print("\nEnd of packet.\n");
            }
            else { Serial.print(message);} // Should be "Send to TTC"
          }
      }
    }
  }
  }
  // Dummy mega service loop activity to demonstrate task scheduling
  // print to the Serial terminal every 10 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= tenSec) {
    Serial.println("Example Mega Action.");
    previousMillis = currentMillis; // Reset the timer
  }
// To be developed skeleton
    // Process received data from the other board
  // Process and send commands from TTC
    // Send ADCS commands
    // Send TCS commands
    // Send TTC commands
    // Send EPS commands
  // Get telemetry from subsystems
    // Get ADCS telem
    // Get TCS telem
    // Get TTC telem
    // Get EPS telem
  // Process telemetry and check rules?
    // Process ADCS rules and any commands
    // Process TCS rules and any commands
    // Process TTC rules and any commands
    // Process EPS rules and any commands
  // Package telem and send to TTC over UART
  // EPS housekeeping?
}
// Should be generalized for multiple subsystems, TCS for now
void sendTcsCommands(){
  // Send dummy command to enable heater every 5 sec
  bool sendCommand = false;
  char command;
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillisTCS;
  if (elapsedMillis > fiveSec) {
    command = 'B';
    sendCommand = true;
    Serial.println("Mega turning heaters OFF.\n");
    previousMillisTCS = currentMillis; // Reset the timer
  }
  // Build and send command using protocol
  // Pick arbitrary command, will refine this
  if (sendCommand){
    Serial1.print(START_MARKER);
    Serial1.print(ADDR_TCS);
    Serial1.print(COMMAND);
    Serial1.print(tcsCommandKeys[1]);
    Serial1.print(END_MARKER);
  }
  }
void getEpsTelem() {
  // TODO, gather telem from EPS system
}
void sendEpsCommands(){
  // TODO, send commands to EPS system
}
