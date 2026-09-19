#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    // Set max_memtable_size = 3 so flush triggers every 3 entries
    KVStore db(wal_file, 3);

    std::cout << "=== Inserting 7 Key-Value Pairs ===\n";
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red"); // Triggers 1st flush (3 keys -> sst_000001.sst)

    db.put("date", "brown");
    db.put("elderberry", "purple");
    db.put("fig", "green");      // Triggers 2nd flush (3 keys -> sst_000002.sst)

    db.put("grape", "purple");   // Remains in MemTable (1 key)

    std::cout << "\n=== Current Database Overview ===\n";
    db.print_all();

    std::cout << "\n=== Testing Multi-Tier Lookups ===\n";

    std::cout << "Looking up 'grape':\n";
    auto v1 = db.get("grape");

    std::cout << "Looking up 'fig':\n";
    auto v2 = db.get("fig");

    std::cout << "Looking up 'apple':\n";
    auto v3 = db.get("apple");

    std::cout << "Looking up 'mango' (Non-existent):\n";
    auto v4 = db.get("mango");
    if (!v4) {
        std::cout << "  -> 'mango' not found anywhere.\n";
    }

    return 0;
}