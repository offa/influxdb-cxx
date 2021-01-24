set(INTERNAL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/src
    )

add_library(InfluxDB-Http OBJECT HTTP.cxx)
target_include_directories(InfluxDB-Http PRIVATE ${INTERNAL_INCLUDE_DIRS})
target_include_directories(InfluxDB-Http SYSTEM PUBLIC $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>)


add_library(InfluxDB-BoostSupport OBJECT
    $<$<NOT:$<BOOL:${Boost_FOUND}>>:NoBoostSupport.cxx>
    $<$<BOOL:${Boost_FOUND}>:BoostSupport.cxx>
    $<$<BOOL:${Boost_FOUND}>:UDP.cxx>
    $<$<BOOL:${Boost_FOUND}>:UnixSocket.cxx>
    )
target_include_directories(InfluxDB-BoostSupport PRIVATE ${INTERNAL_INCLUDE_DIRS})
target_link_libraries(InfluxDB-BoostSupport PRIVATE $<$<BOOL:${Boost_FOUND}>:Boost::system>)


add_library(InfluxDB
    InfluxDB.cxx
    Point.cxx
    InfluxDBFactory.cxx
    $<TARGET_OBJECTS:InfluxDB-Http>
    $<TARGET_OBJECTS:InfluxDB-BoostSupport>
    )
add_library(InfluxData::InfluxDB ALIAS InfluxDB)

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
)

# Use C++17
target_compile_features(InfluxDB PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
