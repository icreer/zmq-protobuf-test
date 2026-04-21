#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

int main(int argc, char** argv) {
    int num_routers = 1;
    if (argc > 1) {
        num_routers = std::stoi(argv[1]);
    }

    // Use more I/O threads to handle multiple parallel 1GB streams
    zmq::context_t context(4); 
    std::vector<zmq::socket_t> sockets;
    std::vector<zmq::pollitem_t> poll_items;

    int start_port = 5555;
    int hwm = 10; // Low HWM is critical when dealing with 1GB messages per socket

    for (int i = 0; i < num_routers; ++i) {
        sockets.emplace_back(context, ZMQ_ROUTER);
        auto& sock = sockets.back();
        
        sock.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
        sock.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));

        std::string addr = "tcp://*:" + std::to_string(start_port + i);
        sock.bind(addr);
        
        poll_items.push_back({ static_cast<void*>(sock), 0, ZMQ_POLLIN, 0 });
    }

    std::cout << "ROUTER: Opened " << num_routers << " sockets starting at port " << start_port << std::endl;

    while (true) {
        // Poll with no timeout to wait for any dealer to send data
        zmq::poll(poll_items.data(), poll_items.size(), -1);

        for (size_t i = 0; i < poll_items.size(); ++i) {
            if (poll_items[i].revents & ZMQ_POLLIN) {
                zmq::message_t identity, empty_frame, payload;
                auto& sock = sockets[i];

                // Receive envelope: [Identity][Empty][Payload]
                if (!sock.recv(identity)) break;
                if (!sock.recv(empty_frame) || empty_frame.size() != 0) break;
                if (!sock.recv(payload)) break;

                // Echo back
                sock.send(identity, zmq::send_flags::sndmore);
                sock.send(empty_frame, zmq::send_flags::sndmore);
                sock.send(payload, zmq::send_flags::none);
            }
        }
    }

    return 0;
}