cmake_minimum_required(VERSION 3.6)
project(forth)

set(CMAKE_C_STANDARD 11)


# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

set(SOURCE_FILES
        src/util.h
        src/util.c
        src/eval.c
        src/eval.h
        src/debug_helpers.c
        src/debug_helpers.h
        )

add_executable(forth ${SOURCE_FILES} src/main.c)

set(TEST_FILES ${SOURCE_FILES} test/minctest.h test/test_main.c)
add_executable(test_forth ${TEST_FILES})
