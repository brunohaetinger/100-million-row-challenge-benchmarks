#!/usr/bin/env python3
"""
High-performance Python implementation using multiprocessing and memory mapping.
"""

import sys
import os
import mmap
import json
import multiprocessing as mp
from collections import defaultdict
from time import perf_counter
import re

DATE_LEN = 10
CHUNK_SIZE = 64 * 1024 * 1024

def find_path_and_date(line):
    """Extract path and date from line"""
    comma_idx = line.find(',')
    if comma_idx == -1:
        return None, None
    
    url = line[:comma_idx]
    timestamp = line[comma_idx + 1:comma_idx + 1 + DATE_LEN]
    
    path_start = url.find('/', 8)
    if path_start == -1:
        return None, None
    
    path = url[path_start:]
    
    return path, timestamp

def process_chunk(args):
    """Process a chunk of the file"""
    file_path, start_pos, end_pos = args
    result = defaultdict(lambda: defaultdict(int))
    
    with open(file_path, 'rb') as f:
        f.seek(start_pos)
        data = f.read(end_pos - start_pos)
        
        pos = 0
        while pos < len(data):
            newline_pos = data.find(b'\n', pos)
            if newline_pos == -1:
                break
            
            line_bytes = data[pos:newline_pos]
            line = line_bytes.decode('utf-8', errors='ignore')
            
            path, timestamp = find_path_and_date(line)
            if path and timestamp:
                result[path][timestamp] += 1
            
            pos = newline_pos + 1
    
    return dict(result)

def format_output(data):
    """Format output as sorted JSON"""
    grouped = {}
    
    for path in sorted(data.keys()):
        dates = data[path]
        grouped[path] = dict(sorted(dates.items()))
    
    return grouped

def main(input_file):
    file_size = os.path.getsize(input_file)
    
    print(f"Processing file: {input_file}")
    print(f"File size: {file_size // (1024*1024)} MB")
    
    start_time = perf_counter()
    
    num_workers = min(mp.cpu_count(), max(1, file_size // CHUNK_SIZE))
    chunk_size = file_size // num_workers
    
    chunks = []
    for i in range(num_workers):
        start_pos = i * chunk_size
        end_pos = (i + 1) * chunk_size if i < num_workers - 1 else file_size
        
        # Adjust to line boundaries
        if i > 0:
            with open(input_file, 'rb') as f:
                f.seek(start_pos)
                line = f.readline()
                start_pos = f.tell()
        
        chunks.append((input_file, start_pos, end_pos))
    
    with mp.Pool(num_workers) as pool:
        results = pool.map(process_chunk, chunks)
    
    # Merge results
    final_result = defaultdict(lambda: defaultdict(int))
    for res in results:
        for path, dates in res.items():
            for date, count in dates.items():
                final_result[path][date] += count
    
    output = format_output(final_result)
    
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
