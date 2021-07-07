set(INTERNAL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/src
    )

add_library(InfluxDB-Http OBJECT HTTP.cxx)
target_include_directories(InfluxDB-Http PRIVATE ${INTERNAL_INCLUDE_DIRS})
target_include_directories(InfluxDB-Http SYSTEM PUBLIC $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>)


add_library(InfluxDB-BoostSupport OBJECT
    $<$<NOT:$<BOOL:${INFLUXCXX_WITH_BOOST}>>:NoBoostSupport.cxx>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:BoostSupport.cxx>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:UDP.cxx>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:TCP.cxx>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:UnixSocket.cxx>
    )
target_include_directories(InfluxDB-BoostSupport PRIVATE ${INTERNAL_INCLUDE_DIRS})

target_link_libraries(InfluxDB-BoostSupport
    PRIVATE
        $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:Boost::boost>
        $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:Boost::system>
        date
        )

# #117: Workaround for Boost ASIO null-dereference
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "12")
    set_source_files_properties(UDP.cxx TCP.cxx UnixSocket.cxx PROPERTIES COMPILE_OPTIONS "-Wno-null-dereference")
endif()

add_library(InfluxDB-Internal OBJECT LineProtocol.cxx)
target_include_directories(InfluxDB-Internal PRIVATE ${INTERNAL_INCLUDE_DIRS})


add_library(InfluxDB
    InfluxDB.cxx
    Point.cxx
    InfluxDBFactory.cxx
    Proxy.cxx
    $<TARGET_OBJECTS:InfluxDB-Internal>
    $<TARGET_OBJECTS:InfluxDB-Http>
    $<TARGET_OBJECTS:InfluxDB-BoostSupport>
    )
add_library(InfluxData::InfluxDB ALIAS InfluxDB)

#
# Here are a set of rules to help you update your library version information:
#
#    If the library source code has changed at all since the last update,
#        then increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
#    If any interfaces have been added, removed, or changed since the last update,
#        increment current, and set revision to 0.
#    If any interfaces have been added since the last public release,
#        then increment age.
#    If any interfaces have been removed or changed since the last public release,
#        then set age to 0.
#
# set_target_properties(InfluxDB PROPERTIES VERSION c.r.a SOVERSION c)
#
set_target_properties(InfluxDB PROPERTIES VERSION 0.0.0 SOVERSION 0)

generate_export_header(InfluxDB)

target_include_directories(InfluxDB
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    # for export header
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link targets
target_link_libraries(InfluxDB
  PRIVATE
    CURL::libcurl
    Threads::Threads
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:Boost::boost>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:Boost::system>
)

# Use C++17
target_compile_features(InfluxDB PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
