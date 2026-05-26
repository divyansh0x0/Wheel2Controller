cmake_minimum_required(VERSION 3.10)

# 1. FORCING CMAKE TO CROSS-COMPILE
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Prevent CMake from trying to link a test binary during configuration
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 2. DEFINING THE ARM TOOLCHAIN
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

project(Wheel2Firmware CXX C ASM)

# 3. SETTING C++ STANDARD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. CPU-SPECIFIC FLAGS — these MUST be passed to the compiler so it generates
#    Thumb-2 instructions. Without them the compiler emits ARM-mode code,
#    which immediately HardFaults on the Cortex-M3 (a Thumb-only core).
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_FLAGS} -O0 -g3 -Wall -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -O0 -g3 -Wall")

add_executable(Wheel2Firmware startup.cpp src/main.cpp)

# 5. LINKER FLAGS
target_link_options(Wheel2Firmware PRIVATE
        -T${CMAKE_SOURCE_DIR}/linker.ld
        -nostartfiles
        --specs=nosys.specs
        -mcpu=cortex-m3
        -mthumb
)
add_custom_command(TARGET Wheel2Firmware POST_BUILD
        COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:Wheel2Firmware> ${CMAKE_CURRENT_BINARY_DIR}/Wheel2Firmware.bin
)