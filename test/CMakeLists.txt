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

    add_executable(${test_name} ${test})
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
            )

    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 60)
  endforeach()
