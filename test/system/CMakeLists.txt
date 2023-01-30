add_executable(InfluxDBST InfluxDBST.cxx)
target_link_libraries(InfluxDBST PRIVATE InfluxDB Catch2::Catch2WithMain)

add_executable(HttpAuthST HttpAuthST.cxx)
target_link_libraries(HttpAuthST PRIVATE InfluxDB Catch2::Catch2WithMain)


add_custom_target(systemtest InfluxDBST
        COMMENT "Running system tests\n\n"
        VERBATIM
        )

add_custom_target(systemtest-auth HttpAuthST
        COMMENT "Running system tests with authentication\n\n"
        VERBATIM
        )

