cmake_minimum_required(VERSION 3.30)

include("${CMAKE_SOURCE_DIR}/cmake/CDataPack.cmake")

project(Hello VERSION 1.0)

add_executable(packdata src/pack.c)

add_library(unpackdata STATIC src/unpack.c)

add_executable(test src/test.c)
target_link_libraries(test unpackdata)

pack_data(
    test
    "${CMAKE_SOURCE_DIR}/res"
)