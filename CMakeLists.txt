cmake_minimum_required(VERSION 3.6)
project(WirelessSignalAdjuster)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp WirelessSignalAdjuster.cpp WirelessSignalAdjuster.h)
add_executable(WirelessSignalAdjuster ${SOURCE_FILES})