# 100 Million Row Challenge - Multi-Language Benchmark

This project benchmarks different language implementations for processing 100 million rows of web analytics data.

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

## Benchmark Results (100M rows)

| Rank | Implementation | Directory | Time |
|------|----------------|-----------|------|
| 🥇 | Rust | `rust/` | **0.508s** |
| 🥈 | C++ | `cpp/` | **0.751s** |
| 🥉 | Python-C++ Hybrid | `python-cpp/` | **1.1s** |
| 4 | Bun | `bun/` | **18s** | 
| 5 | Python | `python/` | **11s** | 
| 6 | Ruby (fork) | `ruby/` | **13s** | 
| 7 | Ruby (single) | `ruby/` | **84s** | 

## Shared Data

- `measurements.txt` - Generated test data (100M rows, ~5.6GB)
- `test_small.txt` - Small test file for validation (in each directory)

## Quick Start

### Python-C++ Hybrid (Fastest)
```bash
cd python-cpp
./run.sh test/test_small.txt    # Test
./run.sh measurements.txt       # 100M rows (~1.1s)
```

### C++
```bash
cd cpp
./run.sh test/test_small.txt    # Test
./run.sh measurements.txt       # 100M rows (~1s)
```

### Python
```bash
cd python
./run.sh test/test_small.txt    # Test
./run.sh measurements.txt       # 100M rows (~11s)
```

### Bun
```bash
cd bun
./run.sh test/test_small.txt    # Test
./run.sh measurements.txt       # 100M rows (~18s)
```

### Ruby
```bash
cd ruby
./run.sh test_small.txt         # Test
./run.sh measurements.txt       # 100M rows (~13s)
```

## Generate Test Data

```bash
cd ruby && ./generate.sh 100000000  # Generate 100M rows (~5.6GB)
```

## Key Insights

1. **Python-C++ Hybrid** wins - C++ parsing speed + Python simplicity
2. **Pure C++** is fastest native implementation
3. **Bun** surprisingly competitive for JS runtime
4. **Python multiprocessing** solid (~11s)
5. **Ruby fork** competitive with Python
6. Zig reference remains the gold standard (~0.77s)

## Individual Comparison Files

Each directory contains `COMPARISON.md` with detailed analysis.

**Ready to benchmark!** 🚀
