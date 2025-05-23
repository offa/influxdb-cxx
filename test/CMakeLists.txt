
find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)

if (NOT TARGET trompeloeil::trompeloeil)
    add_library(trompeloeil::trompeloeil INTERFACE IMPORTED)
    target_link_libraries(trompeloeil::trompeloeil INTERFACE trompeloeil)
endif()

if (MSVC)
    target_compile_definitions(trompeloeil::trompeloeil INTERFACE NOMINMAX)
endif()

add_subdirectory("mock")


function(add_unittest name)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(TEST_OPTION "" "" ${multiValueArgs} ${ARGN})

    add_executable(${name} ${name}.cxx)
    target_link_libraries(${name} PRIVATE
        ${TEST_OPTION_DEPENDS}
        Catch2::Catch2WithMain
        trompeloeil::trompeloeil
        Threads::Threads
        )
    target_include_directories(${name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/mock
            )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unittest(PointTest DEPENDS InfluxDB)
add_unittest(LineProtocolTest DEPENDS InfluxDB InfluxDB-Internal)
add_unittest(InfluxDBTest DEPENDS InfluxDB)
add_unittest(InfluxDBFactoryTest DEPENDS InfluxDB)
add_unittest(ProxyTest DEPENDS InfluxDB)
add_unittest(HttpTest DEPENDS InfluxDB-Core InfluxDB-Internal InfluxDB-BoostSupport CprMock Threads::Threads)
add_unittest(UriParserTest)

add_unittest(NoBoostSupportTest)
target_sources(NoBoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/NoBoostSupport.cxx)
target_link_libraries(NoBoostSupportTest PRIVATE InfluxDB)

if (INFLUXCXX_WITH_BOOST)
    add_unittest(BoostSupportTest DEPENDS InfluxDB-BoostSupport InfluxDB Boost::system date::date)
endif()


add_custom_target(unittest PointTest
    COMMAND LineProtocolTest
    COMMAND InfluxDBTest
    COMMAND InfluxDBFactoryTest
    COMMAND ProxyTest
    COMMAND HttpTest
    COMMAND UriParserTest
    COMMAND NoBoostSupportTest
    COMMAND $<$<AND:$<BOOL:${INFLUXCXX_WITH_BOOST}>,$<NOT:$<PLATFORM_ID:Windows>>>:BoostSupportTest>

    COMMENT "Running unit tests\n\n"
    VERBATIM
    )


if (INFLUXCXX_WITH_BOOST)
    add_dependencies(unittest BoostSupportTest)
endif()


if (INFLUXCXX_SYSTEMTEST)
    add_subdirectory(system)
endif()
