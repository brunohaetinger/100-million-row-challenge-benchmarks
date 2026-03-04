# 100-million-row-challenge-ruby

Ruby implementation of the 100 million row challenge for processing web analytics data.

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

Uses Ruby 3.1+ Ractors for true parallelism:
- Ractors (Ruby Actors) for parallel processing without GIL
- Chunked file reading for memory efficiency
- Local aggregation per Ractor with final merge
- Yajl for fast JSON serialization
- Sorted JSON output

## Requirements

- Ruby 3.1 or higher (for Ractor support)
- Bundler gem

## Installation

```bash
bundle install
```

## Usage

### Quick test

```bash
make test
```

This runs the implementation with a small test file to verify correctness.

### Generate test data

```bash
make generate
```

This generates 100 million rows of test data in `measurements.txt`.

Alternatively, specify a custom number of rows:

```bash
./generate.sh 10000000
```

### Run the challenge

```bash
make run
```

Or directly:

```bash
./run.sh measurements.txt
```

### Clean

```bash
make clean
```

## Performance

Results will vary based on hardware. The implementation is optimized for:
- True parallelism via Ractors (bypasses GIL)
- Efficient memory usage with chunked processing
- Fast JSON serialization with Yajl
- Minimal object allocations during aggregation

Expected performance on modern hardware:
- 100M rows: ~2-5 seconds (depending on CPU cores and disk I/O)

Compare with other implementations to evaluate Ruby performance.
