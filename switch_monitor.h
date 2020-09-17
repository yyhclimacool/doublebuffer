/*
 * @file switch_monitor.h
 * @author yyhclimacool@gmail.com
 * @date 2020-09-15
 * @brief switch monitor returns should do switch or not
 */

#ifndef SWITCH_MONITOR_H
#define SWITCH_MONITOR_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glog/logging.h>

#include "reload_config.h"

class SwitchMonitor {
public:
  virtual ~SwitchMonitor(){}
  virtual bool init(const ReloadConfig &) {
    LOG(FATAL) << "Not implemented.";
    return true;
  }

  virtual bool shouldSwitch() {
    LOG(FATAL) << "Not implemented.";
    return true;
  }

  virtual void doneSwitch() {
    LOG(FATAL) << "Not implemented.";
  }
};

class FileSwitchMonitor : public SwitchMonitor {
public:
  FileSwitchMonitor() = default;
  ~FileSwitchMonitor() {}

  virtual bool init(const ReloadConfig &cfg) override {
    if (cfg.monitor_type_ == ReloadConfig::MONITOR_FILE 
        && cfg.filename_.size() != 0) {
      filename_ = cfg.filename_;
      struct stat st;
      if (stat(filename_.c_str(), &st) != 0) {
        LOG(FATAL) << "Get stat of filename=" << filename_ << " failed. Check the file.";
        return false;
      }
    } else {
      LOG(FATAL) << "Init FileSwitchMonitor with ReloadConfig=" << cfg << " failed.";
      return false;
    }
    return true;
  }

  virtual bool shouldSwitch() override {
    struct stat st;
    if (stat(filename_.c_str(), &st) != 0) {
      LOG(ERROR) << "Get stat of filename=" << filename_ << " failed. Check the file.";
      return false;
    }

    time_t file_mod_ts = st.st_mtime;
    if (file_mod_ts > last_updated_ts_) {
      check_ts_ = file_mod_ts;
      return true;
    }

    return false;
  }

  virtual void doneSwitch() override {
    last_updated_ts_ = check_ts_;
  }
private:
  std::string filename_;
  time_t check_ts_ = 0;
  time_t last_updated_ts_ = 0;
};

using SwitchMonitorSPtr = std::shared_ptr<SwitchMonitor>;

class SwitchMonitorFactory {
public:
  static SwitchMonitorSPtr Create(const ReloadConfig &cfg) {
    switch(cfg.monitor_type_) {
      case (ReloadConfig::MONITOR_FILE):
        return std::make_shared<FileSwitchMonitor>();
        break;
      default:
        LOG(ERROR) << "Create SwitchMonitor with ReloadConfig=" << cfg << " failed.";
        break;
    }
    return nullptr;
  }
};

#endif // SWITCH_MONITOR_H
