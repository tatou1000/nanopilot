#pragma once

#include <cstdlib>
#include <fstream>

#include <common/util.h>

#ifdef QCOM
#define Hardware HardwareEon
#elif QCOM2
#define Hardware HardwareTici
#else
#define Hardware HardwareNone
#endif


// no-op base hw class
class HardwareNone {
public:
  static constexpr float MAX_VOLUME = 0;
  static constexpr float MIN_VOLUME = 0;

  static std::string get_os_version() { return "openpilot for PC"; };

  static void reboot() {};
  static void poweroff() {};
  static void set_brightness(int percent) {};

  static bool get_ssh_enabled() { return false; };
  static void set_ssh_enabled(bool enabled) {};
};

class HardwareEon : public HardwareNone {
public:
  static constexpr float MAX_VOLUME = 1.0;
  static constexpr float MIN_VOLUME = 0.5;

  static std::string get_os_version() {
    return "NEOS " + util::read_file("/VERSION");
  };

  static void reboot() { std::system("reboot"); };
  static void poweroff() { std::system("LD_LIBRARY_PATH= svc power shutdown"); };
  static void set_brightness(int percent) {
    std::ofstream brightness_control("/sys/class/leds/lcd-backlight/brightness");
    if (brightness_control.is_open()) {
      brightness_control << (int)(percent * (255/100.)) << "\n";
      brightness_control.close();
    }
  };

  static bool get_ssh_enabled() {
    return std::system("getprop persist.neos.ssh | grep -qF '1'") == 0;
  };
  static void set_ssh_enabled(bool enabled) {
    std::string cmd = util::string_format("setprop persist.neos.ssh %d", enabled ? 1 : 0);
    std::system(cmd.c_str());
  };
};

class HardwareTici : public HardwareNone {
public:
  static constexpr float MAX_VOLUME = 0.5;
  static constexpr float MIN_VOLUME = 0.4;

  static std::string get_os_version() {
    return "AGNOS " + util::read_file("/VERSION");
  };

  static void reboot() { std::system("sudo reboot"); };
  static void poweroff() { std::system("sudo poweroff"); };
  static void set_brightness(int percent) {
    std::ofstream brightness_control("/sys/class/backlight/panel0-backlight/brightness");
    if (brightness_control.is_open()) {
      brightness_control << (percent * (int)(1023/100.)) << "\n";
      brightness_control.close();
    }
  };

  static bool get_ssh_enabled() { return Params().read_db_bool("SshEnabled"); };
  static void set_ssh_enabled(bool enabled) { Params().write_db_value("SshEnabled", (enabled ? "1" : "0")); };
};
