#ifndef KVSTORE_HPP
#define KVSTORE_HPP
#include <iostream>
#include <string>
#include <map>
#include <optional>


class KVStore{
private:
    std::map<std::string, std::string> data_;
public:
    //Insert or update a key-value pair
    void put(const std::string& key, const std::string& value){
        data_[key] = value;
    }

    // Retrieve a value by key
    std::optional<std::string> get(const std::string& key) const {
        auto it = data_.find(key);
        if(it != data_.end()){
            return it->second;
        }
        return std::nullopt;
    }

    // Removing a key
    bool remove(const std::string& key){
        return data_.erase(key)>0;
    }

    //helper to print current storage state for testing
    void print_all() const {
        std::cout << "--- Current DB State (" << data_.size() << " keys) ---\n";    
        for(const auto& [key, value]: data_){
            std::cout << "  " << key << " => " << value << "\n";
        }
        std::cout << "------------------------------------\n";
    }

};


#endif //KVSTORE_HPP