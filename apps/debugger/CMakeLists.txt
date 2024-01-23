add_executable(debugger.elf ping.c)
target_link_libraries(debugger.elf microkit uart gdb)