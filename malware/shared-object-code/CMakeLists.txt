cmake_minimum_required(VERSION 3.10) 
project(PeopleCounter)

#Find OpenCV

find_package(OpenCV REQUIRED) 
include_directories(${OpenCV_INCLUDE_DIRS})

#Find JNI

find_package(JNI REQUIRED) 
include_directories(${JNI_INCLUDE_DIRS})

add_library(peoplecounter SHARED PeopleCounter.cpp)

#Link libraries

target_link_libraries(peoplecounter ${OpenCV_LIBRARIES})