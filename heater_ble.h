#pragma once
#include "esphome.h"

enum class HeaterCommand : uint8_t {
    GET_STATUS = 0x01,
    SET_RUN_MODE = 0x02,
    SET_POWER = 0x03,
    SET_PARAMETER = 0x04,

    SET_CURRENT_TIME = 0x0A,

    SET_AUTO_ON_START_TIME = 0x0B,
    SET_AUTO_ON_RUN_DURATION = 0x0C,
    SET_AUTO_ON_ENABLED = 0x0D,

    SET_LANGUAGE = 0x0E,
    SET_TEMP_UNIT = 0x0F,
    SET_TANK_VOLUME = 0x10,
    SET_PUMP_TYPE = 0x11,
    SET_I_STOP = 0x12,
    SET_ALTITUDE_UNIT = 0x13,
    SET_TEMP_COMPENSATION = 0x14,
    SET_BRIGHTNESS = 0x15,
};

enum class RunMode : uint8_t {
    GEAR = 0x01,
    TEMPERATURE = 0x02,
    VENTILATION = 0x03,
};

enum class RunStep : uint8_t {
    IDLE = 0x00,
    SELF_TEST = 0x01,
    IGNITION = 0x02,
    HEATING = 0x03,
    COOLING = 0x04,
    STANDBY = 0x05,
    VENTILATION = 0x06,
};

enum class Language : uint8_t {
    ENGLISH = 0x00,
    CHINESE = 0x01, // note: may be disabled by firmware, in which case GERMAN will be set instead of CHINESE
    RUSSIAN = 0x02,
    OFF = 0x03,
    GERMAN = 0x04,
};

enum class TemperatureUnit : uint8_t {
    CELSIUS = 0x00,
    FAHRENHEIT = 0x01,
};

enum class AltitudeUnit : uint8_t {
    KILOMETERS = 0x00,
    KILOFEET = 0x01,
    OFF = 0xFF,
};

struct HeaterTime {
    uint8_t hour;
    uint8_t minute;
};

struct HeaterStatus {
    HeaterCommand response_to;
    bool is_powered_on;

    uint8_t error_code_55;
    uint8_t error_code_66;

    RunMode run_mode;
    RunStep run_step;
    uint8_t run_target_temperature;
    uint8_t run_target_temperature_c;
    uint8_t run_current_gear;

    float voltage;

    float altitude;
    AltitudeUnit altitude_unit;

    float temperature_room;
    float temperature_shell;
    TemperatureUnit temperature_unit;
    int8_t temperature_compensation;

    HeaterTime time_last_set_at;
    HeaterTime auto_on_start;
    uint16_t auto_on_duration;
    bool auto_on_enabled;

    Language language;
    uint8_t tank_volume;
    uint8_t pump_type;
    uint8_t unknown_auto_setting;
    uint8_t brightness;

    bool co_sensor_present;
    uint16_t co_ppm;

    uint32_t part_number;
    uint8_t motherboard_version;
};

static int8_t _get_s8(const std::vector<uint8_t> &data, size_t i) {
    return static_cast<int8_t>(data[i]);
}

static uint16_t _get_u16(const std::vector<uint8_t> &data, size_t high, size_t low) {
    return uint16_t(data[high]) << 8 | uint16_t(data[low]);
}

static int16_t _get_s16(const std::vector<uint8_t> &data, size_t high, size_t low) {
    return static_cast<int16_t>(_get_u16(data, high, low));
}

static HeaterTime _get_time(const std::vector<uint8_t> &data, size_t high, size_t low) {
    uint16_t minutes_from_midnight = _get_u16(data, high, low);
    return {
        static_cast<uint8_t>(minutes_from_midnight / 60),
        static_cast<uint8_t>(minutes_from_midnight % 60),
    };
}

