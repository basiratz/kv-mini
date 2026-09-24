#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    // MemTable size = 8, Block size = 4
    KVStore db(wal_file, 8, 4);

    std::cout << "=== Inserting 8 items to fill MemTable ===\n";
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red");
    db.put("date", "brown");
    db.put("elderberry", "purple");
    db.put("fig", "green");
    db.put("grape", "purple");
    db.put("honeydew", "green"); // Triggers flush to sst_000001.sst

    std::cout << "\n=== DB Overview ===\n";
    db.print_all();

    std::cout << "\n=== Testing Sparse Index Lookups ===\n";

    // "cherry" is index 2 in SSTable (between index 0 "apple" and index 4 "elderberry")
    std::cout << "Looking up 'cherry' (Not directly indexed, lies inside block 0):\n";
    auto v1 = db.get("cherry");

    // "elderberry" is index 4 in SSTable (Directly indexed entry)
    std::cout << "\nLooking up 'elderberry' (Directly indexed entry):\n";
    auto v2 = db.get("elderberry");

    // "fig" is index 5 in SSTable (Inside block 1)
    std::cout << "\nLooking up 'fig' (Inside block 1):\n";
    auto v3 = db.get("fig");

    std::cout << "\nLooking up 'watermelon' (Non-existent):\n";
    auto v4 = db.get("watermelon");
    if (!v4) {
        std::cout << "  -> 'watermelon' correctly not found.\n";
    }

    return 0;
}