cmake_minimum_required(VERSION 4.1)
project(Wheel2Controller C)

set(CMAKE_C_STANDARD 11)

add_library(Wheel2Controller STATIC library.c)
