# Workaround until Catch can be integrated through conan.
Include(FetchContent)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.3)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(trompeloeil
    GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
    GIT_TAG v39)
FetchContent_MakeAvailable(trompeloeil)

add_library(TestMain STATIC TestMain.cpp)
target_link_libraries(TestMain PUBLIC Catch2::Catch2 trompeloeil)


function(add_unittest name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE TestMain InfluxDB)
    target_include_directories(${name} PRIVATE ${PROJECT_BINARY_DIR}/src)
    add_test(${name} ${name})
endfunction()

add_unittest(PointTest)
add_unittest(InfluxDBTest)


if (Boost_FOUND)
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
endif()
