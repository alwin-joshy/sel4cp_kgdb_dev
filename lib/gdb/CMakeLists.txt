add_library(gdb STATIC gdb.c arch/arm/64/gdb.c include/gdb.h include/arch/arm/64/gdb.h)
target_include_directories(gdb PUBLIC include)
target_link_libraries(gdb microkit util uart)