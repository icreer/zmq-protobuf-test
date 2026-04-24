cmake_minimum_required(VERSION 3.10)
project(push_pull_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the ZeroMQ C library (libzmq)
# This populates ZMQ_LIBRARIES and ZMQ_INCLUDE_DIRS
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

find_package(Protobuf REQUIRED)


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS payload.proto)

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

add_executable(pushPullTest pushPullTest.cpp)
target_link_libraries(pushPullTest PUBLIC ${ZMQ_LIBRARIES})
target_include_directories(pushPullTest PUBLIC ${ZMQ_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS payload.proto)

add_executable(endToEndPushPullTest pushPullTestEndtoEnd.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(endToEndPushPullTest PUBLIC ${ZMQ_LIBRARIES} protobuf::libprotobuf)
target_include_directories(endToEndPushPullTest PUBLIC ${ZMQ_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
