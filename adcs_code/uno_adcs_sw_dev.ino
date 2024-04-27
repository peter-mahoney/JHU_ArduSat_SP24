/* ADCS Code 675.701 Spring 2024 - Greg May
 * This code is modified from the original owner and updated for application specific purposes & increased functionality
 * Updates include:
 * - Addition of Photodiode sensing for orientation toward "Sun"
 * - Alterations due to HW differences
 * - UART Interface to CD&H Control System for commanding purposes
 * - Telemtry reporting to CD&H Control System
 * Original code base:
 * Charles GRASSIN, 2021
 * https://charleslabs.fr/en/project-Reaction+Wheel+Attitude+Control
 */

#define SERIAL_DEBUG_ENABLE 0 /* 0 = disable, 1 = enable */
#define MPU6050_CALIBRATION 1 /* 0 = disable, 1 = enable */
#define CONTROLLER_MODE 0 /* 0 = Speed stabilization only, 1 = Speed and Attitude stabilization, 2 = same as 1, but change set point every N secondes */
#define TELEMETRY_DEBUG_ENABLE 0 /* 0 = disable, 1 = enable, sends telemtry over serial monitor for debugging */
#define CDH_TELEMETRY 1 /* 0 = disable, 1 = enable, sends telemetry to MEGA CDH system */

//-----CD&H Interface-----
#include <stdio.h>
#include <string.h>
#include "config.h"
#define ADDR_ADCS "ADCS_ADDRS"
char myAddress;
unsigned long previousMillis = 0;

// -------PINS-------
#define PIN_DIR    2
#define PIN_STEP   3
#define PIN_SLEEP  4
#define PIN_LED    13
const int sensorPins[4] = {A0, A1, A2, A3};
// ------------------

// -----STEPPER------
#include "AccelStepper.h" // This is a hacked version of this library to allow
                          // speed control instead of position control.
AccelStepper myStepper(1, PIN_STEP, PIN_DIR);
double motorSpeed = 0;
#define MICROSTEPPING 4 /* 1 or 2 or 4 or 8 or 16 or 32 */
#define ACCELERATION 1750 /* Steps per s was 1750 */
#define MAX_SPEED (300 * MICROSTEPPING)
// ------------------

// -------PID--------
#include "PID.h"
const double P_TERM = 0.050 * MICROSTEPPING;
const double I_TERM = 0.000 * MICROSTEPPING;
const double D_TERM = 0.017 * MICROSTEPPING; 
PIDController pidSpeed(P_TERM, I_TERM, D_TERM);
PIDAngleController pidAttitude(2.5, 0, 400);
// ------------------

// ------MPU6050-----
#include <Wire.h>
#define MPU_ADDRESS 0x68   // I2C address of the MPU-6050
double GYRO_ERROR = 61.96; // rollingAvg error compensation
double yawAngle=0, yawAngularSpeed=0;
float GyX;
float GyY;
float GyZ;
// ------------------

// ------GENERAL-----
long timeCur, timePrev, timeStart, dwellTime; 
const int numReadings= 5;
double readings[numReadings];
int readIndex = 0;
double total = 0, rollingAvg = 0;
double targetAttitude = 0;
const int desiredLightLevel = 80; // Desired light level indicating the sun is facing sensor 1
bool rwEnabled;
int sensorValue1;
int sensorValue2;
int sensorValue3;
int sensorValue4;
int maxLightValue = 0; // Store the max light value
int maxSensorIndex = 0; // Store the sensor with the max light value
// ------------------

void setup() {
  #if SERIAL_DEBUG_ENABLE == 1
    Serial.begin(9600);
    delay(500);
    Serial.println("Serial Enabled");
  #endif

//Enables Comms back to C&DH Subsystem
  #if CDH_TELEMETRY == 1
      Serial.begin(9600);
      for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        if (addrs[i].name == ADDR_ADCS) {
            myAddress=addrs[i].address;
            break;
    }
  }
  #endif
  
  // Gyro setup
  Wire.begin();
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // wakes up the MPU-6050
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x1B); // GYRO_CONFIG register
  Wire.write(0x10); // 1000dps full scale
  Wire.endTransmission(true);

  // Gyro cal (only if selected, otherwise use saved value)
  #if MPU6050_CALIBRATION == 1
    calibrateMPU();
    Serial.println("calibrating MPU.");
  #endif

  // LED
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,1);
  
  // Initial stepper parameters
  myStepper.setEnablePin (PIN_SLEEP);
  rwEnabled=false;
  myStepper.disableOutputs();
  myStepper.setAcceleration(ACCELERATION);
  setSpeedStepper(0);
  
  timeCur = millis();
  timeStart = timeCur;
  dwellTime = millis();
}

// FSM variables
byte controllerState = 0;
int counts = 0;

