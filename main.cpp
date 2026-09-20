#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    // Small MemTable size (3) to trigger flushes quickly for testing
    KVStore db(wal_file, 3);

    std::cout << "=== Populating Storage Engine ===\n";
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red"); // Flush 1 (sst_000001.sst)

    db.put("date", "brown");
    db.put("elderberry", "purple");
    db.put("fig", "green");      // Flush 2 (sst_000002.sst)

    db.put("grape", "purple");
    db.put("honeydew", "green");
    db.put("kiwi", "brown");     // Flush 3 (sst_000003.sst)

    db.put("lemon", "yellow");   // Stored in MemTable

    std::cout << "\n=== Testing Binary Search Lookups ===\n";

    std::cout << "1. Looking up 'apple' (In sst_000001.sst):\n";
    auto v1 = db.get("apple");

    std::cout << "\n2. Looking up 'elderberry' (In sst_000002.sst):\n";
    auto v2 = db.get("elderberry");

    std::cout << "\n3. Looking up 'lemon' (In MemTable):\n";
    auto v3 = db.get("lemon");

    std::cout << "\n4. Removing 'apple' (Inserts Tombstone):\n";
    if (db.remove("apple")) {
        std::cout << "  -> Successfully marked 'apple' for deletion.\n";
    }

    std::cout << "\n5. Looking up 'apple' after deletion:\n";
    auto v4 = db.get("apple");
    if (!v4) {
        std::cout << "  -> Verified 'apple' returns missing/deleted!\n";
    }

    return 0;
}