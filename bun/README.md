# 100-million-row-challenge-bun

Bun implementation of the 100 million row challenge for processing web analytics data.

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

Uses Bun's fast JavaScript runtime with:
- Fast file I/O using Bun's native APIs
- Parallel processing with Worker threads
- Efficient memory usage
- JSON serialization

## Requirements

- Bun runtime (https://bun.sh)

## Usage

### Quick test

```bash
cd bun
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
- Bun's native file I/O
- Worker threads for parallel processing
- Efficient memory usage

Expected performance on modern hardware:
- 100M rows: ~2-5 seconds (depending on CPU cores and disk I/O)
