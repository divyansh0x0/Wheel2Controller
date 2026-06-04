cmake_minimum_required(VERSION 3.10)

# 1. ENABLE CMAKE CROSS COMPILATION
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Prevent CMake from trying to link a test binary during configuration
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 2. DEFINE THE ARM TOOLCHAIN
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

project(Wheel2Firmware CXX C ASM)

# 3. SETTING C++ STANDARD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 4. COMPILER FLAGS (Separated by Build Type)
# -----------------------------------------------------------------------------

# Common Flags (Always active: Architecture, Strict Warnings, C++ stripping)
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(WARNING_FLAGS "-Wall -Wextra -Wdouble-promotion -Wfloat-conversion -Werror=double-promotion")
set(CXX_ONLY_FLAGS "-fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit")

# Apply Common Flags
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} ${WARNING_FLAGS} ${CXX_ONLY_FLAGS}")
set(CMAKE_C_FLAGS "${CPU_FLAGS} ${WARNING_FLAGS}")

# --- DEBUG BUILD FLAGS ---
# -Og: Optimizes specifically for debugging (keeps variables readable in GDB).
# -g3: Maximum debug symbols, including preprocessor macros.
# -DDEBUG: Sets a global #define DEBUG macro you can use in your code.
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -DDEBUG")

# --- RELEASE BUILD FLAGS ---
# -O3: Maximum speed optimization (aggressively collapses inline functions).
# -DNDEBUG: Standard C++ macro that automatically strips out any assert() checks.
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Default to Debug if the user doesn't specify a type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type selected, defaulting to Debug")
endif()

# -----------------------------------------------------------------------------
# 5. DEFINE SOURCES
# -----------------------------------------------------------------------------
include_directories(include)

add_executable(Wheel2Firmware
        startup.cpp
        src/main.cpp
        include/drivers/USART.h
        src/drivers/I2C.cpp
        include/drivers/I2C.h
)

# -----------------------------------------------------------------------------
# 6. LINKER FLAGS (The Linker Diet)
# -----------------------------------------------------------------------------
target_link_options(Wheel2Firmware PRIVATE
        -T${CMAKE_SOURCE_DIR}/linker.ld
        -nostartfiles
        --specs=nano.specs
        --specs=nosys.specs
        -mcpu=cortex-m3
        -mthumb
        -Wl,--gc-sections
)

# 7. GENERATE THE RAW BINARY
add_custom_command(TARGET Wheel2Firmware POST_BUILD
        COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:Wheel2Firmware> ${CMAKE_CURRENT_BINARY_DIR}/Wheel2Firmware.bin
)