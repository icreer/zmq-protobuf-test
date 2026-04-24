#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include "payload.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

int main() {
    // 1. Prepare the list of message sizes
    std::vector<size_t> sizes;
    
    // Start at 2 bytes, increment by 64 bytes up to 10MB
    const size_t ten_mb = 10 * 1024 * 1024;
    for (size_t s = 2; s <= ten_mb; s += 64) {
        sizes.push_back(s);
    }

    // Append large specific sizes
    sizes.push_back(50ULL * 1024 * 1024);
    sizes.push_back(100ULL * 1024 * 1024);
    sizes.push_back(1024ULL * 1024 * 1024); // 1 GB
    sizes.push_back(static_cast<size_t>(1.5 * 1024 * 1024 * 1024));
    sizes.push_back(static_cast<size_t>(1.98 * 1024 * 1024 * 1024));

    // 2. Open output file
    std::ofstream outfile("protobuftestresulats.csv");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file 'protobuftestresulats'" << std::endl;
        return 1;
    }
    outfile << "Size_MB,Serialization_Time_ms,Deserialization_Time_ms,Total_Time_ms" << std::endl;

    std::cout << "Starting Protobuf Performance Test..." << std::endl;

    for (size_t size : sizes) {
        // Setup the message with dummy data
        test::Payload msg;
        msg.mutable_data()->assign(size, 'A');

        // --- Measure Serialization ---
        auto start_ser = std::chrono::high_resolution_clock::now();
        
        std::string serialized_data;
        if (!msg.SerializeToString(&serialized_data)) {
            std::cerr << "Serialization failed for size: " << size << std::endl;
            continue;
        }
        
        auto end_ser = std::chrono::high_resolution_clock::now();

        // --- Measure Deserialization ---
        test::Payload decoded_msg;
        auto start_des = std::chrono::high_resolution_clock::now();
        
        // We must use CodedInputStream to increase the total bytes limit for large messages
        google::protobuf::io::ArrayInputStream array_in(serialized_data.data(), serialized_data.size());
        google::protobuf::io::CodedInputStream coded_in(&array_in);
        
        // Set limit to ~2GB (maximum allowed by Protobuf)
        coded_in.SetTotalBytesLimit(2147483647); 
        
        if (!decoded_msg.ParseFromCodedStream(&coded_in)) {
            std::cerr << "Deserialization failed for size: " << size << std::endl;
            continue;
        }
        
        auto end_des = std::chrono::high_resolution_clock::now();

        // 3. Calculate and record results
        std::chrono::duration<double, std::milli> ser_diff = end_ser - start_ser;
        std::chrono::duration<double, std::milli> des_diff = end_des - start_des;
        double total_ms = ser_diff.count() + des_diff.count();
        double size_mb = static_cast<double>(size) / (1024.0 * 1024.0);

        outfile << size_mb << "," 
                << ser_diff.count() << "," 
                << des_diff.count() << "," 
                << total_ms << std::endl;

        // Provide feedback for long-running large tests
        if (size >= 100 * 1024 * 1024) {
            std::cout << "Processed " << (size / (1024.0 * 1024.0)) << " MB message..." << std::endl;
        }
    }

    outfile.close();
    std::cout << "Test complete. Results written to 'protobuftestresulats'" << std::endl;

    return 0;
}