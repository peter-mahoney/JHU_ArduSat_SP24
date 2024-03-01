// Code skeleton for ArduSat Arduino Mega Core SW
// Peter Mahoney SP '24
#define START_MARKER '<'
#define END_MARKER   '>'
#define ADDR_TCS 'A'

unsigned long previousMillis = 0;
unsigned long previousMillisTCS = 0;
const long interval = 10000; // 10 second interval

void setup() {
  // initiate serial interfaces
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for Serial Monitor to open
  }
  // Serial 1 is ArduSat UART bus
  Serial1.begin(9600);
  // instantiate global variables

}

void loop() {
  //  Serial.println("Hello from Arduino Mega!");
  delay(100);
  // Send dummy TCS commands
  sendTcsCommands();
  // Get Telemetry Over UART
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
            else { Serial.print(message);}
          }
      }
    }
  }
  }

  // Print to the Serial terminal every 10 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    Serial.println("Example Mega Action.");
    previousMillis = currentMillis; // Reset the timer
  }

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

void sendTcsCommands(){
  // Send dummy command to enable heater every 5 sec
  bool sendCommand = false;
  char command;
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - previousMillisTCS;
  if (elapsedMillis > interval) {
    command = "A";
    sendCommand = true;
    previousMillisTCS = currentMillis; // Reset the timer
  }
  else if (elapsedMillis < interval && elapsedMillis > interval/2) {
    command = "B";
    sendCommand = true;
    previousMillisTCS = currentMillis; // Reset the timer
  }
  if (sendCommand){
    Serial1.print(START_MARKER);
    Serial1.print(ADDR_TCS);
    Serial1.print(command);
    Serial1.print(END_MARKER);
  }
  }
void getEpsTelem() {
  // Gather telem from EPS system
}
void sendEpsCommands(){
  // Send commands to EPS system
}