// Main loop
void loop() {
 myStepper.run();
 int sensor1Value = analogRead(sensorPins[0]); // Read light level from sensor 1

  // If sensor 1 is not yet pointing towards the sun, rotate the motor
  if (sensor1Value < desiredLightLevel) {
    // Read light values from all sensors and find the max
    maxLightValue = 0;
    for (int i = 0; i < 4; i++) {
      int sensorValue = analogRead(sensorPins[i]);
      if (sensorValue > maxLightValue) {
        maxLightValue = sensorValue;
        maxSensorIndex = i;
      }
    }

    // Set motor direction based on which sensor has the highest light reading
    if (maxSensorIndex != 0 && rwEnabled == true) {
      // Determine rotation direction: clockwise or counterclockwise
      digitalWrite(PIN_DIR, maxSensorIndex < 2 ? HIGH : LOW); // Simplified: adjust for your setup
      if (maxSensorIndex < 2){
        motorSpeed = MAX_SPEED;
      }
      else{
        motorSpeed = -MAX_SPEED;
      }
     
      // Make the motor step
      if (rwEnabled == true){
        setSpeedStepper(motorSpeed);
        // Pulse stepper
        while (millis()-dwellTime < 1000){
          myStepper.run();
        }
        dwellTime = millis();
      }
      }
      // digitalWrite(PIN_STEP, HIGH);
      // delayMicroseconds(800); // Speed control
      // digitalWrite(PIN_STEP, LOW);
      // delayMicroseconds(800); // Speed control
    }
  
  else {
    if (motorSpeed > 500 || motorSpeed < -500){
      motorSpeed = 0;
    }
    // If the desired light level is reached, stabilize S/C in desired orientation using IMU
    // Every 10ms, read MPU and call controllers
    if(millis() - timeCur > 10) {
      timePrev = timeCur;
      timeCur = millis();

      // Measure Gyro value
     yawAngularSpeed = ((double)readMPU()-GYRO_ERROR) / 32.8;
     yawAngle += (yawAngularSpeed * (timeCur - timePrev) / 1000.0);
     // Put angle between -180 and 180
     while (yawAngle <= -180) yawAngle += 360; 
     while (yawAngle > 180)   yawAngle -= 360;

     // Low Pass Filter the angular speed (https://www.arduino.cc/en/Tutorial/BuiltInExamples/Smoothing)
      total = total - readings[readIndex]; 
      readings[readIndex] = yawAngularSpeed;
      total = total + readings[readIndex];
     readIndex = readIndex + 1;
     if (readIndex >= numReadings) {
        readIndex = 0;
        rollingAvg = total / numReadings; 
     }
      // Compute controller output
      #if CONTROLLER_MODE == 0
        // Detumbling only
        motorSpeed += pidSpeed.compute(0,rollingAvg,timeCur - timePrev);
      if (CONTROLLER_MODE == 1 || CONTROLLER_MODE == 2)
        // Change set point
        #if CONTROLLER_MODE == 2
         counts ++;
         if(counts == 250) {
           counts = 0;
            if(targetAttitude == 0)
             targetAttitude = 180;
            else
             targetAttitude = 0;
         }
        #endif
      #endif
      
      // FSM transition
      if(controllerState == 1 &&  fabs(rollingAvg) > 360 /* °/s */){
        controllerState = 0;digitalWrite(PIN_LED,0);
      }
      else if(controllerState == 0 && fabs(rollingAvg) < 45 /* °/s */) {
        controllerState = 1;
      }
      
      //FSM action
      if(controllerState == 0){
        motorSpeed += pidSpeed.compute(0,rollingAvg,timeCur - timePrev);
        pidAttitude.compute(targetAttitude,yawAngle,timeCur - timePrev);
      }
      else {
        motorSpeed += pidSpeed.compute(pidAttitude.compute(targetAttitude,yawAngle,timeCur - timePrev),rollingAvg,timeCur - timePrev);
      }

      // Constrain speed to valid interval (saturation)
    if(motorSpeed > MAX_SPEED) {
      motorSpeed = MAX_SPEED;
    }
    else if (motorSpeed < -MAX_SPEED)  {
      motorSpeed = -MAX_SPEED;
    }

    if (rwEnabled == true){
     setSpeedStepper(motorSpeed);
    }
  }
  }
      // Read commands and send unsolicited telem to mega every 5 seconds
    #if CDH_TELEMETRY == 1
      collectData();
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= TLM_PERIOD) {
        delay(10);
        char commandRead = readCommand();
        // ! means it wasn't a command received
        if (commandRead != '!') {
          processCommand(commandRead);
        }
        sendTlm();
        previousMillis=currentMillis;
      }
    #endif

     // Report attitude and speed
      #if SERIAL_DEBUG_ENABLE == 1
        Serial.print("Yaw Angle: ");
        Serial.print(yawAngle);
        Serial.print("   Rolling Avg: ");
        Serial.println(rollingAvg);
      #endif

    // Telemetry Report of Photodiode light levels and IMU Gyroscopic Measurments to serial monitor for debugging purposes
      #if TELEMETRY_DEBUG_ENABLE == 1
        SunSenseTelemetry();
        MPUtelemetry();
      #endif
    }

