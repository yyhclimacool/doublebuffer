cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(rpc_benchmark LANGUAGES C CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CXXFLAGS
  -pipe
  -Wall
  -W
  -Wextra
  -fPIC
  -fstrict-aliasing
  -Wno-invalid-offsetof
  -Wno-unused-parameter
)

string(REPLACE ";" " " CXXFLAGS "${CXXFLAGS}")
string(APPEND CMAKE_CXX_FLAGS " ")
string(APPEND CMAKE_CXX_FLAGS ${CXXFLAGS})

message(STATUS "CMAKE_C_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE})
message(STATUS "CMAKE_C_FLAGS_RELWITHDEBINFO: " ${CMAKE_C_FLAGS_RELWITHDEBINFO})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_C_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE})
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: " ${CMAKE_C_FLAGS_RELWITHDEBINFO})

add_executable(double_buffer_unittest double_buffer_unittest.cc double_buffer.cc)
target_link_libraries(double_buffer_unittest glog gflags pthread)
install(TARGETS double_buffer_unittest
  RUNTIME DESTINATION bin)
