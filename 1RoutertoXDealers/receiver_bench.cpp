#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <thread>

int main() {
    zmq::context_t context(1); // 1 I/O thread
    zmq::socket_t dealer_socket(context, ZMQ_DEALER);
    
    std::ofstream outfile("benchmark_results.csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    outfile << "Message_Size_Bytes,Round_Trip_Time_Seconds,Throughput_MBs" << std::endl;

    // Increase the high-water mark to handle large messages
    int hwm = 100; 
    dealer_socket.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    dealer_socket.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
    
    std::cout << "DEALER: Connecting to ROUTER at tcp://localhost:5555..." << std::endl;
    dealer_socket.connect("tcp://localhost:5555");

    size_t start_size = 2;
    size_t end_size = static_cast<size_t>(1024LL * 1024 * 1024); // 1GB
    int num_steps = 10000;
    std::vector<size_t> sizes;
    double step = static_cast<double>(end_size - start_size) / (num_steps - 1);
    for (int i = 0; i < num_steps; ++i) {
        sizes.push_back(start_size + static_cast<size_t>(i * step));
    }
    
    // Pre-allocate buffer to avoid thrashing memory during the benchmark
    std::vector<char> data(end_size, 'A');

    // Give the connection a moment to stabilize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (size_t size : sizes) {
        zmq::message_t request_payload(data.data(), size);

        auto start = std::chrono::high_resolution_clock::now();
        
        // DEALER must send an empty delimiter frame when talking to a ROUTER 
        // that expects REQ-style envelopes.
        dealer_socket.send(zmq::message_t(0), zmq::send_flags::sndmore);
        dealer_socket.send(request_payload, zmq::send_flags::none);

        // Receive the reply: First the empty delimiter, then the payload
        zmq::message_t delimiter;
        zmq::message_t payload;
        
        auto res_delimiter = dealer_socket.recv(delimiter, zmq::recv_flags::none);
        if (!res_delimiter || delimiter.size() != 0) {
            std::cerr << "DEALER: Protocol error, expected empty delimiter." << std::endl;
            break;
        }
        auto res_payload = dealer_socket.recv(payload, zmq::recv_flags::none);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        if (res_payload) {
            double MBs = (static_cast<double>(size) / (1024.0 * 1024.0)) / diff.count();
            // std::cout << "DEALER: Sent/Received " << size << " bytes. RTT: " << diff.count() << "s, Throughput: " << MBs << " MB/s" << std::endl;
            outfile << size << "," << diff.count() << "," << MBs << std::endl;
        } else {
            std::cerr << "DEALER: Error receiving reply or context terminated." << std::endl;
            break;
        }
    }

    return 0;
}