/*
BLE Protocol Status Frame (48 bytes) - AA55/AA66 Protocol

+=======+========================+===========+============+========+===================+========================================================================+
| Bytes | Field Name             | Type      | Endianness | Scale  | Range             | Notes                                                                  |
+=======+========================+===========+============+========+===================+========================================================================+
| 0-1   | Header                 |           |            |        | 0xAA55, 0xAA66    |                                                                        |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 2     | Command                | uint8_t   |            |        | 0x00-0x15         | The command this packet is responding to                               |
+-------+------------------------+-----------+------------+--------+-----------------+--------------------------------------------------------------------------+
| 3     | Power State            | uint8_t   |            |        | 0, 1              | 0=Off, 1=On                                                            |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 4     | Error Code (0x55)      | uint8_t   |            |        | 0-255             | Error code for  0x55 frame                                             |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 5     | Run Step               | uint8_t   |            |        | 0-6               | 0=Idle, 1=Self-test, 2=Ignition, 3=Heating,                            |
|       |                        |           |            |        |                   | 4=Cooling, 5=Standby, 6=Ventilation                                    |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 6-7   | Altitude               | uint16_t  | LE         | 0.1    | 0-6553.5          | Altitude in meters or feet (×0.1)                                      |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 8     | Run Mode               | uint8_t   |            |        | 1, 2, 3           | 1=Gear, 2=Temperature, 3=Auto/Ventilation (likely only supports 1 & 2) |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 9     | Target Temp/Gear       | uint8_t   |            |        | 8-36, 46-97, 1-10 | °C (8-36), °F (46-97), or Gear (1-10) depending on byte 27 / mode      |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 10    | Gear Level             | uint8_t   |            |        | 1-10              | Current power/gear level                                               |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 11-12 | Input Voltage          | uint16_t  | BE         | 0.1    | 0-6553.5          | Supply voltage in volts (×0.1)                                         |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 13-14 | Shell Temperature      | int16_t   | BE         | 0.1    | -3276.8 to 3276.7 | Heater Core temp in °C (ignores byte 27 setting)                       |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 15-18 | Unknown                | uint8_t[] |            |        | -                 | Likely Padding                                                         |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 19-20 | Start Time             | uint16_t  | BE         |        | 0-1439            | Last time set, in minutes since midnight                               |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 21-22 | Auto On Time           | uint16_t  | BE         |        | 0-1439            | Scheduled start time in minutes since midnight                         |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 23-24 | Run Duration           | uint16_t  | BE         |        | 30-630            | Scheduled run duration in minutes, 30 min step                         |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 25    | Auto Timer Enabled     | uint8_t   |            |        | 0, 1              | Schedule 0=Disabled, 1=Enabled                                         |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 26    | Language               | uint8_t   |            |        | 0-4               | 0=EN, 1=CN, 2=RU, 3=off, 4=DE (CN may be disabled)                     |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 27    | Temperature Unit       | uint8_t   |            |        | 0-1               | 0=°C, 1=°F                                                             |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 28    | Tank Volume            | uint8_t   |            | 5L     | 0-10, ≥20, 255    | 0=Not set, 1-10 = 5L-50L (×5L), ≥20=BT flag, 255=Not configured        |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 29    | Pump Type              | uint8_t   |            |        | 0-3, ≥20, 255     | 0-3=Pump model (16/22/28/32ul), ≥20=RF433 flag, 255=Not configured     |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 30    | Altitude Unit          | uint8_t   |            |        | 0-1, 0xFF         | 0=Km, 1=Kft, 0xFF=Disabled                                             |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 31    | Auto Setting           | uint8_t   |            |        |                   | Unknown purpose                                                        |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 32-33 | Room Temperature       | int16_t   | BE         | 0.1    |                   | Room temp in °C                                                        |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 34    | Temp Compensation      | int8_t    |            |        | -9 to +9          | Temperature compensation offset selected unit (byte 27)                |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 35    | Error Code (0x66)      | uint8_t   |            |        |                   | Error code for 0x66 frame                                              |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 36    | Brightness             | uint8_t   |            |        | 0-255             | 1-10, 20-100 (×10), 200=auto, 201-208=enhanced, 0/255=off              |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 37    | CO Sensor Present      | uint8_t   |            |        | 0, 1              | 0=No CO sensor, 1=CO sensor present                                    |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 38-39 | CO Level               | uint16_t  | LE         |        | 0-65535           | CO concentration in PPM (only valid if byte 37=1)                      |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 40-43 | Part Number            | uint32_t  | LE         |        | 0-4294967295      | Device part/serial number                                              |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 44    | Motherboard Version    | uint8_t   |            |        | 0, 3, 4, 5        | HW revision: 0=basic, 3/4/5=advanced features                          |
+-------+------------------------+-----------+------------+--------+-------------------+------------------------------------------------------------------------+
| 45-47 | Padding                |           |            |        |                   |                                                                        |
+=======+========================+===========+============+========+===================+========================================================================+
*/
//
// AA.66.01.00.00.00.0C.E8.01.12.0A.00.00.00.00.00.00.00.00.03.B8.01.B3.00.1E.00.00.00.15.15.FF.00.FF.D8.FF.00.04.00.00.00.00.00.00.00.00.00.00.00 (48)
static bool heater_ble_parse_aa66_frame(const std::vector<uint8_t> &data, HeaterStatus &status) {
    status.response_to = static_cast<HeaterCommand>(data[2]);
    status.is_powered_on = data[3] != 0;
    status.error_code_55 = data[4];
    status.run_step = static_cast<RunStep>(data[5]);
    status.altitude = _get_u16(data, 6, 7) / 10.0f;
    status.run_mode = static_cast<RunMode>(data[8]);
    status.run_target_temperature = data[9];
    status.run_current_gear = data[10];
    status.voltage = _get_u16(data, 11, 12) / 10.0f;
    status.temperature_shell = _get_s16(data, 13, 14);
    status.time_last_set_at = _get_time(data, 19, 20);
    status.auto_on_start = _get_time(data, 21, 22);
    status.auto_on_duration = _get_u16(data, 23, 24);
    status.auto_on_enabled = data[25] > 0;
    status.language = static_cast<Language>(data[26]);
    status.temperature_unit = static_cast<TemperatureUnit>(data[27]);
    status.tank_volume = data[28];
    status.pump_type = data[29];
    status.altitude_unit = static_cast<AltitudeUnit>(data[30]);
    status.unknown_auto_setting = data[31];
    status.temperature_room = _get_s16(data, 32, 33) / 10.0f;
    status.temperature_compensation = _get_s8(data, 34);
    status.error_code_66 = data[35];
    status.brightness = data[36];
    status.co_sensor_present = data[37] > 0;
    status.co_ppm = _get_u16(data, 38, 39);
    status.part_number = data[43] << 24 | data[42] << 16 | data[41] << 8 | data[40]; // TODO verify LE
    status.motherboard_version = data[44];

    ESP_LOGI("BLE", "aunit: %d", data[30]);

    // convert units
    if (status.temperature_unit == TemperatureUnit::FAHRENHEIT) {
        status.run_target_temperature_c = roundf((status.run_target_temperature - 32) * 5 / 9);
    } else {
        status.run_target_temperature_c = status.run_target_temperature;
    }

    return true;
}

