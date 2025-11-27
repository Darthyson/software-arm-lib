cmake_minimum_required(VERSION 3.28)

project(lib-test-cases LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Enable verbose output from the build system")
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CATCH_PATH "${CMAKE_SOURCE_DIR}/../../Catch" CACHE PATH "Path to the Catch project")
set(SBLIB_PATH "${CMAKE_SOURCE_DIR}/../../sblib" CACHE PATH "Path to the sblib project")
set(SBLIB_TEST_PATH "${CMAKE_SOURCE_DIR}/../sblib" CACHE PATH "Path to the sblib-test project")

if (NOT EXISTS "${CATCH_PATH}")
    message(FATAL_ERROR "Catch project not found. Check setting of CATCH_PATH (${CATCH_PATH})")
endif()

if (NOT EXISTS "${SBLIB_PATH}")
    message(FATAL_ERROR "sblib project not found. Check setting of SBLIB_PATH (${SBLIB_PATH})")
endif()

if (NOT EXISTS "${SBLIB_TEST_PATH}")
    message(FATAL_ERROR "sblib-test project not found. Check setting of SBLIB_TEST_PATH (${SBLIB_TEST_PATH})")
endif()

# Enable testing
include(CTest)

include(${CATCH_PATH}/catch.cmake)                # add catch.hpp
include(${CMAKE_SOURCE_DIR}/lib-test-cases.cmake) # add all test cases

set(SBLIB_TEST_CASE_SRC
    ${SBLIB_CATCH_SRC}
    ${SBLIB_LIB_TEST_CASES_SRC}
)

set(TARGET_X86 ${CMAKE_PROJECT_NAME}_x86)
set(TARGET_X64 ${CMAKE_PROJECT_NAME}_x64)

set(INCLUDE_DIRECTORIES
    ${CATCH_PATH}/inc          # catch
    ${SBLIB_PATH}/inc          # sblib
    ${SBLIB_TEST_PATH}/inc     # sblib-test
    ${SBLIB_TEST_PATH}/cpu-emu # sblib-test cpu emulation
)

#todo create options for definitions/symbols
##add_definitions(-DSERIAL_RX_PIN=PIO2_7)
##add_definitions(-DSERIAL_TX_PIN=PIO2_8)
##add_definitions(-DINCLUDE_SERIAL)
##add_definitions(-DROUTER)
##add_definitions(-DBUSMONITOR)
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
    #  todo Warnings to enable in future
    # -Wextra
    # -pedantic
    # -Wconversion
    -Wall -Wlogical-op -Woverloaded-virtual
    -fmessage-length=0
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
)

add_link_options(
    #-fuse-ld=lld
    #$<$<CONFIG:Release>:-flto>
    #$<$<CONFIG:Release>:-ffat-lto-objects>
    $<$<CONFIG:Release>:-s>
    #$<$<CONFIG:Debug>: >
)

# Check if building for 32bit architecture
if(SBLIB_ARCHITECTURE STREQUAL "x86")
    add_executable(${TARGET_X86} ${SBLIB_TEST_CASE_SRC})
    target_include_directories(${TARGET_X86} PRIVATE ${INCLUDE_DIRECTORIES})
    add_subdirectory(${SBLIB_TEST_PATH} sblib-test_x86)
    target_compile_options(${TARGET_X86} PRIVATE
        -m32
    )
    target_link_options(${TARGET_X86} PRIVATE
        -m32
    )
    target_link_libraries(${TARGET_X86} sblib-test_x86)

    # Register test with Catch2 options for better output
    add_test(NAME ${TARGET_X86} 
        COMMAND ${TARGET_X86} --reporter console --success
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # Add test to run with JUnit XML output
    add_test(NAME ${TARGET_X86}_junit
        COMMAND ${TARGET_X86} --reporter junit --out $<TARGET_FILE_DIR:${TARGET_X86}>/${TARGET_X86}_result.xml
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_X86}>
    )

# Check if building for 64bit architecture
elseif(SBLIB_ARCHITECTURE STREQUAL "x64")
    add_executable(${TARGET_X64} ${SBLIB_TEST_CASE_SRC})
    target_include_directories(${TARGET_X64} PRIVATE ${INCLUDE_DIRECTORIES})
    add_subdirectory(${SBLIB_TEST_PATH} sblib-test_x64)
    target_compile_options(${TARGET_X64} PRIVATE
        -m64
    )
    target_link_options(${TARGET_X64} PRIVATE 
        -m64
    )
    target_link_libraries(${TARGET_X64} sblib-test_x64)

    # Register test with Catch2 options for better output
    add_test(NAME ${TARGET_X64}
        COMMAND ${TARGET_X64} --reporter console --success
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # Add test to run with JUnit XML output
    add_test(NAME ${TARGET_X64}_junit
        COMMAND ${TARGET_X64} --reporter junit --out $<TARGET_FILE_DIR:${TARGET_X64}>/${TARGET_X64}_result.xml
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_X64}>
    )
else()
    message(FATAL_ERROR "Unknown architecture ${SBLIB_ARCHITECTURE}. (project ${PROJECT_NAME})")
endif()