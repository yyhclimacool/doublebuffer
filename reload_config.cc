/*
 * @file reload_config.cc
 * @author yyhclimacool@gmail.com
 * @date 2020-09-15
 * @brief reload config
 */

#include "double_buffer.h"

const char *ReloadConfigModeStr[ReloadConfig::MODE_NUM] = {
  "ACTIVE_MODE",
  "PASSIVE_MODE",
};

const char *ReloadConifgMonitorTypeStr[ReloadConfig::MONITOR_TYPE_NUM] = {
  "MONITOR_FILE",
};

std::ostream &operator<<(std::ostream &os, const ReloadConfig &config) {
  return os << "[mode=" << ReloadConfigModeStr[config.mode_] 
    << ", monitor_type=" << ReloadConifgMonitorTypeStr[config.monitor_type_] 
    << ", monitor_interval=" << config.monitor_interval_ 
    << ", old_buffer_lives_period=" << config.old_buffer_lives_period_ 
    << ", filename=" << config.filename_ << "]";
}