static std::vector<uint8_t> heater_ble_decrypt_frame(const std::vector<uint8_t> &data) {
    const uint8_t key[8] = {0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64};
    std::vector<uint8_t> decrypted(data.size());

    for (size_t i = 0; i < data.size(); i++) {
        decrypted[i] = data[i] ^ key[i % 8];
    }

    ESP_LOGD("BLE", "Decrypted frame: %s", format_hex_pretty(decrypted).c_str());
    return decrypted;
}

static bool heater_ble_parse_frame(const std::vector<uint8_t> &data, HeaterStatus &status) {
    ESP_LOGD("BLE", "Raw frame: %s", format_hex_pretty(data).c_str());
    if (data.size() != 48) { return false; }

    if (data[0] == 0xDA && data[1] == 0x07) {
        return heater_ble_parse_aa66_frame(heater_ble_decrypt_frame(data), status);
    } else if (data[0] == 0xAA && data[1] == 0x66) {
        return heater_ble_parse_aa66_frame(data, status);
    } else if (data[0] == 0xAA && data[1] == 0x55) {
        return heater_ble_parse_aa66_frame(data, status);
    } else {
        return false;
    }
}

static std::vector<uint8_t> heater_ble_payload(HeaterCommand cmd, uint8_t data = 0x00) {
    uint16_t passkey = id(heater_passkey);
    uint8_t pass_hi = passkey / 100;
    uint8_t pass_lo = passkey % 100;
    uint8_t checksum = (pass_hi + pass_lo + static_cast<uint8_t>(cmd) + data + 0x00) & 0xFF;

    return { 0xAA, 0x55, pass_hi, pass_lo, static_cast<uint8_t>(cmd), data, 0x00, checksum };
}

