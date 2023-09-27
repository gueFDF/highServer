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

# set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_BUILD_TYPE "Debug")

# 设置可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置项目文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/usr/local/lib)


# 设置编译头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include/asynLog)
include_directories(${PROJECT_SOURCE_DIR}/include/utils)
include_directories(${PROJECT_SOURCE_DIR}/include/net)
include_directories(/usr/local/include)




add_subdirectory(./src/utils)
add_subdirectory(./src/asynlog)
add_subdirectory(./src/net)
add_subdirectory(./example)
add_subdirectory(./test)



 