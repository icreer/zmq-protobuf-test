#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_DEALER);
    
    std::ofstream outfile("sender_bench_results.csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open sender output file" << std::endl;
        return 1;
    }
    outfile << "Size_Bytes,Time_Seconds" << std::endl;

    // Increase the high-water mark to handle large messages
    int hwm = 10;
    socket.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    
    //std::cout << "Connecting to receiver..." << std::endl;
    socket.connect("tcp://localhost:5555");

    size_t start_size = 2;
    size_t end_size = static_cast<size_t>(1024LL * 1024 * 1024);
    int num_steps = 1000;
    std::vector<size_t> sizes;
    double step = static_cast<double>(end_size - start_size) / (num_steps - 1);
    for (int i = 0; i < num_steps; ++i) {
        sizes.push_back(start_size + static_cast<size_t>(i * step));
    }

    for (size_t size : sizes) {
       // std::cout << "Preparing to send " << size << " bytes..." << std::endl;
        
        // Initialize buffer with dummy data
        std::vector<char> data(size, 'A');
        
        // Small delay to ensure receiver is ready for the next burst
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // First, send the size so the receiver knows what to expect
        zmq::message_t size_msg(&size, sizeof(size_t));
        socket.send(size_msg, zmq::send_flags::sndmore);

        // Send the actual payload
        auto start = std::chrono::high_resolution_clock::now();
        
        zmq::message_t payload(data.data(), size);
        socket.send(payload, zmq::send_flags::none);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        
        // std::cout << "Sent " << size << " bytes in " << diff.count() << "s " 
        //           << "(Local queueing time)" << std::endl;
        outfile << size << "," << diff.count() << std::endl;
    }

    return 0;
}