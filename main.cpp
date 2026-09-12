#include <iostream>
#include "kvstore.hpp"

int main() {
    KVStore db;

    std::cout << "1. Inserting entries...\n";
    db.put("name", "Ali");
    db.put("city", "Karachi");
    db.put("role", "Engineer");
    db.print_all();

    std::cout << "\n2. Testing get()...\n";
    auto name = db.get("name");
    if (name) {
        std::cout << "Found name: " << *name << "\n";
    } else {
        std::cout << "Name not found!\n";
    }

    auto country = db.get("country");
    if (!country) {
        std::cout << "Country successfully reported as missing.\n";
    }

    std::cout << "\n3. Testing remove()...\n";
    if (db.remove("city")) {
        std::cout << "Successfully removed 'city'.\n";
    }
    db.print_all();

    return 0;
}