project(asteroidsyncservice)
cmake_minimum_required(VERSION 2.6)

find_package(Qt5Core)

set(CMAKE_AUTOMOC ON)

include_directories($ENV{GMOCK_HOME}/googlemock/include/gmock $ENV{GMOCK_HOME}/googletest/include/gtest)
link_directories($ENV{GMOCK_HOME}/googlemock $ENV{GMOCK_HOME}/googletest)
add_definitions(-std=c++0x)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall")

set(sources 
   main.cpp 
   asteroidsyncserviceTest.cpp
   ../asteroidsyncserviced/openweathermapparser.cpp
   readjsonfile.cpp)
add_executable(test ${sources})

target_link_libraries(test Qt5::Core)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
target_link_libraries(test pthread)