static std::vector<uint8_t> heater_ble_payload_16(HeaterCommand cmd, uint16_t data) {
    uint16_t passkey = id(heater_passkey);
    uint8_t pass_hi = passkey / 100;
    uint8_t pass_lo = passkey % 100;
    uint8_t data_low = data & 0xFF;
    uint8_t data_high = data >> 8;
    uint8_t checksum = (pass_hi + pass_lo + static_cast<uint8_t>(cmd) + data_low + data_high) & 0xFF;

    return { 0xAA, 0x55, pass_hi, pass_lo, static_cast<uint8_t>(cmd), data_low, data_high, checksum };
}

static const char *to_string(HeaterCommand v) {
    switch (v) {
        case HeaterCommand::GET_STATUS: return "GET_STATUS";
        case HeaterCommand::SET_RUN_MODE: return "SET_RUN_MODE";
        case HeaterCommand::SET_POWER: return "SET_POWER";
        case HeaterCommand::SET_PARAMETER: return "SET_PARAMETER";
        case HeaterCommand::SET_CURRENT_TIME: return "SET_CURRENT_TIME";
        case HeaterCommand::SET_AUTO_ON_START_TIME: return "SET_AUTO_ON_START_TIME";
        case HeaterCommand::SET_AUTO_ON_RUN_DURATION: return "SET_AUTO_ON_RUN_DURATION";
        case HeaterCommand::SET_AUTO_ON_ENABLED: return "SET_AUTO_ON_ENABLED";
        case HeaterCommand::SET_LANGUAGE: return "SET_LANGUAGE";
        case HeaterCommand::SET_TEMP_UNIT: return "SET_TEMP_UNIT";
        case HeaterCommand::SET_TANK_VOLUME: return "SET_TANK_VOLUME";
        case HeaterCommand::SET_PUMP_TYPE: return "SET_PUMP_TYPE";
        case HeaterCommand::SET_I_STOP: return "SET_I_STOP";
        case HeaterCommand::SET_ALTITUDE_UNIT: return "SET_ALTITUDE_UNIT";
        case HeaterCommand::SET_TEMP_COMPENSATION: return "SET_TEMP_COMPENSATION";
        case HeaterCommand::SET_BRIGHTNESS: return "SET_BRIGHTNESS";
        default: return "UNKNOWN_COMMAND";
    }
}

static const char *to_string(RunMode v) {
    switch (v) {
        case RunMode::GEAR: return "GEAR";
        case RunMode::TEMPERATURE: return "TEMPERATURE";
        case RunMode::VENTILATION: return "VENTILATION";
        default: return "UNKNOWN_RUN_MODE";
    }
}

static const char *to_string(RunStep v) {
    switch (v) {
        case RunStep::IDLE: return "IDLE";
        case RunStep::SELF_TEST: return "SELF_TEST";
        case RunStep::IGNITION: return "IGNITION";
        case RunStep::HEATING: return "HEATING";
        case RunStep::COOLING: return "COOLING";
        case RunStep::STANDBY: return "STANDBY";
        case RunStep::VENTILATION: return "VENTILATION";
        default: return "UNKNOWN_RUN_STEP";
    }
}

static const char *to_string(Language v) {
    switch (v) {
        case Language::ENGLISH: return "ENGLISH";
        case Language::CHINESE: return "CHINESE";
        case Language::RUSSIAN: return "RUSSIAN";
        case Language::OFF: return "DISABLED";
        case Language::GERMAN: return "GERMAN";
        default: return "UNKNOWN_LANGUAGE";
    }
}

static const char *to_string(TemperatureUnit v) {
    switch (v) {
        case TemperatureUnit::CELSIUS: return "C";
        case TemperatureUnit::FAHRENHEIT: return "F";
        default: return "?";
    }
}

static const char *to_string(AltitudeUnit v) {
    switch (v) {
        case AltitudeUnit::KILOMETERS: return "km";
        case AltitudeUnit::KILOFEET: return "kft";
        case AltitudeUnit::OFF: return "disabled";
        default: return "?";
    }
}