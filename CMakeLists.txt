cmake_minimum_required(VERSION 3.6)
project(forth)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES src/util.h src/util.c src/eval_from_file.c src/eval_from_file.h)

add_executable(forth ${SOURCE_FILES} src/main.c)

set(TEST_FILES ${SOURCE_FILES} test/minctest.h test/test_main.c)
add_executable(test_forth ${TEST_FILES})
