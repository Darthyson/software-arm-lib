cmake_minimum_required(VERSION 3.31)

project(sblib LANGUAGES CXX VERSION 2.10)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SBLPC_DIR ${CMAKE_SOURCE_DIR}/../../CMSIS_CORE_LPC11xx CACHE PATH "Where the CMSIS_CORE_LPC11xx lib is")
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


set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "How to build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
set(LOGGING OFF CACHE BOOL "Whether to through the serial port")


# todo create options for all defines/symbols
option(DEBUG_BUS "Enable dumping of state machine interrupt data e.g timer values" OFF)
option(DEBUG_BUS_BITLEVEL "Extension used with DEBUG_BUS to dump interrupt of each bit -
                           Use with care due to easy overflow of the trace buffer" OFF)

set(COMMON_DEFINES
    __USE_CMSIS=CMSIS_CORE_LPC11xx
    __LPC11XX__
    __NEWLIB__
    CORE_M0
)

set(DEBUG_DEFINES_CORE
    ${COMMON_DEFINES}
    INCLUDE_SERIAL
    SERIAL_SPEED=1500000
    DEBUG_BUS
    DEBUG_BUS_BITLEVEL
    DUMP_TELEGRAMS
    PIO_FOR_TEL_END_IND=PIO1_4
    DUMP_COM_OBJ
    DUMP_MEM_OPS
    DUMP_PROPERTIES
    DUMP_TL4
#    SERIAL_RX_PIN=PIO2_7
#    SERIAL_TX_PIN=PIO2_8
#    ROUTER
#    BUSMONITOR
)

set(DEBUG_DEFINES_ARM
    ${DEBUG_DEFINES_CORE}
)

set(DEBUG_DEFINES_X86_X64
    ${DEBUG_DEFINES_CORE}
    IAP_EMULATION
)

set(RELEASE_DEFINES_CORE
    ${COMMON_DEFINES}
)

set(RELEASE_DEFINES_ARM
    ${RELEASE_DEFINES_CORE}
)

set(RELEASE_DEFINES_X86_X64
    ${RELEASE_DEFINES_CORE}
    IAP_EMULATION
)

# Most of the debug compile flags are already set by the toolchain file (arm-none-eabi_mcuxpresso.cmake.cmake)
set(COMMON_FLAGS_ARM
    # todo warnings to enable in future
    # -pedantic
    # -Wconversion
)

set(DEBUG_FLAGS_ARM
    ${COMMON_FLAGS_ARM}
)

set(RELEASE_FLAGS_ARM
    ${COMMON_FLAGS_ARM}
    -Os # optimize for size
)

set (COMMON_X86_X64_FLAGS
    -Wall -Wlogical-op -Woverloaded-virtual
    -O0
    -g3
    -fmessage-length=0
)

set(DEBUG_X86_FLAGS
    ${COMMON_X86_X64_FLAGS}
    #-m32
)

set(RELEASE_X86_FLAGS
    ${COMMON_X86_X64_FLAGS}
    #-m32
)

set(DEBUG_X64_FLAGS
    ${COMMON_X86_X64_FLAGS}
    #-m64
)

set(RELEASE_X64_FLAGS
    ${COMMON_X86_X64_FLAGS}
    #-m64
)

# include sblib source code
include_directories(inc) # sblib/inc directory
include(sblib.cmake)     # list of all sblib source code files (*.h, *.cpp, ...)

# include potential logging source code
# todo LOGGING is broken
if (LOGGING AND CMAKE_CROSSCOMPILING)
    if(NOT DEFINED SBLOG_DIR)
        message(FATAL_ERROR "LOGGING enabled but SBLOG_DIR is not defined. Set -DSBLOG_DIR=<path> or unset LOGGING.")
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
        $<$<CONFIG:Debug>: ${DEBUG_DEFINES_ARM}>
        $<$<CONFIG:Release>: ${RELEASE_DEFINES_ARM}>
    )
    target_compile_options(${TARGET_ARM} PRIVATE
        $<$<CONFIG:Debug>: ${DEBUG_FLAGS_ARM}>
        $<$<CONFIG:Release>: ${RELEASE_FLAGS_ARM}>
    )
# Check if building for 32bit architecture    
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(NOTICE "Compiling ${PROJECT_NAME} for 32bit architecture.")
    add_library(${TARGET_X86} STATIC ${SBLIB_SRC}) # 32bit sblib
    target_include_directories(${TARGET_X86} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../test/sblib/cpu-emu)
    target_compile_definitions(${TARGET_X86} PRIVATE
        $<$<CONFIG:Debug>: ${DEBUG_DEFINES_X86_X64}>
        $<$<CONFIG:Release>: ${RELEASE_DEFINES_X86_X64}>
    )
    target_compile_options(${TARGET_X86} PRIVATE
        $<$<CONFIG:Debug>: ${DEBUG_X86_FLAGS}>
        $<$<CONFIG:Release>: ${RELEASE_X86_FLAGS}>
    )
# Check if building for 64bit architecture    
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    #todo test x64 target
    message(AUTHOR_WARNING "64bit target ${TARGET_X64} is not tested yet")
    message(NOTICE "Compiling ${PROJECT_NAME} for 64bit architecture.")
    add_library(${TARGET_X64} STATIC ${SBLIB_SRC}) # 64bit sblib
    target_include_directories(${TARGET_X64} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../test/sblib/cpu-emu)
    target_compile_definitions(${TARGET_X64} PRIVATE
        $<$<CONFIG:Debug>: ${DEBUG_DEFINES_X86_X64}>
        $<$<CONFIG:Release>: ${RELEASE_DEFINES_X86_X64}>
    )
    target_compile_options(${TARGET_X64} PRIVATE
        $<$<CONFIG:Debug>: ${DEBUG_X64_FLAGS}>
        $<$<CONFIG:Release>: ${RELEASE_X64_FLAGS}>
    )
else()
    message(FATAL_ERROR "Unknown architecture sizeof(void*)=${CMAKE_SIZEOF_VOID_P}. (project ${PROJECT_NAME})")
endif()
