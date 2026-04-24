#!/bin/bash

echo "Starting Test"

./build/bin/sender_bench  

# 1 Receiver 
./build/bin/receiver_bench 1

# 5 Receiver
./build/bin/receiver_bench 51 & ./build/bin/receiver_bench 52 & ./build/bin/receiver_bench 53 & ./build/bin/receiver_bench 54 & ./build/bin/receiver_bench 55

# 15 Receivers
./build/bin/receiver_bench 151 & ./build/bin/receiver_bench 152 & ./build/bin/receiver_bench 153 & ./build/bin/receiver_bench 154 & ./build/bin/receiver_bench 155 & ./build/bin/receiver_bench 156 & ./build/bin/receiver_bench 157 & ./build/bin/receiver_bench 158 & ./build/bin/receiver_bench 159 & ./build/bin/receiver_bench 1510 & ./build/bin/receiver_bench 1511 & ./build/bin/receiver_bench 1512 & ./build/bin/receiver_bench 1513 & ./build/bin/receiver_bench 1514 & ./build/bin/receiver_bench 1515


# 30 Receivers
./build/bin/receiver_bench 301 & ./build/bin/receiver_bench 302 & ./build/bin/receiver_bench 303 & ./build/bin/receiver_bench 304 & ./build/bin/receiver_bench 305 & ./build/bin/receiver_bench 306 & ./build/bin/receiver_bench 307 & ./build/bin/receiver_bench 308 & ./build/bin/receiver_bench 309 & ./build/bin/receiver_bench 3010 & ./build/bin/receiver_bench 3011 & ./build/bin/receiver_bench 3012 & ./build/bin/receiver_bench 3013 & ./build/bin/receiver_bench 3014 & ./build/bin/receiver_bench 3015 & ./build/bin/receiver_bench 3016 & ./build/bin/receiver_bench 3017 & ./build/bin/receiver_bench 3018 & ./build/bin/receiver_bench 3019 & ./build/bin/receiver_bench 3020 & ./build/bin/receiver_bench 3021 & ./build/bin/receiver_bench 3022 & ./build/bin/receiver_bench 3023 & ./build/bin/receiver_bench 3024 & ./build/bin/receiver_bench 3025 &  ./build/bin/receiver_bench 3026 & ./build/bin/receiver_bench 3027 & ./build/bin/receiver_bench 3028 & ./build/bin/receiver_bench 3029 & ./build/bin/receiver_bench 3030             



echo "Finished Test"