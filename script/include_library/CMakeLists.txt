cmake_minimum_required(VERSION 3.16)
project(influxcxx-test)

if(INFLUXCXX_AS_SUBDIR)
    message(STATUS "include as subdirectory")
    add_subdirectory(influxdb-cxx)
else()
    message(STATUS "include using find_package")
    find_package(InfluxDB REQUIRED)
endif()

add_executable(influxcxx-writer main.cpp)
target_link_libraries(influxcxx-writer PRIVATE InfluxData::InfluxDB)
