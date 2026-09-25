#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    // MemTable size = 3 (triggers flushes often to build multiple SSTables)
    KVStore db(wal_file, 3, 4);

    std::cout << "=== Inserting Batch 1 (Flushes to sst_000001.sst) ===\n";
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red");

    std::cout << "\n=== Inserting Batch 2 (Flushes to sst_000002.sst) ===\n";
    db.put("date", "brown");
    db.put("elderberry", "purple");
    db.put("fig", "green");

    std::cout << "\n=== Inserting Batch 3 (MemTable) ===\n";
    db.put("grape", "purple");

    std::cout << "\n=== Testing Bloom Filter Performance ===\n";

    std::cout << "\n1. Looking up 'apple' (Exists in sst_000001.sst):\n";
    auto v1 = db.get("apple");

    std::cout << "\n2. Looking up 'mango' (Non-existent everywhere):\n";
    auto v2 = db.get("mango");
    if (!v2) {
        std::cout << "  -> 'mango' correctly not found.\n";
    }

    return 0;
}