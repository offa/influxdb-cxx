add_library(CurlMock STATIC CurlMock.cxx)
target_include_directories(CurlMock PUBLIC ".")
target_link_libraries(CurlMock PRIVATE Catch2::Catch2 trompeloeil)

target_include_directories(CurlMock SYSTEM PUBLIC $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(CurlMock
    PRIVATE
        BUILDING_LIBCURL
    PUBLIC
        $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_COMPILE_DEFINITIONS>
        )


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(CurlMock PRIVATE -Wno-varargs)
endif()
