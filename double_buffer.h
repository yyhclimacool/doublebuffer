/*
 * @file double_buffer.h
 * @author yyhclimacool@gmail.com
 * @date 2020-09-15
 * @brief double buffer
 */

#ifndef DOUBLEBUFFER_H
#define DOUBLEBUFFER_H

#include <memory>
#include <string>
#include <thread>

#include <glog/logging.h>

#include "reload_config.h"
#include "switch_monitor.h"

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
    LOG(INFO) << "double_buffer_name=" << name_ << " destruct done.";
  }

  DoubleBuffer(const DoubleBuffer &) = delete;
  DoubleBuffer &operator=(const DoubleBuffer &) = delete;

  template<typename... Args>
  bool init(Args &&... args);

  template<typename... Args>
  bool reloadBuffer(Args &&... args);

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

template<typename T>
template<typename... Args>
bool DoubleBuffer<T>::init(Args &&... args) {
  SwitchMonitorFactory factory;
  switch_monitor_ = factory.Create(config_);
  if (!switch_monitor_) {
    LOG(WARNING) << "Create SwitchMonitor for double_buffer=" << name_ << " failed. using config=" << config_;
    return false;
  }

  if (!switch_monitor_->init(config_)) {
    LOG(WARNING) << "Init SwitchMonitor for double_buffer=" << name_ << " failed.";
    return false;
  }

  if (!reloadBuffer(std::forward<Args>(args) ...)) {
    LOG(WARNING) << "Initial load data for double_buffer=" << name_ << " failed.";
    return false;
  }

  if (config_.mode_ == ReloadConfig::ACTIVE_MODE) {
    monitor_thread_ = std::thread([&]() {
      while (!this->stop_monitor_) {
        sleep(this->config_.monitor_interval_);

        this->reloadBuffer(std::forward<Args>(args) ...);
      }
    });
  }

  return true;
}

template<typename T>
template<typename... Args>
bool DoubleBuffer<T>::reloadBuffer(Args &&... args) {
  if (!switch_monitor_) {
    LOG(ERROR) << "switch_monitor is nullptr, return false.";
    return false;
  }

  if (!switch_monitor_->shouldSwitch()) {
    DLOG(INFO) << "Switch monitor tells me not to switch.";
    return true;
  }

  int alternative_index = 1 - cur_index_;
  auto t_sptr = std::make_shared<T>(std::forward<Args>(args) ...);
  if (!t_sptr) {
    LOG(ERROR) << "Memory allocation for buffer failed.";
    return false;
  }
  if (!t_sptr->init()) {
    LOG(ERROR) << "Init failed and clean up the mess.";
    return false;
  }

  double_buffer_[alternative_index] = t_sptr;

  // from now on, will get new buffer
  cur_index_ = alternative_index;
  LOG(INFO) << "Switch buffer for double_buffer=" << name_ << " cur_index=" << cur_index_ << " successed.";

  switch_monitor_->doneSwitch();

  // Delay of destruction, the destruction maybe a time consuming operation.
  if (config_.old_buffer_lives_period_ > 0) {
    sleep(config_.old_buffer_lives_period_);
  }

#ifndef NDEBUG
  auto sptr = double_buffer_[1 - cur_index_];
#endif
  double_buffer_[1 - cur_index_] = nullptr;
#ifndef NDEBUG
  DLOG(INFO) << "Alternative SPTR use_count(except me)=" << sptr.use_count() - 1;
#endif
  LOG(INFO) << "Clear old buffer for double_buffer=" << name_ << " successed.";
  return true;
}

#endif // DOUBLEBUFFER_H
