
find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)


add_library(TestMain STATIC TestMain.cxx)
target_link_libraries(TestMain PUBLIC Catch2::Catch2 trompeloeil::trompeloeil)

add_subdirectory("mock")


function(add_unittest name)
    add_executable(${name} ${name}.cxx)
    target_link_libraries(${name} PRIVATE TestMain InfluxDB)
    target_include_directories(${name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/mock
            )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unittest(PointTest)
add_unittest(LineProtocolTest)
target_link_libraries(LineProtocolTest PRIVATE InfluxDB-Internal)
add_unittest(InfluxDBTest)
add_unittest(InfluxDBFactoryTest)

add_unittest(HttpTest)
target_link_libraries(HttpTest PRIVATE InfluxDB-Http CurlMock)

add_unittest(NoBoostSupportTest)
target_sources(NoBoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/NoBoostSupport.cxx)

if (Boost_FOUND)
    add_unittest(BoostSupportTest)
    target_link_libraries(BoostSupportTest PRIVATE InfluxDB-BoostSupport Threads::Threads Boost::system)
endif()


add_custom_target(unittest PointTest
    COMMAND LineProtocolTest
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
endif()


if (INFLUXCXX_SYSTEMTEST)
    add_subdirectory(system)
endif()
