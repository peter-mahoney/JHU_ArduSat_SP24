# JHU_ArduSat Code Repository
## Background:
- This code repository was used to develop software for the JHU Space Systems Engineering ArduSat Project during the Spring 2024 semester.
- The repository may not be actively maintained after Spring 2024, but the code should remain accessible and applicable on Arduino platforms (with the right sensors included).
- ArduSat (as implemented by the SP'24 project team) is an Arduino based development satellite testbed, using a centralized Arduino MEGA main flight computer along with Arduino UNO subsystem peripherals. ArduSat is not space rated and intended as a educational and testing platform.
- Subsystems include: CDH (command and data handling), FSW (Flight Software), TTC (for wireless communication between "ground" and ArduSat), TCS (thermal control), ADCS (attitude control), EPS (power system), and Structures (mechanical housing). 
- Ground side commanding of ArduSat over TTC is implemented via the Digi XCTU SW application and a basic telemetry monitoring dashboard has been developed in MATLAB
- A block diagram of the ArduSat system as implemented in Spring 24 can be found below:
![ArduSatBlockDiagram drawio](https://github.com/peter-mahoney/JHU_ArduSat/assets/39682607/58a8b349-9ea4-4744-bfe3-efe457230660)

## Code Descriptions:
- `mega_core_sw_dev.ino` is the core FSW code which runs on the central processor of ArduSat (an Arduino MEGA).
- The central processor is responsible for:
  1. Collecting and routing subsystem messages (telemetry and logs) to the TTC subsystem for ground transmission.
  2. Collecting and routing ground commands from the TTC subsystem to the desired subsystem processor
  3. Monitoring the overall health of the spacecraft, including maintaining watchdog timers
  4. EPS (power subsystem) software is also hosted on the central processor, which monitors the health of the solar arrays and batteries.
- A functional block diagram of this code can be found below.

![SmallMegaFSWBlockDiagram drawio](https://github.com/peter-mahoney/JHU_ArduSat/assets/39682607/e49d6327-d1e0-4e75-aa20-cef38da7dd43)

- `uno_<subsystem>_sw_dev.ino` files contain the subsystem SW running on the node/peripheral Arduino UNOs. These files contain both the standard SW interfaces between the subsystem processors and central processor as well as the subsystem specific service loops used to drive elements of the subsystem.
- Generic subsystem SW responsibilites include:
  1. Collecting and sending telemetry packets to the central processor at a configurable, unsolicted cadence (nominally at 0.1 Hz)
  2. Checking for commands from the central processor at every loop through the SW and processing received commands
  3. Generating log messages when commands are successfully processed and transmitting to the central processor.
  4. Implementing fault management by monitoring telmetry and generating subsystem level commands when rules are triggered.
  5. Graceful recovery from central processor commanded resets.
   
  ![SmallUnoFSWBlockDiagram drawio](https://github.com/peter-mahoney/JHU_ArduSat/assets/39682607/a23db181-d40a-432c-a62e-10aeb6b0c016)

- `config.h` is a config file which defines a common source of truth for the following elements of the ArduSat CDH/SW architecture:
  1. The standard protocol for message construction and transmission across the UART interfaces between processors.
  2. Mapping of subsystems to address identifiers used in the message protocols
  3. Mapping of command and telemetry point names/mnemonics to IDs (IDs take the form of alphabet characters)
  
## Instructions for use:
- Within the flight_sw directory is the UNO and MEGA "firmware" code (`.ino` sketch files) which can be uploaded to Arduino boards via the Arduino IDE (or an IDE enabled editor like VS Code)
- Within the ground_sw directory is the MATLAB telemetry monitoring dashboard.
- Prior to compiling and uploading .ino sketches to your Arduino board(s), make sure the `config.h` file is in the same folder as each of the `.ino` sketch files on your machine or else the sketches will not be able to find the config (just copy the same file into each directory)
- groundSW.m contains the code for a telemetry viewer tool developed in MATLAB
-   This may require a dedicated ground terminal Arduino


  
