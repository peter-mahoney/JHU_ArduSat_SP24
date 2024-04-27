/// Packages and Libraries
#include <Adafruit_INA260.h>

Adafruit_INA260 ina260 = Adafruit_INA260();

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Declared Input Pins and Global Variables
int motorPin = 9;
int heaterPin = 8;
unsigned long motorOnDuration;
unsigned long motorOffDuration;
unsigned long heaterOnDuration;
unsigned long heaterOffDuration;
unsigned long motorCountDownOff;
unsigned long motorCountdownOn;
///////////////////////////////////////////////////////////////////////////////////////////////////


/// Pre-initialization
void setup() {
  Serial.begin(9600);

  /// Multimeter (INA260 Monitor) Sequence
  // Wait until serial port is opened
  while (!Serial) { delay(10); }

  Serial.println("Adafruit INA260 Test");

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1)
      ;
  }
  Serial.println("Found INA260 chip");

  // Motor Duration
  motorOffDuration = 6000;
  motorOnDuration = 6000;
  // Heater Duration
  heaterOnDuration = 1000;
  heaterOffDuration = 2000;

  // Motor Countdown
  motorCountDownOff = 0*motorOnDuration - motorOffDuration; // T-minus milliseconds until Off


}
///////////////////////////////////////////////////////////////////////////////////////////////////
/// Fundamental Functions
void motorRun(unsigned long motorOnDuration, unsigned long motorOffDuration){
  Serial.println("Motor Status: OFF");
  digitalWrite(motorPin, LOW);
  delay(motorOffDuration);
  Serial.println("Motor Status: ON");
  digitalWrite(motorPin, HIGH);
  delay(motorOnDuration);
}

void heaterRun(unsigned long heaterOnDuration, unsigned long heaterOffDuration){
  Serial.println("Heater Status: OFF");
  digitalWrite(heaterPin, LOW);
  delay(heaterOffDuration);
  Serial.println("Heater Status: ON");
  digitalWrite(heaterPin, HIGH);
  delay(heaterOnDuration);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Initialization
void loop() {
  delay(100);

  /// Power Monitoring
  Serial.print("Current: ");
  Serial.print(ina260.readCurrent());
  Serial.println(" mA");

  Serial.print("Bus Voltage: ");
  Serial.print(ina260.readBusVoltage()/1000);
  Serial.println("V");

  Serial.print("Power: ");
  Serial.print(ina260.readPower()/1000);
  Serial.println(" W");

  Serial.println();
  delay(1000);


  // Reaction Wheel Operation
  motorRun(motorOnDuration,motorOffDuration);

  // for (int i = motorCountDownOff; i >= 0; i--) {
  //   // Print the current countdown value
  //   Serial.print(i);
  //   Serial.println(" seconds remaining...");
  //   delay(100);
  // }

  delay(100);
}
///////////////////////////////////////////////////////////////////////////////////////////////////



