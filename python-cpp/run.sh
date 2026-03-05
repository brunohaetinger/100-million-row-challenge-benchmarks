#!/bin/bash

set -e

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

if [ ! -f "src/fast_processor.cpython-*.so" ]; then
    echo "Building C++ extension..."
    python3 setup.py build_ext --inplace
fi

echo "Running challenge..."
PYTHONPATH=. python3 src/main.py "$INPUT_FILE"
