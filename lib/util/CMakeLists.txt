add_library(util STATIC util.c include/util.h)
target_include_directories(util PUBLIC include)
target_link_libraries(util printf microkit)
