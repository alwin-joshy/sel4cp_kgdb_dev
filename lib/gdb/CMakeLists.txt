include_directories(include)
add_library(gdb STATIC gdb.c arch/arm/64/gdb.c)
target_link_libraries(gdb microkit util uart)