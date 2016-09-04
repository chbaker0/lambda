cmake_minimum_required(VERSION 3.1)
project(lambda)

include_directories(eval parse)

add_subdirectory(parse)
add_subdirectory(eval)

add_executable(main main.cpp)
set_target_properties(main PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(main parse)
