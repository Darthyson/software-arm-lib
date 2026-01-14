cmake_minimum_required(VERSION 3.28)

project(sblib-test CXX)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(SBLIB_ARCHITECTURE "x86" CACHE STRING "Architecture to build for (x86, x64).")

set(CATCH_PATH "${CMAKE_SOURCE_DIR}/../../Catch" CACHE PATH "Path to the Catch project")
set(SBLIB_PATH "${CMAKE_SOURCE_DIR}/../../sblib" CACHE PATH "Path to the sblib")

set(TARGET_X86 ${PROJECT_NAME}_x86)
set(TARGET_X64 ${PROJECT_NAME}_x64)

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

add_compile_definitions(
    IAP_EMULATION
    __LPC11XX__
    $<$<CONFIG:Debug>:DEBUG>
    #$<$<CONFIG:Debug>:SERIAL_SPEED=1500000>
    #$<$<CONFIG:Debug>:DEBUG_BUS>
    #$<$<CONFIG:Debug>:DEBUG_BUS_BITLEVEL>
    #$<$<CONFIG:Debug>:DUMP_TELEGRAMS>
    #$<$<CONFIG:Debug>:PIO_FOR_TEL_END_IND=PIO1_4>
    #$<$<CONFIG:Debug>:DUMP_COM_OBJ>
    #$<$<CONFIG:Debug>:DUMP_MEM_OPS>
    #$<$<CONFIG:Debug>:DUMP_PROPERTIES>
    #$<$<CONFIG:Debug>:DUMP_TL4>
)

add_compile_options(
    -Wall -Wlogical-op -Woverloaded-virtual -Wextra -pedantic
    # todo Warnings to enable in future
    # -Wconversion
    -g3
    -fmessage-length=0
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Debug>:-O0>
)

# Check if building for 32bit architecture
if(SBLIB_ARCHITECTURE STREQUAL "x86")
    add_library(${TARGET_X86} STATIC ${SBLIB_TEST_FULL_SRC})
    target_link_libraries(${TARGET_X86} PUBLIC sblib_x86)
    #add_subdirectory(${CATCH_PATH} catch)
    add_subdirectory(${SBLIB_PATH} sblib_x86)
    target_include_directories(${TARGET_X86} PUBLIC inc)
    target_include_directories(${TARGET_X86} PUBLIC cpu-emu)
    target_include_directories(${TARGET_X86} PUBLIC ${CATCH_PATH}/inc)
    target_include_directories(${TARGET_X86} PUBLIC ${SBLIB_PATH}/inc)
    target_compile_options(${TARGET_X86} PRIVATE
        -m32
    )
# Check if building for 64bit architecture
elseif(SBLIB_ARCHITECTURE STREQUAL "x64")
    add_library(${TARGET_X64} STATIC ${SBLIB_TEST_FULL_SRC})
    target_link_libraries(${TARGET_X64} PUBLIC sblib_x64)
    #add_subdirectory(${CATCH_PATH} catch)
    add_subdirectory(${SBLIB_PATH} sblib_x64)
    target_include_directories(${TARGET_X64} PUBLIC ${CATCH_PATH}/inc)
    target_include_directories(${TARGET_X64} PUBLIC ${SBLIB_PATH}/inc)
    target_include_directories(${TARGET_X64} PUBLIC cpu-emu)
    target_include_directories(${TARGET_X64} PUBLIC inc)
    target_compile_options(${TARGET_X64} PRIVATE
        -m64
    )
else()
    message(FATAL_ERROR "Unknown architecture ${SBLIB_ARCHITECTURE}. (project ${PROJECT_NAME})")
endif()
