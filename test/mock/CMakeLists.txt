add_library(CurlMock STATIC CurlMock.cxx)
target_include_directories(CurlMock PUBLIC ".")
target_link_libraries(CurlMock PRIVATE Catch2::Catch2 trompeloeil)
