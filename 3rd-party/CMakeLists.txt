add_library(date INTERFACE)
target_include_directories(date INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
add_library(date::date ALIAS date)
