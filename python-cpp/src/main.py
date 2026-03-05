#!/usr/bin/env python3
"""
Hybrid Python + C++ implementation for maximum performance.
"""

import sys
import os
import json
from time import perf_counter
import fast_processor

def format_output(data):
    """Format output as sorted JSON (compact)"""
    grouped = {}
    
    paths = sorted(data.keys())
    for path in paths:
        dates = data[path]
        grouped[path] = dict(sorted(dates.items()))
    
    return grouped

def main(input_file):
    file_size = os.path.getsize(input_file)
    
    print(f"Processing file: {input_file}")
    print(f"File size: {file_size // (1024*1024)} MB")
    
    start_time = perf_counter()
    
    num_workers = os.cpu_count()
    
    # Use C++ fast processor
    result = fast_processor.process_file(input_file, num_workers)
    
    output = format_output(result)
    
    # Write compact JSON
    with open('output.json', 'w') as f:
        json.dump(output, f)
    
    end_time = perf_counter()
    duration = end_time - start_time
    
    print(f"Processed in {duration:.3f} seconds")
    print('Output written to: output.json')

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 src/main.py <input_file>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found")
        sys.exit(1)
    
    main(input_file)
