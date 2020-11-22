add_library(CurlMock STATIC CurlMock.cxx)
target_include_directories(CurlMock PUBLIC ".")
target_link_libraries(CurlMock PRIVATE Catch2::Catch2 trompeloeil)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(CurlMock PRIVATE -Wno-varargs)
endif()
