#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <thread>

int main(int argc, char** argv) {
    int id = 0;
    if (argc > 1)
    {
        id = std::stoi(argv[1]);
    }

    zmq::context_t context(5); // 1 I/O thread
    zmq::socket_t dealer_socket(context, ZMQ_DEALER);
    
    std::ofstream outfile("benchmark_results" + std::to_string(id)+".csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    outfile << "Message_Size_Bytes,Round_Trip_Time_ms,Throughput_MBs" << std::endl;

    // Increase the high-water mark to handle large messages
    int hwm = 100; 
    dealer_socket.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    dealer_socket.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
    
    std::cout << "DEALER: Connecting to ROUTER at tcp://localhost:5555..." << std::endl;
    dealer_socket.connect("tcp://localhost:5555");

    std::vector<size_t> sizes;
    // Start at 2 bytes, increment by 64 bytes up to 5MB
    const size_t five_mb = 5 * 1024 * 1024;
    for (size_t s = 2; s <= five_mb; s += 64) {
        sizes.push_back(s);
    }

    // Append large specific sizes
    sizes.push_back(10 * 1024 * 1024);
    sizes.push_back(50ULL * 1024 * 1024);
    sizes.push_back(100ULL * 1024 * 1024);
    sizes.push_back(1024ULL * 1024 * 1024); // 1 GB
    sizes.push_back(static_cast<size_t>(1.5 * 1024 * 1024 * 1024));
    sizes.push_back(static_cast<size_t>(1.98 * 1024 * 1024 * 1024));
    
    // Pre-allocate buffer to avoid thrashing memory during the benchmark
    std::vector<char> data(sizes.size(), 'A');

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
            outfile << (static_cast<double>(size) / (1024.0 * 1024.0)) << "," << diff.count() * 1000  << "," << MBs << std::endl;
        } else {
            std::cerr << "DEALER: Error receiving reply or context terminated." << std::endl;
            break;
        }
    }

    return 0;
}