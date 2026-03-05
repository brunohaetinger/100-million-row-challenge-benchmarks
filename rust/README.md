# 100-million-row-challenge-rust

Rust implementation of the 100 million row challenge for processing web analytics data.

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

Uses Rust with high-performance optimizations:
- Memory-mapped I/O for zero-copy file reading
- Rayon for parallel processing
- HashBrown for fast hashmaps
- Thread-local aggregation with merge
- Serde for JSON serialization

## Requirements

- Rust toolchain (rustc 1.70+)
- Cargo

## Building

```bash
cd rust
cargo build --release
```

## Usage

### Quick test
```bash
./target/release/main test/test_small.txt
```

### Generate test data
```bash
./generate.sh 100000000
```

### Run the challenge
```bash
./target/release/main measurements.txt
```

## Performance

Expected performance on modern hardware:
- 100M rows: **~0.8-2 seconds** (depending on CPU cores and disk I/O)

Rust's zero-cost abstractions + parallel processing make it one of the fastest implementations.
