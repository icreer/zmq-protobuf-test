cmake_minimum_required(VERSION 3.10)
project(zmq-protobuf-test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find the Protobuf package
find_package(Protobuf REQUIRED)

# 2. Find ZeroMQ (required for the other benchmarks in your project)
# find_package(ZeroMQ REQUIRED)

# 3. Generate C++ source and header files from payload.proto
# This automatically runs protoc and adds the generated files to PROTO_SRCS and PROTO_HDRS
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS payload.proto)

# 4. Define the Protobuf benchmark executable
add_executable(protobuftest protobuftest.cpp ${PROTO_SRCS} ${PROTO_HDRS})

# 5. Link against Protobuf and set include directories
# Using protobuf::libprotobuf is the modern CMake way to handle dependencies and includes
target_link_libraries(protobuftest PRIVATE protobuf::libprotobuf)
target_include_directories(protobuftest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# 6. Optimization for Performance Testing
# Benchmark results are significantly impacted by compiler optimizations
if(NOT MSVC)
    target_compile_options(protobuftest PRIVATE -O3)
endif()
