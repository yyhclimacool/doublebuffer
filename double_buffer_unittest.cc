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
    return true;
  }
  int getVersion() {
    return version;
  }
private:
  std::string filename_;
  static volatile int version;
};

volatile int FileContent::version = 0;

int main(int argc, char **argv) {
  FLAGS_logbufsecs = 0;
  FLAGS_logbuflevel = -1;
  FLAGS_minloglevel = 0;
  //FLAGS_log_dir = "./logs";
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);

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
    LOG(INFO) << "file version is " << file_content_sptr->getVersion() << " now sleep " << sleep_time / 1000 << " ms";
    usleep(sleep_time);
  }
  LOG(INFO) << "LOG INFO ...";
}
