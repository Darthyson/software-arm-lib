cmake_minimum_required(VERSION 3.28)

project(sblib LANGUAGES CXX VERSION 2.10)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SBLPC_DIR ${CMAKE_SOURCE_DIR}/../../CMSIS_CORE_LPC11xx CACHE PATH "Where the CMSIS_CORE_LPC11xx lib is")
set(SBLIB_CPU_EMULATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../test/sblib/cpu-emu)
# Toolchain file is set in CMakePresets.json or on command line
if(CMAKE_CROSSCOMPILING AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Cross-compiling but CMAKE_TOOLCHAIN_FILE not set. Set in CMakePresets.json or configure with -DCMAKE_TOOLCHAIN_FILE=<path to>/arm-none-eabi_mcuxpresso.cmake")
endif()
if(CMAKE_CROSSCOMPILING AND DEFINED CMAKE_TOOLCHAIN_FILE AND NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR "Toolchain file does not exist: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(TARGET_ARM ${PROJECT_NAME})
set(TARGET_X86 ${PROJECT_NAME}_x86)
set(TARGET_X64 ${PROJECT_NAME}_x64)

set(SBLIB_LOGGING OFF CACHE BOOL "Whether to log through the serial port")

set(SBLIB_ARCHITECTURE "arm" CACHE STRING "Architecture to build for (arm, x86, x64).")
set_property(CACHE SBLIB_ARCHITECTURE PROPERTY STRINGS arm x86 x64)
option(SBLIB_DEBUG_BUS "Enable dumping of state machine interrupt data e.g timer values." OFF)
option(SBLIB_DEBUG_BUS_BITLEVEL "Extension used with DEBUG_BUS to dump interrupt of each bit -
                                 Use with care due to easy overflow of the trace buffer." OFF)
option(SBLIB_INCLUDE_SERIAL "sblib should open serial port." OFF)
set(SBLIB_SERIAL_SPEED "OFF" CACHE STRING "Serial port speed for debugging (OFF, 57600, 115200, 230400, 576000, 1500000).")
set_property(CACHE SBLIB_SERIAL_SPEED PROPERTY STRINGS OFF 57600 115200 230400 576000 1500000)
option(SBLIB_DUMP_TELEGRAMS "Dump rx and tx telegrams, incl. received ack and timing info over serial interface." OFF)
set(SBLIB_PIO_FOR_TEL_END_IND "OFF" CACHE STRING "Toggles defined PIOx_y pin at the end of a received telegram.")
option(SBLIB_DUMP_COM_OBJ "Dump object handling information on app-server level over serial interface." OFF)
option(SBLIB_DUMP_MEM_OPS "Dump APCI_MEMORY_WRITE_PDU and APCI_MEMORY_READ_PDU over serial interface." OFF)
option(SBLIB_DUMP_PROPERTIES "Dump interface object and property handling over serial interface." OFF)
option(SBLIB_DUMP_TL4 "Dump transport layer 4 protocol handling over serial interface." OFF)
set(SBLIB_SERIAL_RX_PIN "OFF" CACHE STRING "Serial receive pin to use (PIOx_y).")
set(SBLIB_SERIAL_TX_PIN "OFF" CACHE STRING "Serial transmit pin to use (PIOx_y).")
option(SBLIB_ROUTER "Enable to build a ROUTER." OFF)
option(SBLIB_BUSMONITOR "Enables bus monitoring: dump of all bus traffic incl. timing info (no sending, no processing of rx-telegrams)." OFF)


# definitions for all targets and builds
add_compile_definitions(
    __USE_CMSIS=CMSIS_CORE_LPC11xx
    __LPC11XX__
    __NEWLIB__
    CORE_M0
)

# debug definitions for all targets and debug builds
set(DEBUG_DEFINES
    DEBUG
)

# Conditionally add debug options based on CMake options
if(SBLIB_DEBUG_BUS)
    list(APPEND DEBUG_DEFINES DEBUG_BUS)
endif()
if(SBLIB_DEBUG_BUS_BITLEVEL)
    list(APPEND DEBUG_DEFINES DEBUG_BUS_BITLEVEL)
endif()
if(SBLIB_INCLUDE_SERIAL)
    list(APPEND DEBUG_DEFINES INCLUDE_SERIAL)
endif()
if(NOT SBLIB_SERIAL_SPEED STREQUAL "OFF")
    list(APPEND DEBUG_DEFINES SERIAL_SPEED=${SBLIB_SERIAL_SPEED})
endif()
if(SBLIB_DUMP_TELEGRAMS)
    list(APPEND DEBUG_DEFINES DUMP_TELEGRAMS)
endif()
if(NOT SBLIB_PIO_FOR_TEL_END_IND STREQUAL "OFF")
    list(APPEND DEBUG_DEFINES PIO_FOR_TEL_END_IND=${SBLIB_PIO_FOR_TEL_END_IND})
endif()
if(SBLIB_DUMP_COM_OBJ)
    list(APPEND DEBUG_DEFINES DUMP_COM_OBJ)
endif()
if(SBLIB_DUMP_MEM_OPS)
    list(APPEND DEBUG_DEFINES DUMP_MEM_OPS)
endif()
if(SBLIB_DUMP_PROPERTIES)
    list(APPEND DEBUG_DEFINES DUMP_PROPERTIES)
endif()
if(SBLIB_DUMP_TL4)
    list(APPEND DEBUG_DEFINES DUMP_TL4)
endif()
if(NOT SBLIB_SERIAL_RX_PIN STREQUAL "OFF")
    list(APPEND DEBUG_DEFINES SERIAL_RX_PIN=${SBLIB_SERIAL_RX_PIN})
endif()
if(NOT SBLIB_SERIAL_TX_PIN STREQUAL "OFF")
    list(APPEND DEBUG_DEFINES SERIAL_TX_PIN=${SBLIB_SERIAL_TX_PIN})
endif()
if(SBLIB_ROUTER)
    list(APPEND DEBUG_DEFINES ROUTER)
endif()
if(SBLIB_BUSMONITOR)
    list(APPEND DEBUG_DEFINES BUSMONITOR)
endif()

# common compile options for all targets and builds
add_compile_options(
    -Wall 
    -Wlogical-op 
    -Woverloaded-virtual
    -Wextra
    # todo warnings to enable in future
    # -pedantic
    # -Wconversion
    -fmessage-length=0
    -g3
)

# include sblib source code
include_directories(inc) # sblib/inc directory
include(sblib.cmake)     # list of all sblib source code files (*.h, *.cpp, ...)

# include potential logging source code
# todo SBLIB_LOGGING is broken
if (SBLIB_LOGGING AND CMAKE_CROSSCOMPILING)
    if(NOT DEFINED SBLOG_DIR)
        message(FATAL_ERROR "SBLIB_LOGGING enabled but SBLOG_DIR is not defined. Set -DSBLOG_DIR=<path> or unset SBLIB_LOGGING.")
    endif()
    add_subdirectory(${SBLOG_DIR} sblog)
    target_include_directories(${TARGET_ARM} PUBLIC ${SBLOG_DIR})
    file(GLOB LOGSRC ${SBLOG_DIR}/log/*.cpp ${SBLOG_DIR}/log/*.h)
    list(APPEND SBLIB_SRC ${LOGSRC})
    add_definitions(-DLOGGING)
endif()

# Check if cross-compiling for ARM target
if(CMAKE_CROSSCOMPILING)
    message(NOTICE "Compiling ${PROJECT_NAME} for ARM architecture.")
    add_library(${TARGET_ARM} STATIC ${SBLIB_SRC})
    target_link_libraries(${TARGET_ARM} PUBLIC CMSIS_CORE_LPC11xx_arm) # add CMSIS_CORE_LPC11xx_arm library
    target_include_directories(${TARGET_ARM} PUBLIC ${SBLPC_DIR}/inc) # add CMSIS_CORE_LPC11xx_arm include directory
    target_compile_definitions(${TARGET_ARM} PRIVATE
        $<$<CONFIG:Debug>:${DEBUG_DEFINES}>
    )
    target_compile_options(${TARGET_ARM} PRIVATE
        $<$<CONFIG:Debug>: -O0>
        $<$<CONFIG:Release>: -Os>
    )
# Check if building for 32bit architecture
elseif(SBLIB_ARCHITECTURE STREQUAL "x86")
    message(NOTICE "Compiling ${PROJECT_NAME} for 32bit architecture.")
    add_library(${TARGET_X86} STATIC ${SBLIB_SRC}) # 32bit sblib
    target_include_directories(${TARGET_X86} PUBLIC ${SBLIB_CPU_EMULATION_DIR})
    target_compile_definitions(${TARGET_X86} PRIVATE
        IAP_EMULATION
        $<$<CONFIG:Debug>:${DEBUG_DEFINES}>
    )
    target_compile_options(${TARGET_X86} PRIVATE
        -m32
        $<$<CONFIG:Debug>: -O0>
        $<$<CONFIG:Release>: -Os>
    )
# Check if building for 64bit architecture
elseif(SBLIB_ARCHITECTURE STREQUAL "x64")
    message(NOTICE "Compiling ${PROJECT_NAME} for 64bit architecture.")
    add_library(${TARGET_X64} STATIC ${SBLIB_SRC}) # 64bit sblib
    target_include_directories(${TARGET_X64} PUBLIC ${SBLIB_CPU_EMULATION_DIR})
    target_compile_definitions(${TARGET_X64} PRIVATE
        IAP_EMULATION
        $<$<CONFIG:Debug>:${DEBUG_DEFINES}>
    )
    target_compile_options(${TARGET_X64} PRIVATE
        -m64
        $<$<CONFIG:Debug>: -O0>
        $<$<CONFIG:Release>: -Os>
    )    
else()
    message(FATAL_ERROR "Unknown architecture ${SBLIB_ARCHITECTURE}. (project ${PROJECT_NAME})")
endif()
