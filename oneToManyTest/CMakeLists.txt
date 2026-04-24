cmake_minimum_required(VERSION 3.10)
project(oneToManyTest)

# Find the ZeroMQ C library (libzmq)
# This populates ZMQ_LIBRARIES and ZMQ_INCLUDE_DIRS
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Get repo root using git
execute_process(
    COMMAND git rev-parse --show-toplevel
    OUTPUT_VARIABLE REPO_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

link_directories(${ZMQ_LIBRARY_DIRS})

message(STATUS "ZeroMQ include dirs: ${ZMQ_INCLUDE_DIRS}")
message(STATUS "ZeroMQ libraries: ${ZMQ_LIBRARIES}")


add_executable(oneToManyTest onetomany.cpp)
target_link_libraries(oneToManyTest PUBLIC ${ZMQ_LIBRARIES})
target_include_directories(oneToManyTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${ZMQ_INCLUDE_DIRS})

