#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

int main() {
    zmq::context_t context(5); // 1 I/O thread
    zmq::socket_t router_socket(context, ZMQ_ROUTER);
    
    // Increase the high-water mark to handle large messages
    // NOTE: 10,000 is too high for 1GB messages (would use 10TB of RAM).
    // We lower this to prevent OOM crashes.
    int hwm = 100; 
    router_socket.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    router_socket.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));

    std::cout << "ROUTER: Binding to tcp://*:5555..." << std::endl;
    router_socket.bind("tcp://*:5555");
    
    std::cout << "ROUTER: Waiting for messages from DEALERs..." << std::endl;

    // The router will run indefinitely, echoing messages.
    // The benchmark termination will be handled by the receiver.
    while (true) {
        zmq::message_t identity;
        zmq::message_t empty_frame; // ROUTER receives an empty frame after identity
        zmq::message_t payload;

        // Receive identity frame
        auto res_identity = router_socket.recv(identity, zmq::recv_flags::none);
        if (!res_identity) {
            // If recv returns 0, it means the socket was closed or context terminated.
            // For a router, this might indicate no more clients or an external shutdown.
            // For this benchmark, we'll let it run until manually stopped or an error occurs.
            std::cerr << "ROUTER: Error receiving identity or context terminated." << std::endl;
            break;
        }

        // Receive empty frame
        auto res_empty = router_socket.recv(empty_frame, zmq::recv_flags::none);
        if (!res_empty || empty_frame.size() != 0) {
            std::cerr << "ROUTER: Expected empty frame, but got something else or error." << std::endl;
            break;
        }

        // Receive payload
        auto res_payload = router_socket.recv(payload, zmq::recv_flags::none);
        if (!res_payload) {
            std::cerr << "ROUTER: Error receiving payload." << std::endl;
            break;
        }

        // Echo back: identity, empty frame, payload
        router_socket.send(identity, zmq::send_flags::sndmore);
        router_socket.send(empty_frame, zmq::send_flags::sndmore);
        router_socket.send(payload, zmq::send_flags::none);
       // std::cout << "ROUTER: Echoed " << payload.size() << " bytes." << std::endl;
    }

    return 0;
}