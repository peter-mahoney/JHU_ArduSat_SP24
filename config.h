// config.h

#ifndef CONFIG_H
#define CONFIG_H
// Packet start and end markers
#define START_MARKER '<'
#define END_MARKER   '>'
#define TLM_TYPE   'T'
#define CMD_TYPE   'C'
#define LOG_TYPE   'L'

// Define address struct
struct Addresses {
    const char* name;  // Address name (string)
    char address;      // Shorter identifier (char or hexadecimal)
};

// Define packet type struct
struct PacketTypes {
    const char* name;  // Packet type (string)
    char type;      // Shorter identifier (char or hexadecimal)
};
// Define telemetry struct
struct Telemetry {
    const char* name;  // Telemetry point name (string)
    char id;           // Shorter identifier (char or hexadecimal)
};

// Define command struct
struct Commands {
    const char* name;  // Command name (string)
    char id;           // Shorter identifier (char or hexadecimal)
};

// Define autonomy rules struct
struct AutonomyRules {
    const char* name;  // Rule name (string)
    char id;           // Shorter identifier (char or hexadecimal)
};

// Define addresses
const Addresses addrs[] = {
    {"TCS_ADDRS", 'H'},
    {"ADCS_ADDRS", 'A'},
    {"MEGA_ADDRS", 'M'},
    {"TTC_ADDRS", 'T'},
    {"EPS_ADDRS", 'E'},
    // Add more addresses as needed...
};
// Define packet types
const PacketTypes pckts[] = {
    {"TLM_PACKET", 'T'},
    {"CMD_PACKET", 'C'},
    {"LOG_PACKET", 'L'},
    // Add more packet types as needed...
};

// Define telemetry points
const Telemetry tcsPoints[] = {
    {"TEMP_1", 'A'},
    {"TEMP_2", 'B'},
    {"TEMP_3", 'C'},
    {"HEATERS_ENABLED", 'D'},
    {"HEATERS_ON", 'E'},
    // Add more telemetry points as needed...
};

// Define commands
const Commands tcsCmds[] = {
    {"ENABLE_HEATERS", 'A'},
    {"DISABLE_HEATERS", 'B'},
    {"SURVIVAL_SETPOINTS", 'C'},
    {"NOMINAL_SETPOINTS", 'D'},
    // Add more commands as needed...
};
// Define telemetry points
const Telemetry adcsPoints[] = {
    {"RW_ON", 'A'},
    // Add more telemetry points as needed...
};

// Define commands
const Commands adcsCmds[] = {
    {"TURN_ON_RW", 'A'},
    {"TURN_OFF_RW", 'B'},
    // Add more commands as needed...
};
const Commands epsCmds[] = {
    {"ENABLE_PHOTODIODES", 'A'},
    // Add more commands as needed...
};
// Define commands
const Commands megaCmds[] = {
    {"RESET_TCS", 'A'},
    {"RESET_ADCS", 'B'},
    {"SURVIVAL_THERMAL", 'C'},
    {"NOMINAL_THERMAL", 'D'},
    // Add more commands as needed...
};
// Define rules
const AutonomyRules rules[] = {
    {"LOW_VOLTAGE_YELLOW", 'A'},
    {"LOW_VOLTAGE_RED", 'B'},
    {"TCS_WATCHDOG", 'C'},
    {"ADCS_WATCHDOG", 'D'},
    {"TTC_WATCHDOG", 'E'},
    {"OVERTEMP", 'F'},
    {"ADCS_FAULT", 'G'},
    // Add more rules as needed...
};
#endif // End of CONSTANTS_H
