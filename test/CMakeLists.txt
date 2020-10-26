  set(TEST_SRCS
    testUdp.cxx
    testPoint.cxx
    testHttp.cxx
    testQuery.cxx
    testFactory.cxx
    testInfluxDB.cxx
  )
  foreach (test ${TEST_SRCS})
    get_filename_component(test_name ${test} NAME)
    string(REGEX REPLACE ".cxx" "" test_name ${test_name})

    if(${test_name} MATCHES "testUdp")
      # for testing the not-exported interface of InfluxDB,
      # we have to explicitly add the sources / symbols
      set(INFLUXDB_PRIVATE_SRCS ${PROJECT_SOURCE_DIR}/src/HTTP.cxx)
    endif()
    add_executable(${test_name} ${test} "${INFLUXDB_PRIVATE_SRCS}")
    target_link_libraries(${test_name}
      PRIVATE
        InfluxDB
        Boost::unit_test_framework
        Threads::Threads
        CURL::libcurl
    )

    target_include_directories(${test_name}
            PRIVATE
              ${CMAKE_SOURCE_DIR}/src
              ${PROJECT_BINARY_DIR}/src
            )

    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 60)
  endforeach()
