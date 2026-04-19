cmake_minimum_required(VERSION 3.28)

project(CMSIS_CORE_LPC11xx LANGUAGES C)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CROSSCOMPILING AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Cross-compiling but CMAKE_TOOLCHAIN_FILE not set. Configure with -DCMAKE_TOOLCHAIN_FILE=<path to>/arm-none-eabi_mcuxpresso.cmake")
endif()
if(CMAKE_CROSSCOMPILING AND NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR "Toolchain file does not exist: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Determine architecture automatically if not specified
if(NOT DEFINED CMSIS_ARCHITECTURE)
    if(CMAKE_CROSSCOMPILING)
        set(CMSIS_ARCHITECTURE "arm")
    else()
        # Default to native architecture
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(CMSIS_ARCHITECTURE "x64")
        else()
            set(CMSIS_ARCHITECTURE "x86")
        endif()
    endif()
endif()

# Create cache variable with determined/provided value
set(CMSIS_ARCHITECTURE "${CMSIS_ARCHITECTURE}" CACHE STRING "Architecture to build for (arm, x86, x64).")
set_property(CACHE CMSIS_ARCHITECTURE PROPERTY STRINGS arm x86 x64)

# Validate architecture
if(CMAKE_CROSSCOMPILING AND NOT CMSIS_ARCHITECTURE STREQUAL "arm")
    message(FATAL_ERROR "Cross-compiling detected but CMSIS_ARCHITECTURE is '${CMSIS_ARCHITECTURE}' (expected 'arm').")
endif()

message(NOTICE "Configuring ${PROJECT_NAME} for ${CMSIS_ARCHITECTURE} architecture.")

# CMSIS LPC11xx source code
include(LPC11xx_src.cmake)

# Create target based on selected architecture
set(TARGET_NAME ${PROJECT_NAME}_${CMSIS_ARCHITECTURE})
add_library(${TARGET_NAME} STATIC ${CMSIS_CORE_LPC11XX_SRC})

# Public include directories
target_include_directories(${TARGET_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# Common compile definitions for all architectures
target_compile_definitions(${TARGET_NAME} PUBLIC
    __LPC11XX__
    #$<$<CONFIG:Debug>:DEBUG>
)

# Common compile options for all architectures
target_compile_options(${TARGET_NAME} PRIVATE
    -Wall -Wlogical-op -Wextra -pedantic
    -fmessage-length=0
    -g3
    #$<$<CONFIG:Debug>:-O0>
    #$<$<CONFIG:Release>:-Os>
)

# Architecture-specific configuration
if(CMSIS_ARCHITECTURE STREQUAL "arm")
#    target_compile_definitions(${TARGET_NAME} PUBLIC
#        __USE_CMSIS=CMSIS_CORE_LPC11xx
#        __NEWLIB__
#        CORE_M0
#    )
    # ARM-specific compile flags are handled by the toolchain file
    
elseif(CMSIS_ARCHITECTURE STREQUAL "x86")
    target_compile_definitions(${TARGET_NAME} PUBLIC IAP_EMULATION)
    
    # Check if compiler supports -m32 flag (GCC/Clang)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -m32)
        target_link_options(${TARGET_NAME} PRIVATE -m32)
    elseif(MSVC)
        message(WARNING "MSVC x86 builds should use the appropriate x86 toolchain/generator instead of -m32 flag.")
    endif()
    
elseif(CMSIS_ARCHITECTURE STREQUAL "x64")
    target_compile_definitions(${TARGET_NAME} PUBLIC IAP_EMULATION)
    
    # Check if compiler supports -m64 flag (GCC/Clang)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET_NAME} PRIVATE -m64)
        target_link_options(${TARGET_NAME} PRIVATE -m64)
    elseif(MSVC)
        message(WARNING "MSVC x64 builds should use the appropriate x64 toolchain/generator instead of -m64 flag.")
    endif()
else()
    message(FATAL_ERROR "Unknown architecture '${CMSIS_ARCHITECTURE}'. Valid options: arm, x86, x64")
endif()

message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
# message(STATUS "CMAKE_C_FLAGS_MINSIZEREL = ${CMAKE_C_FLAGS_MINSIZEREL}")
# message(STATUS "CMAKE_C_FLAGS_RELWITHDEBINFO = ${CMAKE_C_FLAGS_RELWITHDEBINFO}")

message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
#message(STATUS "CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
#message(STATUS "CMAKE_EXE_LINKER_FLAGS_DEBUG = ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
#message(STATUS "CMAKE_EXE_LINKER_FLAGS_RELEASE = ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
#message(STATUS "CMAKE_ASM_FLAGS = ${CMAKE_ASM_FLAGS}")
#message(STATUS "CMAKE_ASM_FLAGS_DEBUG = ${CMAKE_ASM_FLAGS_DEBUG}")
#message(STATUS "CMAKE_ASM_FLAGS_RELEASE = ${CMAKE_ASM_FLAGS_RELEASE}")
