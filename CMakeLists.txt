# CMakeLists files in this project can
cmake_minimum_required (VERSION 2.8.11)

project(sid_canutils)

add_subdirectory( lib )

find_package(Threads REQUIRED)

add_library (
    sid_canutils
    app/src/sid_canutils.cpp
    )

target_include_directories ( sid_canutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app/include )

target_link_libraries( sid_canutils socketcan_wrapper "${CMAKE_THREAD_LIBS_INIT}" )

add_executable(
    sid_canutils_test
    test/test.cpp
)

target_link_libraries( sid_canutils_test sid_canutils )
