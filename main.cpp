#include "kvstore.hpp"
#include <iostream>

int main() {
    const std::string wal_file = "wal.log";

    std::cout << "=== RUN 1: Writing data & crashing/exiting ===\n";
    {
        KVStore db(wal_file);
        db.put("name", "Ali");
        db.put("city", "Karachi");
        db.put("role", "Engineer");

        std::cout << "State before deletion:\n";
        db.print_all();

        db.remove("city");

        std::cout << "State before shutdown:\n";
        db.print_all();
    } // `db` destructor runs here, object destroyed

    std::cout << "\n=== RUN 2: Startup & Recovering from WAL ===\n";
    {
        // Fresh KVStore instance reading the existing wal.log file
        KVStore db2(wal_file);

        std::cout << "Recovered State:\n";
        db2.print_all();

        auto name = db2.get("name");
        if (name) {
            std::cout << "Successfully retrieved recovered 'name': " << *name << "\n";
        }

        auto city = db2.get("city");
        if (!city) {
            std::cout << "Verified 'city' remains deleted after recovery!\n";
        }
    }

    return 0;
}