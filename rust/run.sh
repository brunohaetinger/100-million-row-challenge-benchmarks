#!/bin/bash

set -e

if [ ! -f "target/release/main" ]; then
    echo "Building Rust..."
    cargo build --release
fi

INPUT_FILE="${1:-measurements.txt}"

if [ ! -f "$INPUT_FILE" ]; then
    if [ -f "test/$INPUT_FILE" ]; then
        INPUT_FILE="test/$INPUT_FILE"
    elif [ -f "../measurements.txt" ]; then
        INPUT_FILE="../measurements.txt"
    fi
fi

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file $INPUT_FILE not found"
    echo "Usage: ./run.sh [input_file]"
    exit 1
fi

echo "Running challenge..."
./target/release/row_challenge "$INPUT_FILE"
