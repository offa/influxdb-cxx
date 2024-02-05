set(INTERNAL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/src
    )


add_library(InfluxDB-BoostSupport OBJECT
    $<$<NOT:$<BOOL:${INFLUXCXX_WITH_BOOST}>>:NoBoostSupport.cxx>
    $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:BoostSupport.cxx UDP.cxx TCP.cxx UnixSocket.cxx>
    )
target_include_directories(InfluxDB-BoostSupport PRIVATE ${INTERNAL_INCLUDE_DIRS})

target_link_libraries(InfluxDB-BoostSupport PRIVATE date::date)

if (INFLUXCXX_WITH_BOOST)
    target_link_libraries(InfluxDB-BoostSupport PRIVATE Boost::boost Boost::system)
endif()

# #117: Workaround for Boost ASIO null-dereference
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "12")
    set_source_files_properties(UDP.cxx TCP.cxx UnixSocket.cxx PROPERTIES COMPILE_OPTIONS "-Wno-null-dereference")
endif()

add_library(InfluxDB-Internal OBJECT LineProtocol.cxx HTTP.cxx)
target_include_directories(InfluxDB-Internal PRIVATE ${INTERNAL_INCLUDE_DIRS})
target_link_libraries(InfluxDB-Internal PRIVATE cpr::cpr)


add_library(InfluxDB-Core OBJECT
  InfluxDB.cxx
  Point.cxx
  InfluxDBFactory.cxx
  InfluxDBBuilder.cxx
  Proxy.cxx
  )
target_include_directories(InfluxDB-Core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/src
)
target_link_libraries(InfluxDB-Core PRIVATE cpr::cpr)


add_library(InfluxDB
    $<TARGET_OBJECTS:InfluxDB-Core>
    $<TARGET_OBJECTS:InfluxDB-Internal>
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
set(SO_VERSION_MAJOR 1)
set_target_properties(InfluxDB PROPERTIES
  VERSION ${SO_VERSION_MAJOR}.0.0
  SOVERSION ${SO_VERSION_MAJOR}
)

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
  PUBLIC
    cpr::cpr
    Threads::Threads
)

# Use C++17
target_compile_features(InfluxDB PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
