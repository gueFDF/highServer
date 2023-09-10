cmake_minimum_required(VERSION 3.16.3)
project(tinyrpc C CXX)

set(CMAKE_CXX_COMPILE  "/usr/bin/clang++")
set(CXX_FLAGS
 -g
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -std=c++11
 -rdynamic
 )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(LI)

# 设置编译头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/Utils)
include_directories(${PROJECT_SOURCE_DIR}/AsynLog)
include_directories(/usr/local/include)


add_executable(main ./main.cc ./AsynLog/LogStream.cc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 