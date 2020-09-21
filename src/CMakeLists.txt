add_library(InfluxDB
  InfluxDB.cxx
  Point.cxx
  InfluxDBFactory.cxx
  $<$<BOOL:${Boost_FOUND}>:UDP.cxx>
  $<$<BOOL:${Boost_FOUND}>:UnixSocket.cxx>
  HTTP.cxx
)


target_include_directories(InfluxDB
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Link targets
target_link_libraries(InfluxDB
  PRIVATE
    $<$<BOOL:${Boost_FOUND}>:Boost::system>
    CURL::libcurl
)


# Use C++17
target_compile_features(InfluxDB PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

# Set compile definition if Boost found
target_compile_definitions(InfluxDB
  PRIVATE
    $<$<BOOL:${Boost_FOUND}>:INFLUXDB_WITH_BOOST>
)
