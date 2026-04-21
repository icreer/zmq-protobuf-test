#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_DEALER);
    
    std::ofstream outfile("benchmark_results.csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    outfile << "Size_Bytes,Time_Seconds,Throughput_MBs" << std::endl;

    int hwm = 10;
    socket.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
    
    // std::cout << "Binding to port 5555..." << std::endl;
    socket.bind("tcp://*:5555");

    while (true) {
        // Receive the size header
        size_t expected_size = 0;
        zmq::message_t size_msg;
        auto res = socket.recv(size_msg);
        if (!res) break;
        
        std::memcpy(&expected_size, size_msg.data(), sizeof(size_t));
        
        if (expected_size == 0) continue;

        // std::cout << "Expecting " << expected_size << " bytes..." << std::endl;

        // Receive the payload and measure time
        zmq::message_t payload;
        auto start = std::chrono::high_resolution_clock::now();
        
        auto res_payload = socket.recv(payload);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        if (res_payload) {
            double MBs = (expected_size / (1024.0 * 1024.0)) / diff.count();
            // std::cout << "Received " << payload.size() << " bytes" << std::endl;
            // std::cout << "Time: " << diff.count() << "s" << std::endl;
            // std::cout << "Throughput: " << MBs << " MB/s" << std::endl;
            // std::cout << "-----------------------------------" << std::endl;
            outfile << payload.size() << "," << diff.count() << "," << MBs << std::endl;
        }

        // Exit if we just finished the 1GB test
        if (expected_size >= static_cast<size_t>(1024LL * 1024 * 1024)) break;
    }

    return 0;
}