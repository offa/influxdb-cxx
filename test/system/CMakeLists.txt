add_executable(InfluxDBST InfluxDBST.cxx)
target_link_libraries(InfluxDBST PRIVATE InfluxDB InfluxDB-Http CURL::libcurl Catch2::Catch2WithMain)
target_include_directories(InfluxDBST PRIVATE ${PROJECT_SOURCE_DIR}/src)


add_custom_target(systemtest InfluxDBST
        COMMENT "Running system tests\n\n"
        VERBATIM
        )
