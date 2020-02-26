cmake_minimum_required(VERSION 2.8)

#=============================#
#      Setup Project          #
#=============================#
project (fano C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c99")

# define folders
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

#=============================#
#     Dependencies            #
#=============================#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#=============================#
#     Setup Source            #
#=============================#
set(FANO_SOURCES
    src/core.c
    src/editor.c
    src/terminal.c
    src/main.c
)

set(FANO_HEADERS
    include/config.h
    include/cons.h
    include/core.h
    include/editor.h
    include/terminal.h
)

#=============================#
#     Setup Output            #
#=============================#
add_executable(fano ${FANO_SOURCES} ${FANO_HEADERS})
