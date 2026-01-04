#pragma once
#include <string>
#include <sstream>
#include <iomanip>

static std::string format_time(const HeaterTime &t) {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << static_cast<int>(t.hour) << ":"
        << std::setw(2) << static_cast<int>(t.minute);
    return oss.str();
}

static std::string format_heater_status_pretty(const HeaterStatus &s) {
    std::ostringstream o;

    o << "HeaterStatus: "
      << "resp=" << to_string(s.response_to)
      << ", power=" << (s.is_powered_on ? "on" : "off")
      << ", mode=" << to_string(s.run_mode)
      << ", step=" << to_string(s.run_step)
      << ", target=" << int(s.run_target_temperature_c) << "C (" << int(s.run_target_temperature)
      << to_string(s.temperature_unit) << ")"
      << ", gear=" << int(s.run_current_gear)
      << ", V=" << std::fixed << std::setprecision(2) << s.voltage
      << ", room=" << s.temperature_room << "C"
      << ", shell=" << s.temperature_shell << "C"
      << ", temp-comp=" << int(s.temperature_compensation) << to_string(s.temperature_unit)
      << ", auto-start=" << format_time(s.auto_on_start)
      << ", auto-dur=" << s.auto_on_duration << "m"
      << ", auto-enabled=" << (s.auto_on_enabled ? "yes" : "no")
      << ", time-last-set=" << format_time(s.time_last_set_at)
      << ", co-sensor: " << (s.co_sensor_present ? "yes" : "no")
      << ", co-ppm=" << s.co_ppm
      << ", alt=" << s.altitude
      << ", alt-unit=" << to_string(s.altitude_unit)
      << ", lang=" << to_string(s.language)
      << ", tank=" << int(s.tank_volume)
      << ", pump=" << int(s.pump_type)
      << ", bright=" << int(s.brightness)
      << ", part=" << s.part_number
      << ", mbv=" << int(s.motherboard_version)
      << ", err=[0x" << std::hex << std::setw(2) << std::setfill('0') << int(s.error_code_55)
      << ", 0x" << std::setw(2) << int(s.error_code_66) << std::dec << "]"
      << std::endl;

    return o.str();
}
