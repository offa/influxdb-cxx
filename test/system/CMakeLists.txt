if (NOT Boost_FOUND)
    message(SEND_ERROR "System tests require Boost")
endif()


add_executable(InfluxDBST InfluxDBST.cxx)
target_link_libraries(InfluxDBST PRIVATE InfluxDB InfluxDB-Http CURL::libcurl TestMain)
target_include_directories(InfluxDBST PRIVATE ${PROJECT_SOURCE_DIR}/src)


add_custom_target(systemtest2 InfluxDBST
        COMMENT "Running system tests\n\n"
        VERBATIM
        )
