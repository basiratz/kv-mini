# High-Performance C++ LSM-Tree Key-Value Engine

A lightweight, high-performance Key-Value storage engine built from scratch in C++17 using the **Log-Structured Merge-Tree (LSM-Tree)** architecture. Designed for high write throughput, efficient disk I/O, and optimal search latency.

---

## Architecture Overview

                  +-------------------+
                  |   Client Writes   |
                  +---------+---------+
                            |
               +------------+------------+
               |                         |
               v                         v
    +-------------------+     +-------------------+
    | Write-Ahead Log   |     | MemTable (RAM)    |
    | (WAL on Disk)     |     | (std::map)        |
    +-------------------+     +---------+---------+
                                         |
                                 Flush when full
                                         v
                              +-------------------+
                              | Immutable SSTable |
                              | (Disk File)       |
                              |                   |
                              |  - Bloom Filter   |
                              |  - Sparse Index   |
                              |  - Data Block     |
                              +---------+---------+
                                         |
                              Two-Pointer Merge
                                         v
                              +-------------------+
                              | Compacted SSTable |
                              +-------------------+

---

## Core Features & Concepts Implemented

1. **MemTable (RAM Buffering):** Fast $O(\log N)$ in-memory insertions using `std::map`.
2. **Write-Ahead Log (WAL):** Append-only logging guarantees crash recovery and crash consistency before flushing.
3. **Immutable Sorted String Tables (SSTables):** Flushes ordered data to disk for fast sequential I/O.
4. **Logical Deletions (Tombstones):** Mask deleted records (`__DELETED_TOMBSTONE__`) without immediate expensive disk rewrites.
5. **Sparse Indexing:** Reduces memory footprint by indexing every $K$-th key offset in RAM, enabling fast binary search and bounded block scans on disk.
6. **Bloom Filters:** Space-efficient probabilistic structure ($k$ salted hash functions over a bit array) to skip disk reads for missing keys with **100% false-negative avoidance**.
7. **Compaction Engine:** background-capable Two-Pointer Merge algorithm that consolidates multiple SSTables, overwrites stale keys, purges tombstones, and frees disk space.

---

## Performance Matrix

| Feature | Primary Advantage |
| :--- | :--- |
| **WAL + MemTable** | Constant-time fast writes without blocking on disk seeks |
| **Sparse Index** | Keeps RAM usage low (e.g., 75% footprint reduction with `block_size=4`) |
| **Bloom Filter** | Bypasses up to 99%+ of negative-lookup disk reads |
| **Compaction** | Prevents SSTable accumulation and reclaims wasted disk space |

---

## Getting Started

### Prerequisites
* A C++17 compliant compiler (`g++` or `clang++`)
* C++ Standard Library (`<fstream>`, `<vector>`, `<map>`, `<functional>`, `<optional>`)

### Build & Run

Clone the repository and compile with `g++`:

```bash
git clone https://github.com/basiratz/lsm-kv-engine.git
cd lsm-kv-engine

# Compile the project
g++ -std=c++17 main.cpp -o kv_store

# Run the executable
./kv_store

###** Code Example**
C++


#include "kvstore.hpp"
#include <iostream>

int main() {
    // Initialize storage engine (WAL path, MemTable limit, Block size)
    KVStore db("wal.log", 3, 4);

    // Insert key-value pairs
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red"); // Auto-flushes to sst_000001.sst when MemTable fills

    // Update and Delete
    db.put("apple", "bright_red");
    db.remove("banana"); // Writes Tombstone

    // Retrieve Key (Checks MemTable -> Bloom Filter -> Sparse Index -> Disk Block)
    auto value = db.get("apple");
    if (value) {
        std::cout << "apple: " << *value << "\n"; // Output: bright_red
    }

    // Run Compaction to consolidate SSTables and purge tombstones
    db.compact();

    return 0;
}


### Directory Structure
.
├── bloom_filter.hpp   # Probabilistic filter implementation (bit array + hashing)
├── sstable.hpp        # SSTable representation, sparse index, block lookup, and compaction
├── wal.hpp            # Write-Ahead Log for crash recovery
├── kvstore.hpp        # Core database coordinator and public API
└── main.cpp           # Integration testing and verification suite

### Author
Built as part of an end-to-end exploration into database storage engine design and low-level systems programming in C++.

