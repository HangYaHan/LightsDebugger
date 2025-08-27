cmake_minimum_required(VERSION 3.10.0)
project(LightsDebugger VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(LightsDebugger ${SRC_FILES})

include_directories(${CMAKE_SOURCE_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})  