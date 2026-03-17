function(clang22_workaround_target targetname)
    # Workaround for catch2 #3076
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "22")
        target_compile_options(${targetname} PRIVATE -Wno-c2y-extensions)
    endif()
endfunction()
