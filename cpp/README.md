# 100-million-row-challenge-cpp

C++ implementation of the 100 million row challenge for processing web analytics data.

## Challenge

Process 100 million rows of web analytics data (URL + timestamp) and aggregate visit counts by URL path and date.

**Input Format:**
```
https://example.com/blog/scala-zio,2024-01-24T01:16:58+00:00
```

**Output Format:**
```json
{
  "/blog/scala-zio": {
    "2024-01-24": 1
  }
}
```

## Implementation

Uses C++ with high-performance optimizations:
- Memory-mapped I/O (mmap) for zero-copy file reading
- Multi-threaded processing with std::thread
- Thread-local hash maps for local aggregation
- Merge results at completion
- Fast JSON serialization

## Requirements

- C++17 compatible compiler (g++ or clang++)
- CMake (optional, can compile directly)

## Building

```bash
mkdir -p build && cd build
cmake ..
make
```

Or directly:
```bash
g++ -O3 -std=c++17 -pthread -o main main.cpp
```

## Usage

### Quick test
```bash
./build/main test/test_small.txt
```

### Generate test data
```bash
./generate.sh 100000000
```

### Run the challenge
```bash
./build/main measurements.txt
```

## Performance

Expected performance on modern hardware:
- 100M rows: ~1-3 seconds (depending on CPU cores and disk I/O)

Uses parallel processing to maximize CPU utilization.
