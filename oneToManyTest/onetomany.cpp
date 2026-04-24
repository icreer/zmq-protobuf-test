#include <zmq.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>

std::vector<size_t> build_sizes() 
{
    std::vector<size_t> sizes;

    for (size_t s = 2; s <= 10 * 1024 * 1024; s += 64) 
    {
        sizes.push_back(s);
    }

    sizes.push_back(50ULL * 1024 * 1024);
    sizes.push_back(100ULL * 1024 * 1024);
    sizes.push_back(500ULL * 1024 * 1024);
    sizes.push_back(1ULL * 1024 * 1024 * 1024);
    sizes.push_back(static_cast<size_t>(1.5 * 1024 * 1024 * 1024));
    sizes.push_back(static_cast<size_t>(1.98 * 1024 * 1024 * 1024));
    sizes.push_back(1980ULL * 1024 * 1024); // ~1.98GB

    return sizes;
}

void run_router(const std::string& endpoint)
{
    zmq::context_t ctx(4);
    zmq::socket_t router(ctx, zmq::socket_type::router);

    router.bind(endpoint);

    while (true)
    {
        std::cerr << "Router Running" << std::endl;
        zmq::message_t identity;
        zmq::message_t empty;
        zmq::message_t payload;

        router.recv(identity);
        //router.recv(empty);
        router.recv(payload);

        // Echo back
        router.send(identity, zmq::send_flags::sndmore);
        //router.send(zmq::message_t(), zmq::send_flags::sndmore);
        router.send(payload, zmq::send_flags::none);
        
    }
}

void dealer_worker(const std::string& endpoint,const std::vector<size_t>& sizes,int id)
{
    zmq::context_t ctx(4);
    zmq::socket_t dealer(ctx, zmq::socket_type::dealer);

    dealer.set(zmq::sockopt::routing_id, "dealer-" + std::to_string(id));
    dealer.connect(endpoint);

    std::ofstream outfile("onetoManyID" +std::to_string(id)+".csv");

    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        exit;
    }
    outfile << "Dealer ID, Size_MB ,Latency_ms" << std::endl;

    dealer.set(zmq::sockopt::rcvtimeo, 1000); // Time out at 5 sceond this is the time ourt that is working
    // dealer.set(zmq::sockopt::sndtimeo, 1000);

    for (auto size : sizes) 
    {
        std::vector<char> buffer(size, 'x');

        zmq::message_t msg(buffer.data(), buffer.size());

        auto start = std::chrono::high_resolution_clock::now();

        dealer.send(zmq::message_t(), zmq::send_flags::sndmore); // empty frame
        dealer.send(msg, zmq::send_flags::none);

        zmq::message_t reply;
        zmq::recv_result_t res = dealer.recv(reply);

        auto end = std::chrono::high_resolution_clock::now();

        
        if (!res)
        {
            std::cerr << "Timeout on dealer " << id << "\n";
            continue;
        }

        std::chrono::duration<double, std::milli> us = end - start;

        outfile << id << ","<< (size/(1024.0 * 1024.0)) << "," << us.count() << " \n";
    }
    outfile.close();
}

int main(int argc, char** argv) 
{
    std::string endpoint = "tcp://127.0.0.1:5555";
    int num_dealers = 1;

    if (argc > 1)
    {
        num_dealers = std::stoi(argv[1]);
    }

    auto sizes = build_sizes();

    // Start router
    std::thread router_thread(run_router, endpoint);

    // Give router time to bind
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start dealers
    std::vector<std::thread> dealers;
    for (int i = 0; i < num_dealers; ++i)
    {
        dealers.emplace_back(dealer_worker, endpoint, sizes, i);
    }

    for (auto& t : dealers)
    {
        t.join();
    }

    return 0;
}