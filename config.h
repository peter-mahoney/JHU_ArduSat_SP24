// config.h

#ifndef CONFIG_H
#define CONFIG_H
// Packet start and end markers
#define START_MARKER '<'
#define END_MARKER   '>'
#define TLM_TYPE   'T'
#define CMD_TYPE   'C'
#define LOG_TYPE   'L'
// Telem publish period (msec)
#define TLM_PERIOD 5000

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
    {"PANEL_TEMP_1_C", 'A'},
    {"PANEL_TEMP_2_C", 'B'},
    {"TEMP_BOARD_C", 'C'},
    {"HEATERS_ENABLED", 'D'},
    {"HEATER_1_ON", 'E'},
    {"HEATER_2_ON", 'F'},
    {"HEATER_RAIL_AMPS", 'G'},
    {"HEATER_RAIL_VOLTS", 'H'},
    {"HEATER_RAIL_WATTS", 'I'},
    // Add more telemetry points as needed...
};

// Define commands
const Commands tcsCmds[] = {
    {"ENABLE_HEATERS", 'A'},
    {"DISABLE_HEATERS", 'B'},
    {"NOMINAL_SETPOINTS", 'C'},
    {"SURVIVAL_SETPOINTS", 'D'},
    // Add more commands as needed...
};
// Define telemetry points
const Telemetry adcsPoints[] = {
    {"PDIODE_A", 'A'},
    {"PDIODE_B", 'B'},
    {"PDIODE_B", 'C'},
    {"PDIODE_B", 'D'},
    {"MOTOR_ON", 'E'},
    {"IMU_ENABLED", 'F'},
    {"IMU_X_ACCEL", 'G'},
    {"IMU_Y_ACCEL", 'H'},
    {"IMU_Z_ACCEL", 'I'},

    // Add more telemetry points as needed...
};

// Define commands
const Commands adcsCmds[] = {
    {"ACTIVATE_RXWHEEL", 'A'},
    {"SET_SPEED_RXWHEEL", 'B'},
    {"DISABLE_RXWHEEL", 'C'},
    {"SET_IMU_HIGH", 'D'},
    {"SET_IMU_MID", 'E'},
    {"SET_IMU_LOW", 'F'},
    // Add more commands as needed...
};
// Define telemetry points
const Telemetry epsPoints[] = {
    {"MPPT_OUT_VOLTS", 'A'},
    {"MPPT_OUT_AMPS", 'B'},
    {"MPPT_OUT_POWER", 'C'},
    {"BOOST_OUT_VOLTS", 'D'},
    {"BOOST_OUT_AMPS", 'E'},
    {"BOOST_OUT_POWER", 'F'},
    {"BATT_SOC", 'G'},
    {"BATT_VOLTS", 'H'},
    {"BATT_ENABLED", 'I'},
    {"BATT_CHARGING", 'J'},
    {"RAIL_ENABLED", 'K'},
    // Add more telemetry points as needed...
};
const Commands epsCmds[] = {
    {"ENABLE_BATTERY", 'A'},
    {"DISABLE_BATTERY", 'B'},
    {"ENABLE_RAIL", 'C'},
    {"DISABLE_RAIL", 'D'},
    // Add more commands as needed...
};
// Define commands
const Commands cdhCmds[] = {
    {"RESET_TCS", 'A'},
    {"RESET_ADCS", 'B'},
    {"RESET_TTC", 'C'},
    {"RESET_ALL", 'D'},
    // Add more commands as needed...
};
// Define rules
const AutonomyRules cdhAutonomy[] = {
    {"TCS_WATCHDOG", 'A'},
    {"ADCS_WATCHDOG", 'B'},
    {"TTC_WATCHDOG", 'C'},
    {"OVERTEMP", 'D'},
    {"ADCS_FAULT", 'E'},
    // Add more rules as needed...
};
const AutonomyRules epsAutonomy[] = {
    {"UNSAFE_BATT_SOC", 'A'},
    {"LOW_RAIL_VOLTAGE", 'B'},
    {"HIGH_RAIL_VOLTAGE", 'C'},
    // Add more rules as needed...
};
#endif // End of CONSTANTS_H
