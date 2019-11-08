# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("fileTransmissionLinux")

# 包含子项目。
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -march=native -O3 -lpthread -lprotobuf" )
LINK_LIBRARIES(-lpthread -lm)
LINK_LIBRARIES(-lcrypto -lm)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)   
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/liba)   

add_subdirectory ("common")
add_subdirectory ("client")
add_subdirectory ("server")

