# CMakeList.txt: serverdemo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

aux_source_directory(. DIR_LIB_SRCS)

add_library (common ${DIR_LIB_SRCS})