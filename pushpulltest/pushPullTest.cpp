#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <atomic>

int main() {
    const int port = 5555;
    
    // 1. Prepare the list of message sizes
    std::vector<size_t> sizes;
    const size_t ten_mb = 10 * 1024 * 1024;
    for (size_t s = 2; s <= ten_mb; s += 64) {
        sizes.push_back(s);
    }
    sizes.push_back(50ULL * 1024 * 1024);
    sizes.push_back(100ULL * 1024 * 1024);
    sizes.push_back(500ULL * 1024 * 1024);
    sizes.push_back(1024ULL * 1024 * 1024); // 1 GB
    sizes.push_back(static_cast<size_t>(1.5 * 1024 * 1024 * 1024));
    sizes.push_back(static_cast<size_t>(1.98 * 1024 * 1024 * 1024));

    // 2. Open output file
    std::ofstream outfile("pushpullresults.csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    outfile << "Port,Size_MB,Latency_ms" << std::endl;

    zmq::context_t context(1);
    
    // Pre-allocate the largest buffer once to avoid repeated allocations
    std::vector<char> buffer(sizes.back(), 'A');

    std::cout << "Starting Push-Pull Performance Test on port " << port << "..." << std::endl;

    std::string addr = "tcp://127.0.0.1:" + std::to_string(port);
    
    // Setup Puller (Receiver)
    zmq::socket_t puller(context, ZMQ_PULL);
    int hwm = 1;    // Keep HWM low for massive messages to manage memory usage
    int linger = 0; // Close immediately and release the port
    puller.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
    puller.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));

    try {
        puller.bind(addr);
    } catch (const zmq::error_t& e) {
        std::cerr << "Failed to bind to " << addr << ": " << e.what() << std::endl;
        std::cerr << "Try running: fuser -k " << port << "/tcp to clear the port." << std::endl;
        return 1;
    }

    // Setup Pusher (Sender)
    zmq::socket_t pusher(context, ZMQ_PUSH);
    pusher.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    pusher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    pusher.connect(addr);

    // Give ZMQ a moment to perform the handshake and stabilize the TCP connection
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    for (size_t size : sizes) {
        std::atomic<bool> received{false};
        std::chrono::high_resolution_clock::time_point end_time;

        // Start receiver thread for this specific message
        std::thread receiver([&]() {
            zmq::message_t msg;
            auto res = puller.recv(msg, zmq::recv_flags::none);
            end_time = std::chrono::high_resolution_clock::now();
            received = true;
        });

        // Measure end-to-end latency: Send start -> Receive end
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Zero-copy message creation using the pre-allocated buffer
        zmq::message_t task(buffer.data(), size, nullptr);
        pusher.send(task, zmq::send_flags::none);

        // Wait for receiver to finish processing the message
        receiver.join();

        if (received) {
            std::chrono::duration<double, std::milli> diff = end_time - start_time;
            double size_mb = static_cast<double>(size) / (1024.0 * 1024.0);
            
            outfile << port << "," << size_mb << "," << diff.count() << "\n";
        }

        // Console feedback for progress tracking on larger payloads
        if (size >= 100 * 1024 * 1024) {
            std::cout << "Processed " << (size / (1024.0 * 1024.0)) << " MB message..." << std::endl;
        }
    }

    pusher.close();
    puller.close();

    outfile.close();
    std::cout << "Test complete. Results written to 'pushpullresults.csv'" << std::endl;

    return 0;
}