add_executable(InfluxDBIT InfluxDBIT.cxx)
target_link_libraries(InfluxDBIT PRIVATE InfluxDB InfluxDB-Http CURL::libcurl TestMain)
target_include_directories(InfluxDBIT PRIVATE ${PROJECT_SOURCE_DIR}/src)


add_custom_target(integrationtest InfluxDBIT
        COMMENT "Running integration tests\n\n"
        VERBATIM
        )
