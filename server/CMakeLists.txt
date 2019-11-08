# CMakeList.txt: serverdemo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


aux_source_directory(. DIR_SRCS)

add_executable (serverd "server.cpp")

# 将源代码添加到此项目的可执行文件。
target_link_libraries(serverd common)

# TODO: 如有需要，请添加测试并安装目标。
