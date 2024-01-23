cmake_minimum_required(VERSION 3.26)

set(CMAKE_SYSTEM_NAME Generic)

# Microkit setup
set(MICROKIT_SDK "/Users/alwinjoshy/work/sel4cp_custom_sdk/sel4cp/release/microkit-sdk-1.2.6")
set(MICROKIT_CONFIG debug)
set(CPU cortex-a53)
set(BOARD odroidc2)
set(BOARD_DIR ${MICROKIT_SDK}/board/${BOARD}/${MICROKIT_CONFIG})
set(MICROKIT_TOOL ${MICROKIT_SDK}/bin/microkit)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(IMAGE_FILE ${EXECUTABLE_OUTPUT_PATH}/loader.img)
set(REPORT_FILE ${EXECUTABLE_OUTPUT_PATH}/report.txt)

# Compiler shit
set(CMAKE_C_COMPILER aarch64-none-elf-gcc)
set(CMAKE_C_FLAGS "-mcpu=${CPU} -mstrict-align -nostdlib -ffreestanding -g3 -Wall -Wno-array-bounds -Wno-unused-variable -Wno-unused-function -Werror -DBOARD_${BOARD}")
set(CMAKE_EXE_LINKER_FLAGS "-T ${BOARD_DIR}/lib/microkit.ld")

project(my_system C)

include_directories(${BOARD_DIR}/include)
link_directories(${BOARD_DIR}/lib)

add_library(microkit STATIC IMPORTED)
set_target_properties(microkit PROPERTIES IMPORTED_LOCATION ${BOARD_DIR}/lib/libmicrokit.a)

#add_subdirectory(lib/gdb)
add_subdirectory(lib/printf)
add_subdirectory(lib/uart)
add_subdirectory(lib/util)

add_subdirectory(apps/ping)
add_subdirectory(apps/pong)

add_custom_target(image ALL # OUTPUT ${IMAGE_FILE}
                   COMMAND ${MICROKIT_TOOL} ${CMAKE_SOURCE_DIR}/example.system --search-path ${EXECUTABLE_OUTPUT_PATH}
                            --board ${BOARD} --config ${MICROKIT_CONFIG} -o ${IMAGE_FILE} -r ${REPORT_FILE}
                   DEPENDS ping.elf pong.elf
        )