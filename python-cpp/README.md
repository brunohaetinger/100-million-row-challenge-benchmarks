# 100-million-row-challenge-python-cpp

Hybrid Python + C++ implementation. Python handles orchestration and JSON, C++ handles high-performance I/O and parsing.

## Architecture

- **Python**: Orchestration, multiprocessing, JSON serialization, file coordination
- **C++**: Memory-mapped I/O, ultra-fast line parsing, aggregation
- **Communication**: Shared memory via `multiprocessing.shared_memory`

## Why Hybrid?

- C++ for zero-copy mmap + parsing (~10x faster than Python)
- Python for easy multiprocessing + JSON handling
- Total: Best of both worlds

## Requirements

- Python 3.8+
- C++17 compiler (g++)
- `setuptools` for compilation

## Building

```bash
cd python-cpp
python3 setup.py build_ext --inplace
```

## Usage

```bash
./run.sh measurements.txt
```

## Performance Target

Expected: **~2-5 seconds** for 100M rows (combining C++ speed with Python simplicity)
