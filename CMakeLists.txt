cmake_minimum_required(VERSION 4.1)
project(Wheel2Firmware.elf CXX)

set(CMAKE_C_STANDARD 11)

add_executable(Wheel2Firmware startup.cpp src/main.cpp)
target_link_options(Wheel2Firmware PRIVATE -T${CMAKE_SOURCE_DIR}/linker.ld -nostartfiles --specs=nosys.specs)