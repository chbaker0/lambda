cmake_minimum_required(VERSION 3.1)
project(lambda)

include_directories(eval parse)

add_subdirectory(parse)
add_subdirectory(eval)

add_executable(repl repl.cpp)
set_target_properties(repl PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(repl parse eval)
