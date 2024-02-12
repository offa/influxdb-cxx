cmake_minimum_required(VERSION 3.12)
project(test_package CXX)

find_package(InfluxDB REQUIRED)

add_executable(test_package example.cpp)
target_link_libraries(test_package PRIVATE InfluxData::InfluxDB)
target_compile_features(test_package PRIVATE cxx_std_20)
