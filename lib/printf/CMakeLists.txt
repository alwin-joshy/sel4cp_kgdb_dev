add_library(printf STATIC printf.c include/printf.h)
target_include_directories(printf PUBLIC include)
