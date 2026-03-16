# Create initial Assembler flags
message(STATUS "Assembler flags in ${CURRENT_FILE_NAME} are untested.")
set(ASM_FLAGS_LIST
    # TODO ASM flags are untested
    -c -x assembler-with-cpp -gdwarf-4 -mcpu=cortex-m0 -mthumb -specs=nano.specs
)
string(JOIN " " ASM_FLAGS ${ASM_FLAGS_LIST})
set(ASM_FLAGS_DEBUG "-DDEBUG -g3") # cmake doesn't add by default -DDEBUG for debug builds
set(ASM_FLAGS_RELEASE "-DNDEBUG")