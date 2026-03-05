# 100-million-row-challenge-python

Python implementation of the 100 million row challenge for processing web analytics data.

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

Uses Python with high-performance optimizations:
- Memory-mapped I/O (mmap) for efficient file reading
- Multiprocessing for parallel processing
- Process-local dictionaries for aggregation
- Merge results at completion
- Fast JSON serialization

## Requirements

- Python 3.8+
- No external dependencies (uses standard library)

## Usage

### Quick test
```bash
cd python
python src/main.py test/test_small.txt
```

### Generate test data
```bash
./generate.sh 100000000
```

### Run the challenge
```bash
./run.sh measurements.txt
```

Or directly:
```bash
python src/main.py measurements.txt
```

## Performance

Expected performance on modern hardware:
- 100M rows: ~5-15 seconds (depending on CPU cores and disk I/O)

Uses multiprocessing to maximize CPU utilization.
