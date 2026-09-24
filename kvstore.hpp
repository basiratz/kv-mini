#ifndef KVSTORE_HPP
#define KVSTORE_HPP

#include "wal.hpp"
#include "sstable.hpp"
#include <iostream>
#include <string>
#include <map>
#include <optional>
#include <vector>
#include <iomanip>
#include <sstream>

const std::string TOMBSTONE = "__DELETED_TOMBSTONE__";

    
class KVStore{
private:
    std::map<std::string, std::string> data_;
    WAL wal_;
    std::vector<SSTable> sstables_;
    size_t max_memtable_size_;
    size_t block_size_;
    size_t sstable_counter_ = 0;

    // helper to generate sequential filenames like 000001.sst ..
    std::string generate_sstable_filename(){
        sstable_counter_++;
        std::stringstream ss;
        ss << "sst_" << std::setfill('0') <<std::setw(6) << sstable_counter_ << ".sst";
        return ss.str();
    }

    //flushes current MemTable data_ to an SSTable file on disk
    void  flush(){
        if(data_.empty()) return;

        std::string filename = generate_sstable_filename();
        std::cout << "[FLUSH] MemTable full. Flushing to " << filename << "...\n";  
        
        if(SSTable::write_from_memtable(filename, data_)){
            sstables_.push_back(SSTable(filename, block_size_));
            data_.clear(); //clear the RAM

            // Clear the WAL file since contents are now safely persisited in the SSTable
            std::ofstream clear_wal("wal.log", std::ofstream::trunc);
        } else{
            std::cerr << "[ERROR] Failed to flush MemTable to " << filename << "\n";
        }
    
    }

public:
    //constructor recovers the state automatically from the log file
    explicit KVStore(const std::string& wal_path, size_t max_memtable_size = 8, size_t block_size = 4)
        : wal_(wal_path), max_memtable_size_(max_memtable_size), block_size_(block_size){
        
        // recover un-flushed entries from WAL
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

        // Check if MemTable reached threshold size
        if(data_.size() >= max_memtable_size_){
            flush();
        }
    }

    //get value 
    std::optional<std::string> get(const std::string& key) const {
        //1. Check MemTable (RAM) ffirst
        auto it = data_.find(key);
        if(it != data_.end()){
            if(it->second == TOMBSTONE){
                return std::nullopt;
            }
            return it->second;
        }
        //2. Check in the SSTable on disk from NEWEST to OLDEST
        for(auto sst_it = sstables_.rbegin(); sst_it != sstables_.rend(); ++sst_it){
            auto val = sst_it->get(key);
            if(val){
                if(*val == TOMBSTONE){
                    return std::nullopt; // key was deleted in a newer SSTable
                }
                std::cout << "  -> Found '" << key << "' in " << sst_it->filename() << " (Disk)\n";
                return val;
            }
        }

        return std::nullopt;
    }

    //Write delete to the log file forst then erase form the map
    bool remove(const std::string& key){
        //1. Check if key exists anywhere in the db first
        if(!get(key).has_value()){
            return false;
        }
      
        //2. Log the deletion in WAL
        wal_.log_delete(key);

        // 3. Store a Tombstone in MemTable to override older SSTable Files
        data_[key] = TOMBSTONE;


        return true;
    }

    //helper to print current storage state
   void print_all() const {
        std::cout << "--- Current DB State ---\n";
        std::cout << "MemTable Keys (" << data_.size() << "):\n";
        for (const auto& [key, value] : data_) {
            std::cout << "  [MemTable] " << key << " => " << value << "\n";
        }
        std::cout << "Flushed SSTables: " << sstables_.size() << " file(s)\n";
        std::cout << "------------------------\n";
    }

};


#endif //KVSTORE_HPP