/*
 * @file reload_config.h
 * @author yyhclimacool@gmail.com
 * @date 2020-09-15
 * @brief configuration for double buffer
 */

#ifndef RELOAD_CONFIG_H
#define RELOAD_CONFIG_H

#include <string>
#include <iostream>

struct ReloadConfig {
  enum Mode {
    ACTIVE_MODE = 0, /* Start a reload thread background */
    PASSIVE_MODE,    /* Reload only asked for */
    MODE_NUM,
  };

  enum MonitorType {
    MONITOR_FILE = 0, /* Monitor file state */
    MONITOR_TYPE_NUM,
  };

  ReloadConfig() = default;
  ReloadConfig(Mode m, MonitorType type, int interval, 
    int period, const std::string &file = "")
    : mode_(m),
      monitor_type_(type),
      monitor_interval_(interval),
      old_buffer_lives_period_(period),
      filename_(file) {}

  Mode mode_ = ACTIVE_MODE;
  MonitorType monitor_type_;
  int monitor_interval_ = 60; /* in seconds */
  int old_buffer_lives_period_ = 10; /* sleep serveral seconds before clear old buffer */
  std::string filename_ = ""; /* if monitor_type_ == MONITOR_FILE, this field must be set. */
};

std::ostream &operator<<(std::ostream &os, const ReloadConfig &config);

#endif // RELOAD_CONFIG_H
