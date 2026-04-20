# Create initial C, C++ compiler flags
set(COMPILER_FLAGS_LIST
    -Wall -Wlogical-op -Woverloaded-virtual -Wextra -pedantic
    # TODO warnings to enable in future
    # -Wconversion
    -g3 -gdwarf-4
    -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti
    -fmerge-constants -mcpu=cortex-m0 -mthumb -fstack-usage -specs=nano.specs
    -fmacro-prefix-map=\"${CMAKE_SOURCE_DIR}/\"=
)
string(JOIN " " COMPILER_FLAGS ${COMPILER_FLAGS_LIST})

set(COMPILER_FLAGS_DEBUG "-DDEBUG -O0")
set(COMPILER_FLAGS_RELEASE "-DNDEBUG -Os -flto -ffat-lto-objects")
set(COMPILER_FLAGS_MINSIZEREL ${COMPILER_FLAGS_RELEASE})
set(COMPILER_FLAGS_RELWITHDEBINFO ${COMPILER_FLAGS_RELEASE})