/*
 * @file double_buffer_unittest.cc
 * @author yyhclimacool@gmail.com
 * @date 2020-09-15
 * @brief double buffer unittest
 */

#include "double_buffer.h"
#include <iostream>
#include <string>
#include <glog/logging.h>

class FileContent {
public:
  FileContent(const std::string &filename)
    : filename_(filename) {}
  
  bool init() {
    ++version;
    detail_ = filename_ + std::to_string(version);
    return true;
  }
  int getVersion() {
    return version;
  }

  std::string &getDetail() {
    return detail_;
  }
private:
  std::string filename_;
  std::string detail_;
  static volatile int version;
};

volatile int FileContent::version = 0;

int main(int argc, char **argv) {
  FLAGS_logbufsecs = 0;
  FLAGS_log_dir = "./logs";
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  ReloadConfig file_double_buffer_reload_config(ReloadConfig::ACTIVE_MODE, 
      ReloadConfig::MONITOR_FILE,
      60,
      5,
      "./test_for_reload");

  DoubleBuffer<FileContent> fileDoubleBuffer("fileDoubelBuffer", file_double_buffer_reload_config);

  if(!fileDoubleBuffer.init(file_double_buffer_reload_config.filename_)) {
    LOG(FATAL) << "init fileDoubleBuffer failed." << std::endl;
    return -1;
  }

  while (true) {
    auto file_content_sptr = fileDoubleBuffer.getBuffer();

    int sleep_time = (rand() % 100) * 10000;

    // start a background thread to hold the shared_ptr.
    if (sleep_time == 0) {
      LOG(INFO) << "Start a background thread to hold the shared_ptr of detail=" << file_content_sptr->getDetail();
      std::thread([=] () {
          sleep(70);
          LOG(INFO) << "In background thread, old file detail=" << file_content_sptr->getDetail() << " and now sleep 70s.";
          sleep(70);
          LOG(INFO) << "In background thread, old file detail=" << file_content_sptr->getDetail() << " and now return background thread.";
      }).detach();
    }
    LOG(INFO) << "file version=" << file_content_sptr->getVersion() << " detail=" << file_content_sptr->getDetail() << " now sleep " << sleep_time / 1000 << " ms";
    usleep(sleep_time);
  }
}
