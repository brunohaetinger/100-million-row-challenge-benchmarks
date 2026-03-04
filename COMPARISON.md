# Implementation Comparison: Ruby vs Other Languages

## Challenge Overview

Process 100 million rows of web analytics data (URL + timestamp) and aggregate visit counts by URL path and date.

## Input/Output Format

All implementations use identical formats:

**Input:**
```
https://example.com/blog/post,2024-01-24T01:16:58+00:00
```

**Output:**
```json
{
  "/blog/post": {
    "2024-01-24": 1
  }
}
```

## Architecture Comparison

### Ruby Implementation (this project)
- Ruby 3.1+ Ractors for true parallelism (bypasses GIL)
- Memory-mapped file reading with File::MAP constants
- Chunked processing aligned to newline boundaries
- One Ractor per CPU core processing independently
- Ractor-local hash maps for aggregation
- Merge results at completion
- Yajl for fast JSON serialization

**Performance:** Expected ~2-5 seconds for 100M rows

### Zig Implementation (reference)
- Memory-mapped I/O for zero-copy file reading
- Splits file into chunks aligned to newline boundaries
- One thread per CPU core processing independently
- Thread-local hash maps for aggregation
- Merge results at completion
- 8MB buffered writes for output

**Performance:** 0.765 seconds for 100M rows (~131M rows/second)

### Scala ZIO Implementation
- ZIO Streams for efficient file I/O
- Chunked stream processing (10,000 lines per chunk)
- Parallel fiber processing using `mapZIOPar`
- Fiber-local hash maps for aggregation
- Merge results at completion
- Direct JSON serialization for output

**Performance:** Expected 2-5 seconds for 100M rows

## Key Similarities

1. Parallel processing (Ractors/threads/fibers)
2. Local aggregation per worker
3. Merge strategy at the end
4. Chunked processing for memory efficiency

## Key Differences

1. Ruby uses Ractors (true parallelism, no GIL)
2. JVM vs native execution vs Ruby VM
3. Green threads vs OS threads
4. Different JSON libraries

## Performance Expectations

The Ruby implementation should be competitive with Scala ZIO:
- Ractors provide true parallelism (bypasses GIL)
- Yajl is a C extension for fast JSON
- Similar chunked processing approach

Expected performance ratio: Similar to Scala ZIO (2-5 seconds)

## Optimization Opportunities

To improve performance:
1. Use larger chunk sizes
2. Tune Ractor count based on CPU cores
3. Use frozen strings for less allocation
4. Consider using FFI for custom C extensions
