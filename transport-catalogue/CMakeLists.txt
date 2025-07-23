cmake_minimum_required(VERSION 3.10)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(transport_catalogue ${SOURCES})