void collectData(){
  sensorValue1 = analogRead(A0);
  sensorValue2 = analogRead(A1);
  sensorValue3 = analogRead(A2);
  sensorValue4 = analogRead(A3);
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDRESS,6,true);
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void sendTlm() {
  // Gather telem from system and send
  int rwEnabledInt = rwEnabled ? 1 : 0;  // Convert bool to int (1 for true, 0 for false)
  char GyXstr[8];
  dtostrf(GyX,6,2,GyXstr);
  char GyYstr[8];
  dtostrf(GyY,6,2,GyYstr);
  char GyZstr[8];
  dtostrf(GyZ,6,2,GyZstr);
  char motorSpeedstr[8];
  dtostrf(motorSpeed,6,2,motorSpeedstr);
  // Buffer to hold the CSV string (adjust the size as needed)
  char csvADCSPacket[70];
  // Format the data into the CSV string
  snprintf(csvADCSPacket, sizeof(csvADCSPacket), "%d,%d,%d,%d,%d,%s,%s,%s,%s", sensorValue1, sensorValue2, sensorValue3, sensorValue4, rwEnabledInt, motorSpeedstr, GyXstr, GyYstr, GyZstr);
  // Send data across UART to MEGA
  Serial.print(START_MARKER);
  Serial.print(myAddress);
  Serial.print(TLM_TYPE);
  Serial.print(csvADCSPacket);
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
  for (int i = 0; i < sizeof(adcsCmds)/ sizeof(adcsCmds[0]); i++) {
        if (adcsCmds[i].id == commandId) {
            commandConfirmed = true;
            commandName = adcsCmds[i].name;
            break; // Character found in the list
        }
    }
  // Character not found in the list
  if (!commandConfirmed){
    Serial.print(START_MARKER);
    Serial.print(myAddress);
    Serial.print(LOG_TYPE);
    Serial.print("Command not found by ADCS.");
    Serial.print(END_MARKER);
    return; 
  }
  // Execute command and send log back to MEGA
  switch (commandId) {
    // Enable stepper motor
    case 'A':
      rwEnabled = true;
      // digitalWrite(PIN_SLEEP, HIGH);
      myStepper.enableOutputs();
      break;
    // Disable stepper motor
    case 'B':
      rwEnabled = false;
      myStepper.stop();
      // digitalWrite(PIN_SLEEP, LOW);
      myStepper.disableOutputs();
      break;
    default:
      return;
  }
    Serial.print(START_MARKER);
    Serial.print(myAddress);
    Serial.print(LOG_TYPE);
    Serial.print("ADCS processed command: ");
    Serial.print(commandName);
    Serial.print(END_MARKER);
  // can add C and D eventually
  }


// Set the current speed and direction of the motor
void setSpeedStepper(double targetSpeed){
  if(targetSpeed > 0)
    myStepper.moveTo(1000000);
  else 
    myStepper.moveTo(-1000000);

  myStepper.setMaxSpeed(fabs(targetSpeed));
}

// Read a yaw angular speed value
int16_t readMPU(){
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDRESS,2,true);
  return Wire.read()<<8|Wire.read();  // 0x43 (GYRO_ZOUT_H) & 0x44 (GYRO_ZOUT_L)
}

//Reports Suns Sensor Telemetry
void SunSenseTelemetry() {
   // Read the input on analog pins A0 through A3:
  sensorValue1 = analogRead(A0);
  sensorValue2 = analogRead(A1);
  sensorValue3 = analogRead(A2);
  sensorValue4 = analogRead(A3);
  // Print the values to the Serial Monitor:
  Serial.print("Sensor 1: ");
  Serial.print(sensorValue1);
  Serial.print("\tSensor 2: ");
  Serial.print(sensorValue2);
  Serial.print("\tSensor 3: ");
  Serial.print(sensorValue3);
  Serial.print("\tSensor 4: ");
  Serial.println(sensorValue4);
}

//Reports MPU Telemetry
void MPUtelemetry(){
Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDRESS,6,true);
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  Serial.print("GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
}

// Calibrate the gyro by doing CALIBRATION_MEASUREMENTS_COUNT measurements
#define CALIBRATION_MEASUREMENTS_COUNT 200
void calibrateMPU(){
  GYRO_ERROR = 0;
  for(int i=0;i<CALIBRATION_MEASUREMENTS_COUNT;i++){
    GYRO_ERROR += readMPU();
    delay(20);
  }
  GYRO_ERROR = GYRO_ERROR/(double)CALIBRATION_MEASUREMENTS_COUNT;
  #if SERIAL_DEBUG_ENABLE == 1
    Serial.println(GYRO_ERROR);
  #endif
}
