message(STATUS "Set CMAKE_<LANG>_FLAGS_INIT flags.")

# Don´t use FORCE option here to avoid overwriting user-defined flags
# Check out https://stackoverflow.com/questions/72549634/cmake-toolchain-file-setting-cmake-cxx-flags

# Set initial C flags
set(CMAKE_C_FLAGS_INIT ${COMPILER_FLAGS} CACHE STRING "Initial C compiler default flags")
set(CMAKE_C_FLAGS_DEBUG_INIT ${COMPILER_FLAGS_DEBUG} CACHE STRING "Initial C compiler Debug flags")
set(CMAKE_C_FLAGS_RELEASE_INIT ${COMPILER_FLAGS_RELEASE} CACHE STRING "Initial C compiler Release flags")
# Set same release C compiler flags for MinSizeRel and RelWithDebInfo to stay consistent with MCUxpresso configs
set(CMAKE_C_FLAGS_MINSIZEREL_INIT ${COMPILER_FLAGS_MINSIZEREL} CACHE STRING "Initial C compiler minimal size Release flags")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT ${COMPILER_FLAGS_RELWITHDEBINFO} CACHE STRING "Initial C compiler Release with debug info flags")

# Set initial C++ flags
set(CMAKE_CXX_FLAGS_INIT ${COMPILER_FLAGS} CACHE STRING "Initial C++ compiler default flags")
set(CMAKE_CXX_FLAGS_DEBUG_INIT ${COMPILER_FLAGS_DEBUG} CACHE STRING "Initial C++ compiler Debug flags")
set(CMAKE_CXX_FLAGS_RELEASE_INIT ${COMPILER_FLAGS_RELEASE} CACHE STRING "Initial C++ compiler Release flags")
# Set same release C++ compiler flags for MinSizeRel and RelWithDebInfo to stay consistent with MCUxpresso configs
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT ${COMPILER_FLAGS_MINSIZEREL} CACHE STRING "Initial C++ compiler minimal size Release flags")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT ${COMPILER_FLAGS_RELWITHDEBINFO} CACHE STRING "Initial C++ compiler Release with debug info flags")

# Set initial Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT ${LINKER_FLAGS} CACHE STRING "Initial Linker default flags")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT ${LINKER_FLAGS_DEBUG} CACHE STRING "Initial Linker Debug flags")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT ${LINKER_FLAGS_RELEASE} CACHE STRING "Initial Linker Release flags")
# Set same release linker flags for MinSizeRel and RelWithDebInfo to stay consistent with MCUxpresso configs
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT ${LINKER_FLAGS_RELEASE} CACHE STRING "Initial Linker minimal size Release flags")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT ${LINKER_FLAGS_RELEASE} CACHE STRING "Initial Linker Release with debug info flags")

# Set initial Assembler flags
# TODO Check if it's ASM_FLAGS or ASMFLAGS_FLAGS like in cmake documentation
#      https://cmake.org/cmake/help/latest/envvar/ASM_DIALECTFLAGS.html
set(CMAKE_ASM_FLAGS_INIT ${ASM_FLAGS} CACHE STRING "Initial ASM default flags")
set(CMAKE_ASM_FLAGS_DEBUG_INIT ${ASM_FLAGS_DEBUG} CACHE STRING "Initial ASM Debug flags")
set(CMAKE_ASM_FLAGS_RELEASE_INIT ${ASM_FLAGS_RELEASE} CACHE STRING "Initial ASM Release flags")
#set same release ASM flags for MinSizeRel and RelWithDebInfo to stay consistent with MCUxpresso configs
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT ${ASM_FLAGS_RELEASE} CACHE STRING "Initial ASM minimal size Release flags")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT ${ASM_FLAGS_RELEASE} CACHE STRING "Initial ASM Release with debug info flags")