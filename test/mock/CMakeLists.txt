add_library(CprMock STATIC CprMock.cxx)
target_include_directories(CprMock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CprMock PRIVATE Catch2::Catch2 trompeloeil::trompeloeil)
target_include_directories(CprMock SYSTEM PUBLIC $<TARGET_PROPERTY:cpr::cpr,INTERFACE_INCLUDE_DIRECTORIES>)
