include_directories(include)
add_library(uart uart.c)
target_link_libraries(uart microkit)