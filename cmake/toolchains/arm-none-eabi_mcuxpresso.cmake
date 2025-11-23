cmake_minimum_required(VERSION 3.31)
    if(NOT TOOLCHAIN_PREFIX)
        message(FATAL_ERROR "No TOOLCHAIN_PREFIX specified.\
                Specify path to arm-none-eabi toolchain with e.g. --DTOOLCHAIN_PREFIX=C:/nxp/MCUXpressoIDE_25.6.136/ide/tools")
    endif()

    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_SYSTEM_PROCESSOR arm)

    set(MIN_MCUXPRESSO_VERSION_SUPPORTED "11.9.0") # 11.9.0.2144 changed path from ../ide/binaries to ../ide/LinkServer/binaries

    # MCUXpresso version check
    string(TOLOWER "${TOOLCHAIN_PREFIX}" LOWER_TOOLCHAIN_PREFIX)
    string(REGEX MATCH "mcuxpressoide" MATCH_STR ${LOWER_TOOLCHAIN_PREFIX})
    if(NOT ${MATCH_STR} STREQUAL "")
        # Get version from specified path prefix
        string(REGEX MATCH "([0-9]+.[0-9]+.[0-9]+)" MCUXPRESSO_VERSION ${TOOLCHAIN_PREFIX})
        if(NOT ${MCUXPRESSO_VERSION} STREQUAL "")
            message(STATUS "MCUXpresso detected: v" ${MCUXPRESSO_VERSION})
            # Check if version is supported
            if(${MCUXPRESSO_VERSION} VERSION_LESS ${MIN_MCUXPRESSO_VERSION_SUPPORTED})
                message(FATAL_ERROR "MCUXpresso version not supported: ${MCUXPRESSO_VERSION}. Use ${MIN_MCUXPRESSO_VERSION_SUPPORTED} or newer.")
            endif()
        else()
            message(NOTICE "Could not check MCUXpresso version. Build may not work correctly.")
        endif()
    else()
        message(NOTICE "Could not find MCUXpresso in ${TOOLCHAIN_PREFIX}. Build may not work correctly.")
    endif()

    if(CMAKE_HOST_EXECUTABLE_SUFFIX STREQUAL ".exe") # CMAKE_HOST_EXECUTABLE_SUFFIX since CMake 3.31
        set(BATCH_SUFFIX ".cmd")
    else()
        set(BATCH_SUFFIX "")
    endif()

    set(TARGET_TRIPLET "arm-none-eabi")

    # Where we find NXP tools for flashing and debugging
    get_filename_component(LINK_SERVER_BIN ${TOOLCHAIN_PREFIX}/../LinkServer/binaries REALPATH CACHE "Path to LinkServer binaries")
    set(BOOT_LINK1 ${LINK_SERVER_BIN}/boot_link1${BATCH_SUFFIX} CACHE FILEPATH "boot_link1 Filename")
    set(BOOT_LINK2 ${LINK_SERVER_BIN}/boot_link2${BATCH_SUFFIX} CACHE FILEPATH "boot_link2 Filename")
    set(REDLINK ${LINK_SERVER_BIN}/crt_emu_cm_redlink${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "redlink")
    if(NOT EXISTS ${BOOT_LINK1})
        message(FATAL_ERROR "boot_link1 file does not exist: " ${BOOT_LINK1})
    endif()

    if(NOT EXISTS ${BOOT_LINK2})
        message(FATAL_ERROR "boot_link2 file does not exist: " ${BOOT_LINK2})
    endif()

    if(NOT EXISTS ${REDLINK})
        message(FATAL_ERROR "crt_emu_cm_redlink file does not exist: " ${REDLINK})
    endif()

    get_filename_component(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin REALPATH CACHE)
    get_filename_component(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include REALPATH CACHE)
    get_filename_component(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib REALPATH CACHE)

    set(TOOLS_PREFIX ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET})
    set(CMAKE_C_COMPILER ${TOOLS_PREFIX}-gcc${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "c compiler")
    set(CMAKE_CXX_COMPILER ${TOOLS_PREFIX}-g++${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "cxx compiler")
    set(CMAKE_ASM_COMPILER ${TOOLS_PREFIX}-as${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "asm compiler")
    set(CMAKE_OBJCOPY ${TOOLS_PREFIX}-objcopy${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "objcopy")
    set(CMAKE_OBJDUMP ${TOOLS_PREFIX}-objdump${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "objdump")
    set(CMAKE_AR ${TOOLS_PREFIX}-ar${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "archiver")
    set(CMAKE_STRIP ${TOOLS_PREFIX}-strip${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "strip")
    set(CMAKE_SIZE ${TOOLS_PREFIX}-size${CMAKE_HOST_EXECUTABLE_SUFFIX} CACHE FILEPATH "size")

    if(NOT EXISTS ${CMAKE_C_COMPILER})
        message(FATAL_ERROR "CMAKE_C_COMPILER file does not exist: ${CMAKE_C_COMPILER}")
    endif()
    if(NOT EXISTS ${CMAKE_CXX_COMPILER})
        message(FATAL_ERROR "CMAKE_CXX_COMPILER file does not exist: ${CMAKE_CXX_COMPILER}")
    endif()
    if(NOT EXISTS ${CMAKE_ASM_COMPILER})
        message(FATAL_ERROR "CMAKE_ASM_COMPILER file does not exist: ${CMAKE_ASM_COMPILER}")
    endif()
    if(NOT EXISTS ${CMAKE_OBJCOPY})
        message(FATAL_ERROR "CMAKE_OBJCOPY file does not exist: ${CMAKE_OBJCOPY}")
    endif()
    if(NOT EXISTS ${CMAKE_OBJDUMP})
        message(FATAL_ERROR "CMAKE_OBJDUMP file does not exist: ${CMAKE_OBJDUMP}")
    endif()
    if(NOT EXISTS ${CMAKE_AR})
        message(FATAL_ERROR "CMAKE_AR file does not exist: ${CMAKE_AR}")
    endif()
    if(NOT EXISTS ${CMAKE_STRIP})
        message(FATAL_ERROR "CMAKE_STRIP file does not exist: ${CMAKE_STRIP}")
    endif()
    if(NOT EXISTS ${CMAKE_SIZE})
        message(FATAL_ERROR "CMAKE_SIZE file does not exist: ${CMAKE_SIZE}")
    endif()

    # Adjust the default behaviour of the FIND_XXX() commands:
    # i)    Search headers and libraries in the target environment
    # ii)   Search programs in the host environment
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    set(CMAKE_COMPILER_IS_GNUCC     1)
    set(CMAKE_C_COMPILER_ID         GNU)
    set(CMAKE_C_COMPILER_ID_RUN     TRUE)
    set(CMAKE_C_COMPILER_FORCED     TRUE)
    set(CMAKE_CXX_COMPILER_ID       GNU)
    set(CMAKE_CXX_COMPILER_ID_RUN   TRUE)
    set(CMAKE_CXX_COMPILER_FORCED   TRUE)



set(FLAGS_LIST 
    -Wall -Wlogical-op -Wextra
    -g3 -gdwarf-4
    -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-exceptions
    -fmerge-constants -mcpu=cortex-m0 -mthumb -fstack-usage -specs=nano.specs
    -fmacro-prefix-map=\"${CMAKE_SOURCE_DIR}/\"=
)
string(JOIN " " FLAGS ${FLAGS_LIST})

set(FLAGS_DEBUG "-O0")
set(FLAGS_RELEASE "-Os -flto -ffat-lto-objects")

# Set initial C flags 
set(CMAKE_C_FLAGS_INIT ${FLAGS} CACHE INTERNAL "C compiler default flags" FORCE)
set(CMAKE_C_FLAGS_DEBUG_INIT ${FLAGS_DEBUG} CACHE INTERNAL "C compiler Debug flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE_INIT ${FLAGS_RELEASE} CACHE INTERNAL "C compiler Release flags" FORCE)

# Set initial C++ flags
set(CMAKE_CXX_FLAGS_INIT ${FLAGS} CACHE INTERNAL "C++ compiler default flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG_INIT ${FLAGS_DEBUG} CACHE INTERNAL "C++ compiler Debug flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE_INIT ${FLAGS_RELEASE} CACHE INTERNAL "C++ compiler Release flags" FORCE)

# todo: Set initial, debug and release ASM flags
#set(CMAKE_ASM<DIALECT>_FLAGS_INIT.
#    ""
#    CACHE INTERNAL "ASM compiler default flags" FORCE
#)

# todo: Set initial linker flags
#set(CMAKE_EXE_LINKER_FLAGS_INIT
#    ""
#    CACHE INTERNAL "Executable linker default flags" FORCE
#)
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT
#    ""
#    CACHE INTERNAL "Executable linker default flags" FORCE
#)
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT
#    ""
#    CACHE INTERNAL "Executable linker default flags" FORCE
#)