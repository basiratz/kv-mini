#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    // MemTable size = 3
    KVStore db(wal_file, 3, 4);

    std::cout << "=== Populating SSTable 1 ===\n";
    db.put("apple", "red");
    db.put("banana", "yellow");
    db.put("cherry", "dark_red"); // Flushes to sst_000001.sst

    std::cout << "\n=== Updating 'apple' & Deleting 'banana' in SSTable 2 ===\n";
    db.put("apple", "bright_red"); // Update apple
    db.remove("banana");           // Delete banana (writes TOMBSTONE)
    db.put("date", "brown");       // Flushes to sst_000002.sst

    db.print_all();

    std::cout << "\n=== Executing Compaction ===\n";
    db.compact(); // Merges sst_000001.sst and sst_000002.sst into sst_000003.sst

    db.print_all();

    std::cout << "\n=== Verifying Lookups After Compaction ===\n";

    std::cout << "\n1. Looking up 'apple' (Should return updated value 'bright_red'):\n";
    auto v1 = db.get("apple");
    if (v1) std::cout << "  -> Value: " << *v1 << "\n";

    std::cout << "\n2. Looking up 'banana' (Tombstone was purged, should return nullopt):\n";
    auto v2 = db.get("banana");
    if (!v2) std::cout << "  -> 'banana' correctly purged!\n";

    return 0;
}