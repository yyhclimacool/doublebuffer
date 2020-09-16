/*
 * @file double_buffer.h
 * @author youyunhong@corp.netease.com
 * @date 2020-09-15
 * @brief double buffer
 */

#ifndef COMMON_BASE_DOUBLEBUFFER_H
#define COMMON_BASE_DOUBLEBUFFER_H

#include <memory>
#include <string>
#include <thread>
#include <glog/logging.h>

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

class SwitchMonitor {
public:
  bool init() {
    return true;
  }
  bool shouldSwitch() {
    return true;
  }

  void doneSwitch() {
    LOG(INFO) << "done switch.";
  }
};

using SwitchMonitorSPtr = std::shared_ptr<SwitchMonitor>;

class SwitchMonitorFactory {
public:
  SwitchMonitorSPtr Create(const ReloadConfig &cfg) {
    return std::make_shared<SwitchMonitor>();
  }
};

template<typename T>
class DoubleBuffer {
public:
  DoubleBuffer(const std::string &n = "DefaultDoubleBuffer")
    : name_(n),
      cur_index_(0),
      stop_monitor_(false) { }

  DoubleBuffer(const std::string &n, const ReloadConfig &conf)
    : name_(n),
      cur_index_(0),
      stop_monitor_(false),
      config_(conf) { }

  ~DoubleBuffer() {
    stop_monitor_ = true;
    if (monitor_thread_.joinable()) {
      monitor_thread_.join();
    }

    // double_buffer_ auto destruct.
    LOG(INFO) << "double_buffer name=" << name_ << " destruct done.";
  }

  DoubleBuffer(const DoubleBuffer &) = delete;
  DoubleBuffer &operator=(const DoubleBuffer &) = delete;

  template<typename... Args>
  bool init(Args &&... args) {
    SwitchMonitorFactory factory;
    switch_monitor_ = factory.Create(config_);
    if (!switch_monitor_) {
      LOG(WARNING) << "Create SwitchMonitor for double_buffer=" << name_ << " failed. using config=" << config_;
      return false;
    }

    if (!switch_monitor_->init()) {
      LOG(WARNING) << "Init SwitchMonitor for double_buffer=" << name_ << " failed.";
      return false;
    }

    if (!reloadBuffer(std::forward<Args>(args) ...)) {
      LOG(WARNING) << "Load data for double_buffer=" << name_ << " failed.";
      return false;
    }

    if (config_.mode_ == ReloadConfig::ACTIVE_MODE) {
      monitor_thread_ = std::thread([&]() {
        while (!this->stop_monitor_) {
          sleep(this->config_.monitor_interval_);

          if (this->switch_monitor_->shouldSwitch()) {
            this->reloadBuffer(std::forward<Args>(args) ...);
          }
        }
      });
    }

    return true;
  }

  template<typename... Args>
  bool reloadBuffer(Args &&... args) {
    int alternative_index = 1 - cur_index_;
    auto t_sptr = std::make_shared<T>(std::forward<Args>(args) ...);
    if (!t_sptr) {
      LOG(ERROR) << "Memory allocation for buffer failed.";
      return false;
    }
    if (!t_sptr->init()) {
      LOG(ERROR) << "init failed and clean up the mess.";
      return false;
    }

    double_buffer_[alternative_index] = t_sptr;

    // from now on, will get new buffer
    cur_index_ = alternative_index;
    LOG(INFO) << "Switch buffer for double buffer name=" << name_ << " done.";

    switch_monitor_->doneSwitch();

    if (config_.old_buffer_lives_period_ > 0) {
      sleep(config_.old_buffer_lives_period_);
    }

    double_buffer_[1 - cur_index_] = nullptr;
    LOG(INFO) << "Reload alternative buffer and clear old buffer for double buffer name=" << name_ << " successed.";
    return true;
  }

  std::shared_ptr<T> getBuffer() {
    return double_buffer_[cur_index_];
  }

  const std::shared_ptr<T> getBuffer() const {
    return double_buffer_[cur_index_];
  }

  static const uint32_t BUF_SIZE = 2;
private:
  std::string name_;
  volatile int cur_index_;
  std::thread monitor_thread_;
  volatile bool stop_monitor_; /* stop the monitor thread */
  ReloadConfig config_;

  SwitchMonitorSPtr switch_monitor_;

  std::shared_ptr<T> double_buffer_[BUF_SIZE];
};

#endif // COMMON_BASE_DOUBLEBUFFER_H
