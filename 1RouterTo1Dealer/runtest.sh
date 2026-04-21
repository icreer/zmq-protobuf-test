#!/bin/bash

echo "Starting Test"

./build/bin/sender_bench  & ./build/bin/receiver_bench

echo "Finished Test