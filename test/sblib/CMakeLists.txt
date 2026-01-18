cmake_minimum_required(VERSION 3.28)

project(sblib-test LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(SBLIB_ARCHITECTURE "x86" CACHE STRING "Architecture to build for (x86, x64).")

set(CATCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../Catch" CACHE PATH "Path to the Catch project")
set(SBLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../sblib" CACHE PATH "Path to the sblib")

if (NOT EXISTS "${CATCH_PATH}")
    message(FATAL_ERROR "Catch project not found. Check setting of CATCH_PATH (${CATCH_PATH})")
endif()
if (NOT EXISTS "${SBLIB_PATH}")
    message(FATAL_ERROR "sblib project not found. Check setting of SBLIB_PATH (${SBLIB_PATH})")
endif()

include(sblib-test.cmake)
include(${CATCH_PATH}/catch.cmake)
set(SBLIB_TEST_FULL_SRC
        ${SBLIB_CATCH_SRC}
#        ${SBLIB_SRC}
        ${SBLIB_TEST_SRC}
)


# Create sblib-test targets
set(TARGET_X64 ${PROJECT_NAME}_x64)
set(TARGET_X86 ${PROJECT_NAME}_x86)
add_library(${TARGET_X86} STATIC ${SBLIB_TEST_FULL_SRC})
add_library(${TARGET_X64} STATIC ${SBLIB_TEST_FULL_SRC})

# Include directories
foreach(target ${TARGET_X86} ${TARGET_X64})
    target_include_directories(${target} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/cpu-emu
        ${CATCH_PATH}/inc
        ${SBLIB_PATH}/inc
    )
endforeach()

add_compile_definitions(
    IAP_EMULATION
    __LPC11XX__
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
foreach(target ${TARGET_X86} ${TARGET_X64})
    target_compile_options(${target} PRIVATE
        -Wall -Wlogical-op -Woverloaded-virtual -Wextra -pedantic
        # todo warnings to enable in future
        # -Wconversion
        -fmessage-length=0
        -g3
        $<$<CONFIG:Debug>: -O0>
        $<$<CONFIG:Release>: -Os>
    )
endforeach()

add_subdirectory(${SBLIB_PATH} sblib)
target_link_libraries(${TARGET_X86} PUBLIC sblib_x86)
target_link_libraries(${TARGET_X64} PUBLIC sblib_x64)

# Check if building for 32bit architecture
if(SBLIB_ARCHITECTURE STREQUAL "x86")
    message(NOTICE "Configuring ${PROJECT_NAME} for x86 architecture.")
    target_compile_options(${TARGET_X86} PRIVATE -m32)
# Check if building for 64bit architecture
elseif(SBLIB_ARCHITECTURE STREQUAL "x64")
    message(NOTICE "Configuring ${PROJECT_NAME} for x64 architecture.")
    target_compile_options(${TARGET_X64} PRIVATE -m64)
else()
    message(FATAL_ERROR "Unknown architecture ${SBLIB_ARCHITECTURE}. (project ${PROJECT_NAME})")
endif()
