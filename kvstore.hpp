#ifndef KVSTORE_HPP
#define KVSTORE_HPP

#include "wal.hpp"
#include <iostream>
#include <string>
#include <map>
#include <optional>


class KVStore{
private:
    std::map<std::string, std::string> data_;
    WAL wal_;

public:
    //constructor recovers the state auto fromt he log file
    explicit KVStore(const std::string& wal_path): wal_(wal_path){
        auto entries = wal_.recover();
        for(const auto& entry: entries){
            if(entry.type == "PUT"){
                data_[entry.key] = entry.value;
            } else if(entry.type == "DELETE"){
                data_.erase(entry.key);
            }
        }
    }

    //write-ahead pattern: Write to log first, then update in-memory state(map)
    void put(const std::string& key, const std::string& value){
        wal_.log_put(key, value);
        data_[key] = value;
    }

    //get value from in-memory state
    std::optional<std::string> get(const std::string& key) const {
        auto it = data_.find(key);
        if(it != data_.end()){
            return it->second;
        }
        return std::nullopt;
    }

    //Write delet to the log file forst then erase form the map
    bool remove(const std::string& key){
        if(data_.find(key) ==  data_.end()){
            return false;
        }
        wal_.log_delete(key);
        data_.erase(key);
        return true;
    }

    //helper to print current storage state
    void print_all() const {
        std::cout << "--- Current DB State (" << data_.size() << " keys) ---\n";    
        for(const auto& [key, value]: data_){
            std::cout << "  " << key << " => " << value << "\n";
        }
        std::cout << "------------------------------------\n";
    }

};


#endif //KVSTORE_HPP