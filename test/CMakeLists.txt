
find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)


add_library(TestMain STATIC TestMain.cxx)
target_link_libraries(TestMain PUBLIC Catch2::Catch2 trompeloeil::trompeloeil)

add_subdirectory("mock")


function(add_unittest name)
    add_executable(${name} ${name}.cxx)
    target_link_libraries(${name} PRIVATE TestMain InfluxDB)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unittest(PointTest)
add_unittest(InfluxDBTest)
add_unittest(InfluxDBFactoryTest)

add_unittest(HttpTest)
target_link_libraries(HttpTest PRIVATE InfluxDB-Http CurlMock)
target_include_directories(HttpTest PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_unittest(NoBoostSupportTest)
target_sources(NoBoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/NoBoostSupport.cxx)
target_include_directories(NoBoostSupportTest PRIVATE ${CMAKE_SOURCE_DIR}/src)

if (Boost_FOUND)
    add_unittest(BoostSupportTest)
    target_sources(BoostSupportTest PRIVATE
        ${PROJECT_SOURCE_DIR}/src/BoostSupport.cxx
        ${PROJECT_SOURCE_DIR}/src/UnixSocket.cxx
        ${PROJECT_SOURCE_DIR}/src/UDP.cxx
        )
    target_link_libraries(BoostSupportTest PRIVATE Threads::Threads Boost::system)
    target_include_directories(BoostSupportTest PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()


add_custom_target(unittest PointTest
    COMMAND InfluxDBTest
    COMMAND InfluxDBFactoryTest
    COMMAND HttpTest
    COMMAND NoBoostSupportTest
    COMMAND $<$<BOOL:${Boost_FOUND}>:BoostSupportTest>

    COMMENT "Running unit tests\n\n"
    VERBATIM
    )


if (Boost_FOUND)
    add_dependencies(unittest BoostSupportTest)

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

        if(${test_name} MATCHES "testUdp" OR ${test_name} MATCHES "testHttp")
            # for testing the not-exported interface of InfluxDB,
            # we have to explicitly add the sources / symbols
            target_link_libraries(${test_name} PRIVATE InfluxDB-Http)
        endif()

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

    add_custom_target(systemtest testUdp
        COMMAND testPoint
        COMMAND testHttp
        COMMAND testQuery
        COMMAND testFactory
        COMMAND testInfluxDB

        COMMENT "Running system tests\n\n"
        VERBATIM
        )
endif()
